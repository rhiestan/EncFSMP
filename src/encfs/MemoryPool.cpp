/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2003, Valient Gough
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MemoryPool.h"

#include <cstring>
#include <openssl/ossl_typ.h>
#include <boost/thread.hpp>

#ifdef HAVE_VALGRIND_MEMCHECK_H
#include <valgrind/memcheck.h>
#else
#define VALGRIND_MAKE_MEM_NOACCESS(a, b)
#define VALGRIND_MAKE_MEM_UNDEFINED(a, b)
#endif

#include <openssl/buffer.h>

#define BLOCKDATA(BLOCK) (unsigned char *)(BLOCK)->data->data

namespace encfs {

struct BlockList {
  BlockList *next;
  int size;
  BUF_MEM *data;
};

static BlockList *allocBlock(int size) {
  auto *block = new BlockList;
  block->size = size;
  block->data = BUF_MEM_new();
  BUF_MEM_grow(block->data, size);
  VALGRIND_MAKE_MEM_NOACCESS(block->data->data, block->data->max);

  return block;
}

static void freeBlock(BlockList *el) {
  VALGRIND_MAKE_MEM_UNDEFINED(el->data->data, el->data->max);
  BUF_MEM_free(el->data);

  delete el;
}

static boost::mutex gMPoolMutex;
static BlockList *gMemPool = nullptr;

MemBlock MemoryPool::allocate(int size) {
  BlockList *block = nullptr;
  {
    boost::lock_guard<boost::mutex> lock(gMPoolMutex);

    BlockList *parent = nullptr;
    block = gMemPool;
    // check if we already have a large enough block available..
    while (block != nullptr && block->size < size) {
      parent = block;
      block = block->next;
    }

    // unlink block from list
    if (block != nullptr) {
      if (parent == nullptr) {
        gMemPool = block->next;
      } else {
        parent->next = block->next;
      }
    }
  }

  if (block == nullptr) {
    block = allocBlock(size);
  }
  block->next = nullptr;

  MemBlock result;
  result.data = BLOCKDATA(block);
  result.internalData = block;

  VALGRIND_MAKE_MEM_UNDEFINED(result.data, size);

  return result;
}

void MemoryPool::release(const MemBlock &mb) {
  boost::lock_guard<boost::mutex> lock(gMPoolMutex);

  auto *block = (BlockList *)mb.internalData;

  // just to be sure there's nothing important left in buffers..
  VALGRIND_MAKE_MEM_UNDEFINED(block->data->data, block->size);
  memset(BLOCKDATA(block), 0, block->size);
  VALGRIND_MAKE_MEM_NOACCESS(block->data->data, block->data->max);

  block->next = gMemPool;
  gMemPool = block;
}

void MemoryPool::destroyAll() {
  BlockList *block = nullptr;
  {
    boost::lock_guard<boost::mutex> lock(gMPoolMutex);

    block = gMemPool;
    gMemPool = nullptr;
  }

  while (block != nullptr) {
    BlockList *next = block->next;

    freeBlock(block);
    block = next;
  }
}

}  // namespace encfs

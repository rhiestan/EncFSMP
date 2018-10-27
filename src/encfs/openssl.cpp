/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2007, Valient Gough
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

#include "config.h"

#include "openssl.h"

#if defined(_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>
#elif defined(HAVE_PTHREAD_H)
#	include <pthread.h>
#endif

#include <cstdlib>
#include <openssl/crypto.h>

#include <boost/thread.hpp>

#define NO_DES
#include <openssl/rand.h>
#include <openssl/ssl.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif

#include "Error.h"

namespace encfs {

unsigned long threads_thread_id()
{
#if defined(_WIN32)
	return GetCurrentThreadId();
#elif defined(HAVE_PTHREAD_H)
    return (unsigned long)pthread_self();
#else
	unsigned long threadid;   
	std::stringstream ss;
	ss << std::hex << boost::this_thread::get_id();
	ss >> threadid;
	return threadid;
#endif
}

static boost::mutex *crypto_locks = nullptr;
void threads_locking_callback(int mode, int n, const char *caller_file,
                               int caller_line) {
  (void)caller_file;
  (void)caller_line;

  if (crypto_locks == nullptr) {
    VLOG(1) << "Allocating " << CRYPTO_num_locks() << " locks for OpenSSL";
    crypto_locks = new boost::mutex[ CRYPTO_num_locks() ];
  }

  if ((mode & CRYPTO_LOCK) != 0) {
    crypto_locks[n].lock();
  } else {
    crypto_locks[n].unlock();
  }
}

void pthreads_locking_cleanup() {
  if (crypto_locks != nullptr) {
    delete[] crypto_locks;
    crypto_locks = nullptr;
  }
}

void openssl_init(bool threaded) {
  // initialize the SSL library
  SSL_load_error_strings();
  SSL_library_init();

  unsigned int randSeed = 0;
  RAND_bytes((unsigned char *)&randSeed, sizeof(randSeed));
  srand(randSeed);

#ifndef OPENSSL_NO_ENGINE
  /* Load all bundled ENGINEs into memory and make them visible */
  ENGINE_load_builtin_engines();
  /* Register all of them for every algorithm they collectively implement */
  ENGINE_register_all_complete();
#endif  // NO_ENGINE

  if (threaded) {
    // provide locking functions to OpenSSL since we'll be running with
    // threads accessing openssl in parallel.
    CRYPTO_set_id_callback(threads_thread_id);
    CRYPTO_set_locking_callback(threads_locking_callback);
  }
}

void openssl_shutdown(bool threaded) {
#ifndef OPENSSL_NO_ENGINE
  ENGINE_cleanup();
#endif

  if (threaded) {
    pthreads_locking_cleanup();
  }
}

}  // namespace encfs

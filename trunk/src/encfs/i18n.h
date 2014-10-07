/*****************************************************************************
 * Author:   Valient Gough <vgough@pobox.com>
 *
 *****************************************************************************
 * Copyright (c) 2004, Valient Gough
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

#ifndef _i18n_incl_
#define _i18n_incl_

#if defined(LOCALEDIR)

#  include "gettext.h"
// make shortcut for gettext
#  define _(STR) gettext (STR)

#  include "autosprintf.h"
using gnu::autosprintf;

#else

#  define gettext(STR) (STR)
#  define gettext_noop(STR) (STR)
#  define _(STR) (STR)
#  define N_(STR) (STR)

#endif

#endif



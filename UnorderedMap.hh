/*
 * Copyright (c) 2016  Giulio Paci <giuliopaci@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 (June
 * 1991) as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you will find it at
 * http://www.gnu.org/licenses/gpl.html, or write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110,
 * USA.
 */

#ifndef _UNORDEREDMAP_HH
#define _UNORDEREDMAP_HH

#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#include <unordered_map>
#else
#include <tr1/unordered_map>
namespace std { using namespace tr1; }
#endif

#endif // _UNORDEREDMAP_HH

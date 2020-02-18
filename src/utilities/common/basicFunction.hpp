/* Copyright (C) 
 * 2019 - Hu Ren, rh890127a@163.com
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */
/**
 * @file basicFunction.hpp
 * @brief basic functionalities that would be used universally
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-16
 */

#include "basicFunction.h"
#include "usingCpp.hpp"

#ifndef HSF_BASICFUNCTION_HPP
#define HSF_BASICFUNCTION_HPP

namespace HSF
{

// convert string to Hash (int64_t)
inline Hash strToHash(const string& str)
{
  return str2Hash(str.c_str() );
}

// convert string to Hash32 (int32_t)
inline Hash32 strToHash32(const string& str)
{
  return str2Hash32(str.c_str() );
}

// remake an directory and move the older one to xx.old
inline int remakeDirectory(const string& name)
{
  return remakeDir(name.c_str());
}

} // namespace HSF

#endif // HSF_BASICFUNCTION_HPP

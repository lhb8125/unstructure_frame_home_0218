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
 * @file usingCpp.hpp
 * @brief Declare some c++ standard feathers
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-16
 */

#include "stdlib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <string>

#ifndef HSF_USINGCPP_HPP
#define HSF_USINGCPP_HPP

namespace HSF
{

// IO stream
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::ofstream;
using std::ostream;
using std::streambuf;
using std::stringstream;

// std container
using std::vector;
using std::pair;
using std::map;
using std::set;
using std::multimap;
using std::multiset;

// std string
using std::string;
// using this to replace std::to_string that add extra ZEROs 
// in float number conversion ( noticed in icc 2018 );
// ZEROs before effective figure will be neglected
template<class U> 
inline string to_string( const U data )
{
  stringstream sstrm;
  // preserve enouph precision for float value
  // commonly 16 for double
  sstrm.precision(16); 
  sstrm << data;
  return sstrm.str();
}

// specilization for float 
template<> 
inline string to_string< float > ( const float data )
{
  stringstream sstrm;
  // preserve enouph precision for float value
  // commonly 7 for float
  sstrm.precision(7);
  sstrm << data;
  return sstrm.str();
}

#if __cplusplus < 201103L

// To preserve presicion, atof is actually returning double
//#define stof(str) strtof((str).c_str(), NULL)
#define stof(str) atof((str).c_str() )
#define stod(str) strtod((str).c_str(), NULL)
#define stold(str) strtold((str).c_str(), NULL)
#define stoi(str) atoi((str).c_str() )
#define stol(str) atol((str).c_str() )
#define stoll(str) atoll((str).c_str() )

#else

// To preserve presicion, stof is actually returning double
using std::stof;
using std::stod;
using std::stold;
using std::stoi;
using std::stol;
using std::stoll;

#endif

}

#endif // HSF_USINGCPP_HPP
 

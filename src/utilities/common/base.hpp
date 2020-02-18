/*
The MIT License

Copyright (c) 2019 Hanfeng GU <hanfenggu@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/**
* @file: base.hpp
* @author: Hanfeng GU
* @email:
* @Date:   2019-10-06 10:07:59
* @Last Modified by:   Hanfeng GU
* @Last Modified time: 2019-11-14 09:50:18
*/

/*
* @brief:
*/

#ifndef BASE_HPP
#define BASE_HPP

#include <mpi.h>

namespace HSF
{
typedef long int  label64;
typedef int       label32;
typedef short int label16;

#if defined(LABEL_INT32)
	typedef int label;
	#define MPI_LABEL MPI_INT
#elif defined(LABEL_INT64)
	typedef long int label;
	#define MPI_LABEL MPI_LONG
#endif


#if defined(SCALAR_FLOAT32)
	typedef float scalar;
	#define MPI_SCALAR MPI_FLOAT
#elif defined(SCALAR_FLOAT64)
	typedef double scalar;
	#define MPI_SCALAR MPI_DOUBLE
#endif

#define DELETE_POINTER(ptr) if(ptr) \
{ \
    delete [] (ptr); \
    ptr = NULL; \
}

#define DELETE_OBJECT_POINTER(ptr) if(ptr) \
{ \
    delete ptr; \
    ptr = NULL; \
}

#define EXIT exit(0)
#define ERROR_EXIT exit(1)

#define SOL_DIM 3

#define CHAR_DIM 200

}

#endif //- Base_hpp

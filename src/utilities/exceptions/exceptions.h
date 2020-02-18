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
 * @file exceptions.h
 * @brief some C compatible exceptions handling utilities 
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-05
 */


#ifndef HSF_EXCEPTIONS_H
#define HSF_EXCEPTIONS_H

#ifdef __cplusplus
extern "C" {
#endif 

#define Terminate(location, content) \
{ \
	printf("Location: \033[31m%s\033[0m, error message: \033[31m%s\033[0m, file: \033[31m%s\033[0m, line: \033[%d\033[0m\n", \
		location, content, __FILE__, __LINE__); \
	exit(-1); \
}

#define WARNING(location, content) \
{ \
	printf("Location: \33[%s], warning message: \33[%s], file: \33[%s], line: \33[%d]\n", \
		location, content, __FILE__, __LINE__); \
}

#define ASSERT( expr ) \
if( ! (expr) ) \


#define ABORT() \
{\
  hsf_print_stack_();\
  hsf_stop_mpi_();\
  abort();\
}

// if( ! (expr) )
// {
//   // some code
//   // abort
//   hsf_print_stack_();
//   hsf_stop_mpi_();
//   abort();
// }

/**
 * @brief hsf_printStack 
 * Print the backtrace to
 * standard IO device or file
 * help locate the scence in which the error occured.
 */
void hsf_print_stack_();

#ifdef __cplusplus
} // extern "C"
#endif 


#endif // HSF_EXCEPTIONS_H

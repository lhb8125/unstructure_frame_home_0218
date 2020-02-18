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
* @file: fieldInterfaces.hpp
* @author: Hanfeng
* @Email:
* @Date:   2019-11-9 10:53:54
* @Last Modified by:   Hanfeng GU
* @Last Modified time: 2019-11-15 09:55:06
*/


#ifndef FIELDINTERFACES_HPP
#define FIELDINTERFACES_HPP

#include "base.hpp"

using namespace HSF;
// namespace HSF
// {

#ifdef __cplusplus
extern "C" {
#endif

/**
* @brief 获取label类型流场信息
* @param[in]  setType label类型流场变量类型名
* @param[in]  fieldName label类型流场变量名
* @param[out]  fPtrPtr 流场变量首地址
* @param[out]  ndim 流场变量结构体内变量数量
* @param[out]  nCells 流场变量结构体数量
*/
void get_label_field_(const char* setType, const char* fieldName,
	void* fPtrPtr, label*ndim, label* nCells);


/**
* @brief 获取scalar类型流场信息
* @param[in]  setType scalar类型流场变量类型名
* @param[in]  fieldName scalar类型流场变量名
* @param[out]  fPtrPtr 流场变量首地址
* @param[out]  ndim 流场变量结构体内变量数量
* @param[out]  nCells 流场变量结构体数量
*/
void get_scalar_field_(const char* setType, const char* fieldName,
	void* fPtrPtr, label* ndim, label* nCells);


/**
* @brief 注册label类型流场信息
* @param[in]  setType label类型流场变量类型名
* @param[in]  fieldName label类型流场变量名
* @param[in]  fPtr 流场变量首地址
* @param[in] ndim 流场变量结构体内变量数量
* @param[in]  n 流场变量结构体数量
*/
void add_label_field_(const char* setType, const char* fieldName,
	label* fPtr, label* ndim, label* n);


/**
* @brief 注册scalar类型流场信息
* @param[in]  setType scalar类型流场变量类型名
* @param[in]  fieldName scalar类型流场变量名
* @param[in]  fPtr 流场变量首地址
* @param[in]  ndim 流场变量结构体内变量数量
* @param[in]  n 流场变量结构体数量
*/
void add_scalar_field_(const char* setType, const char* fieldName,
	scalar* fPtr, label* ndim, label* n);


/**
* @brief 启动label类型流场信息交换
* @param[in]  setType label类型流场变量类型名
* @param[in]  fieldName label类型流场变量名
*/
void start_exchange_label_field_(const char* setType, const char* fieldName);

/**
* @brief 结束label类型流场信息交换
* @param[in]  setType label类型流场变量类型名
* @param[in]  fieldName label类型流场变量名
*/
void finish_exchange_label_field_(const char* setType, const char* fieldName);

/**
* @brief 启动scalar类型流场信息交换
* @param[in]  setType scalar类型流场变量类型名
* @param[in]  fieldName scalar类型流场变量名
*/
void start_exchange_scalar_field_(const char* setType, const char* fieldName);

/**
* @brief 结束scalar类型流场信息交换
* @param[in]  setType scalar类型流场变量类型名
* @param[in]  fieldName scalar类型流场变量名
*/
void finish_exchange_scalar_field_(const char* setType, const char* fieldName);



#ifdef __cplusplus
}
#endif

// } //- end namespace HSF

#endif //- end fortranInterfaces_hpp

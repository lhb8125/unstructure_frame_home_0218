/**
* @file: utilInterfaces.h
* @author: Ren Hu
* @brief: fortran function interfaces
* @date:   2019-11-11 10:56:28
* @last Modified by:   lenovo
* @last Modified time: 2019-11-26 14:18:23
*/
#ifndef UTILINTERFACES_H
#define UTILINTERFACES_H

#include "base.hpp"
#include "basicFunction.h"
#include "usingCpp.hpp"
#include "basicFunction.hpp"
#include "container.hpp"
#include "OStream.hpp"
#include "multiOStream.hpp"
#include "dummyOStream.hpp"
#include "communicator.hpp"
#include "communicationManager.hpp"
#include "mpiWrapper.hpp"
// #include "interface.hpp"

using namespace HSF;

// typedef label LABEL;
// typedef scalar SCALAR;

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************utility初始化*****************************************/
/**
* @brief 初始化utility
*/
void init_utility_(); 

/*****************************************进程间数据交换****************************************/

/**
* @brief 获取进程号
* @param[out]  pid 进程ID
*/
void get_pid_( int * pid );

/**
* @brief 获取进程规模
* @param[out]  commSize 进程总数
*/
void get_commsize_( int * commSize );


/**
* @brief 规约整形变量序列
* @param[in][out]  data label类型变量
* @param[in]  count 变量个数
* @param[in]  op 操作类型
*/
void all_reduce_labels_(label* data, const label* count , unsigned int* op);

/**
* @brief 规约浮点变量序列
* @param[inout]  data scalar类型变量
* @param[in]  count 变量个数
* @param[in]  op 操作类型
*/
void all_reduce_scalars_(scalar* data, const label* count, unsigned int* op);

/**
* @brief 主进程广播整形序列
* @param[in][out]  data 变量序列
* @param[in]  count 变量个数
*/
void bcast_labels_(label* data, const label* count);

/**
* @brief 主进程广播浮点型序列
* @param[in][out]  data 变量序列
* @param[in]  count 变量个数
*/
void bcast_scalars_(scalar* data, const label* count);

/**
* @brief 主进程收集整形序列
* @param[in]  sdata 发送序列
* @param[in][out]  rdata 接收序列
* @param[in]  count 变量个数
*/
void gather_labels_(label* sdata, label* rdata, const label* count );

/**
* @brief 主进程收集浮点序列
* @param[in]  sdata 发送序列
* @param[in][out]  rdata 接收序列
* @param[in]  count 变量个数
*/
void gather_scalars_(scalar* sdata, scalar* rdata, const label* count);

/*******************************************标准输出*******************************************/
/**
* @brief 所有进程输出到特定文件
* @param[in]  format 输出格式
* @param[in]  ... 变量名，不定参数个数
*/
void par_std_out_(const char* format, ...);

/**
* @brief 指定进程号输出到屏幕
* @param[in]  pid 指定进程号
* @param[in]  format 输出格式
* @param[in]  ... 变量名，不定参数个数
*/
void proc_std_out_(const label pid, const char* format, ...);

/**
* @brief 主核输出到屏幕
* @param[in]  format 输出格式
* @param[in]  ... 变量名，不定参数个数
*/
void master_std_out_(char* format, ...);

// Fortran 字符串输出
void fort_cout_(const char* str, const int* len);
void fort_pout_(const char* str, const int* len);
void fort_sout_(const int* pid, const char* str, const int* len);

/*******************************************数据输出*******************************************/
/**
* @brief 写出结果文件，用于重启计算
*/
void write_restart_(); // 写出结果文件，用于重启计算

/**
* @brief 写出网格到CGNS文件中，结果文件名由config文件中resPath关键字指定
*/
void write_mesh_();


/**
* @brief 输出整型场到CGNS文件，结果文件名由config文件中resPath关键字指定
* @param[in]  fieldName 场名
* @param[in]  fieldType 场类型
*/
void write_label_field_(const char* fieldName, const char* fieldType);

/**
* @brief 输出浮点场到CGNS文件，结果文件名由config文件中resPath关键字指定
* @param[in]  fieldName 场名
* @param[in]  fieldType 场类型
*/
void write_scalar_field_(const char* fieldName, const char* fieldType);

/****************************************控制参数输出*******************************/
/**
* @brief 获取整型控制参数
* @param[in]  nPara 控制参数个数
* @param[out]  retVal 控制参数值
* @param[in] ...  控制参数树字符串
*/
void get_label_para_(int* retVal, int* nPara, ...);

/**
* @brief 获取浮点型控制参数
* @param[in]  nPara 控制参数个数
* @param[out]  retVal 控制参数值
* @param[in]  ... 控制参数树字符串
*/
void get_scalar_para_(float* retVal, int* nPara, ...);
 
/**
* @brief 获取字符串控制参数
* @param[in]  nPara 控制参数个数
* @param[out]  retVal 控制参数值
* @param[in]  ... 控制参数树字符串
*/
void get_string_para_(char* retVal, int* str_len, int* nPara, ...);




#ifdef __cplusplus
}
#endif


#endif // INTERFACE_H

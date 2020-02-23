/*
* @author: liu hongbin
* @brief: vector interfaces
* @email: lhb8134@foxmail.com
* @date:   2020-02-22 12:09:40
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-22 14:48:06
*/

#ifndef VECTORINTERFACES_H
#define VECTORINTERFACES_H

#include "mpi.h"
// #include "voidtor.hpp"


// #define void Vector<double>
#define HSF_COMM MPI_COMM_WORLD

typedef struct
{
	void* hsf_vec;
}Vec;

#ifdef __cplusplus
extern "C"
{
#endif
/**
* @brief 给串行向量v开辟空间
* @param[in]  v 待开辟空间向量
* @param[in]  n 向量大小
*/
void HSF_VecConstrSeq (Vec* v, int n);

/**
* @brief 给并行向量v开辟空间
* @param[in]  v 待开辟空间向量
* @param[in]  n 向量大小
* @param[in]  nbghosts ghost元素个数
* @param[in]  ghosts ghost元素的全局位置
*/
void HSF_VecConstrPar (Vec* v, int n, int nbghosts, int* ghosts);

/**
* @brief 释放向量空间
*/
void HSF_VecDestr (Vec* v);

/**
* @brief 给向量指定位置赋值
*/
void HSF_VecSetCmp (Vec* v, int ind, double value);

/**
* @brief 给向量所有位置赋相同的值
*/
void HSF_VecSetAllCmp (Vec* v, double value);

/**
* @brief 获得向量指定位置的值，当前进程上
* @param[in]  v 取值向量
* @param[in]  ind 取值位置
* @return  返回向量指定位置的值
*/
double HSF_VecGetCmp (Vec* v, int ind);


/**
* @brief Obtains the local ghosted representation of a parallel vector
* @param[in]  vg the global vector
* @param[out]  vl the local (ghosted) representation, NULL if g is not ghosted
*/
void HSF_VecGhostGetLocalForm(Vec* vg, Vec* vl);

/**
* @brief Restores the local ghosted representation of a parallel vector obtained with V_GhostGetLocalForm()
* @param[in]  vg the global vector
* @param[out]  vl the local (ghosted) representation
*/

void HSF_VecGhostRestoreLocalForm(Vec* vg, Vec* vl);

/**
* @brief 向量ghost部分更新
* @param[in]  v 被更新向量
*/
void HSF_VecGhostUpdate(Vec* v);

/**
* @brief 复制向量
* @param[in]  vfrom 被复制向量
* @param[out]  vto 结果向量
*/
void HSF_VecCopy(Vec* vfrom, Vec* vto);

/**
* @brief 计算向量第二范数
* @param[in]  v 被求向量
* @param[out]  norm2 v的第二范数
*/
void HSF_VecNorm2(Vec* v, double *norm2);

/**
* @brief w = alpha * x + y
* @param[out]  w 结果向量
*/
void HSF_VecWAXPY(Vec* w, double alpha, Vec* x, Vec* y);

/**
* @brief y = alpha * x + y
* @param[out]  y 结果向量
*/
void HSF_VecAXPY(Vec* y, double alpha, Vec* x);

/**
* @brief 点乘 w = x*y
* @param[out]  w 点乘后返回的结果向量
*/
void HSF_VecPointwiseMult(Vec* w, Vec* x, Vec* y);

/**
* @brief 获取向量的最大值及其位置
* @param[in]  x 目标向量
* @param[out]  loc 最大值位置
* @param[out]  val 最大值
*/
void HSF_VecMax(Vec* v, int *loc, double *val);

/**
* @brief 向量写入到文件
*/
void HSF_VecWrite(Vec* x);

#ifdef __cplusplus
};
#endif
#endif
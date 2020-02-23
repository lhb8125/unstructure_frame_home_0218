/**
* @file: hsf_vector.hpp
* @author: Liu Hongbin
* @brief: class vector 
* @date:   2020-02-20
*/
#ifndef HSF_VECTOR_HPP
#define HSF_VECTOR_HPP

#include "mpi.h"

namespace HSF
{


/**
* @brief vector
*/
template<typename T>
class Vector
{
private:
	T* data_;
	int* ghosts_;
	int  nbghosts_;
	int  localSize_;
	int* indice_;
	int  rank_;
	int  nprocs_;
	MPI_Comm comm_;
	MPI_Datatype dataType_; 

	int  sendNum_;
	int* sendCount_;
	int* sendInd_;
	int* recvCount_;
	int* recvDisp_;

public:
	Vector() : 
		data_(NULL), 
		ghosts_(NULL), 
		localSize_(0), 
		nbghosts_(0),
		indice_(NULL), 
		rank_(0), 
		nprocs_(1),
		sendNum_(0),
		sendCount_(NULL),
		sendInd_(NULL),
		recvCount_(NULL),
		recvDisp_(NULL)
	{};
	~Vector();
	/**
	* @brief 开辟长度为n的空间
	* @param[in] n 向量长度
	*/
	void createVecSeq(int n);

	/**
	* @brief 开辟长度为n的空间（多进程）
	* @param[in] n 本地向量长度
	* @param[in] comm MPI通信子
	* @param[in] nbghosts ghost元素个数
	* @param[in] ghosts ghost元素的全局位置
	*/
	void createVecPar(int n, MPI_Comm comm, int nbghosts, int* ghosts);

	/**
	* @brief 销毁向量
	*/
	void destroyVec();

	/**
	* @brief 克隆向量
	* @param[in] vec 源向量
	*/
	void clone(Vector<T>& vec);

	/**
	* @brief 给向量指定位置赋值
	* @param[in] n 赋值元素个数
	* @param[in] ind 向量位置下标（全局）
	* @param[in] value 值
	*/
	void setVals(int n, int* ind, T* value);

	/**
	* @brief 给向量所有位置赋相同值
	* @param[in] value 值
	*/
	void setAllVal(T value);

	/**
	* @brief 获得向量指定位置的值，当前进程上
	* @param[in] n 元素个数
	* @param[in]  ind 取值位置
	* @param[out] val 向量指定位置的值
	*/
	void getVals (int n, int* ind, T* val);

	/**
	* @brief 向量ghost部分更新
	*/
	void ghostUpdate();

	/**
	* @brief 计算向量二范数
	* @return 二范数
	*/
	T norm2();

	T* getArray(){ return this->data_;};

	int getLocalSize() {return this->localSize_;};


}; // end class Vector

} // end namespace HSF

#endif
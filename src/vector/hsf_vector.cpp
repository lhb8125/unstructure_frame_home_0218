/*
* @author: lhb8125
* @fileL hsf_vector.cpp
* @date:   2020-02-20 17:00:58
* @Last Modified by:   lhb8125
* @Last Modified time: 2020-02-22 11:58:06
*/

#include "hsf_vector.hpp"
#include <typeinfo>
#include <math.h>
#include "utilities.hpp"

namespace HSF
{
template<typename T>
Vector<T>::~Vector()
{
	DELETE_POINTER(this->data_);
	DELETE_POINTER(this->indice_);
	DELETE_POINTER(this->ghosts_);
	DELETE_POINTER(this->sendCount_);
	DELETE_POINTER(this->sendInd_);
	DELETE_POINTER(this->recvCount_);
	DELETE_POINTER(this->recvDisp_);
}

template<typename T>
void Vector<T>::createVecSeq(int n)
{
	T* data = new T[n];
	this->data_ = data;
	this->localSize_ = n;
	this->indice_ = new int[2];
	this->indice_[0] = 0;
	this->indice_[1] = n;
}

template<typename T>
void Vector<T>::createVecPar(int n, MPI_Comm comm, int nbghosts, int* ghosts)
{
	this->data_ = new T[n+nbghosts];
	this->localSize_ = n;
	this->nbghosts_ = nbghosts;
	this->ghosts_ = new int[nbghosts];
	for (int i = 0; i < nbghosts; ++i)
	{
		this->ghosts_[i] = ghosts[i];
	}

	this->comm_ = comm;
	int rank, nprocs;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &nprocs);
	this->rank_ = rank;
	this->nprocs_ = nprocs;

	this->indice_ = new int[nprocs+1];
	this->indice_[0] = 0;
	MPI_Allgather(&n, 1, MPI_INT, &this->indice_[1], 1, MPI_INT, comm);
	for (int i = 1; i < nprocs+1; ++i)
	{
		this->indice_[i] += this->indice_[i-1];
	}

	if(typeid(T)==typeid(int)) 
		this->dataType_ = MPI_INT;
	else if(typeid(T)==typeid(float))
		this->dataType_ = MPI_FLOAT;
	else if(typeid(T)==typeid(double))
		this->dataType_ = MPI_DOUBLE;
	else
		Terminate("getCmp", "MPI data type is not supported");

	// 构建ghost通信拓扑
	int sendCount[this->nprocs_];
	Array<Array<int> > sendIndVec(this->nprocs_);
	for(int i=0; i<this->nprocs_;i++) sendCount[i] = 0;
	for (int i = 0; i < nbghosts; ++i)
	{
		bool found = false;
		for (int irank = 0; irank < this->nprocs_; ++irank)
		{
			if(ghosts[i]>=this->indice_[irank] && ghosts[i]<this->indice_[irank+1])
			{
				sendCount[irank]++;
				sendIndVec[irank].push_back(ghosts[i]);
				found = true;
				break;
			}
		}
		if(!found) Terminate("setVals", "the index exceeds range");
	}

	int recvCount[this->nprocs_];
	int recvDisp[this->nprocs_+1];
	int sendDisp[this->nprocs_+1];
	recvDisp[0] = 0;
	sendDisp[0] = 0;
	MPI_Allgather(sendCount, 1, MPI_INT, recvCount, 1, MPI_INT, this->comm_);
	for (int i = 0; i < this->nprocs_; ++i)
	{
		recvDisp[i+1]  += recvCount[i];
		sendDisp[i+1]  += sendCount[i];
	}
	int recvInd[recvDisp[this->nprocs_]];
	ArrayArray<int> sendInd;
	transformArray(sendIndVec, sendInd);

	MPI_Allgatherv(
		sendInd.data,
		sendCount,
		MPI_INT,
		recvInd,
		recvCount,
		recvDisp,
		MPI_INT,
		this->comm_);

	// 本进程发送给其他进程的向量元素总数
	this->sendNum_ = recvDisp[this->nprocs_];
	// 本进程发送给其他进程的向量元素下标
	this->sendInd_ = new int[recvDisp[this->nprocs_]];
	for (int i = 0; i < recvDisp[this->nprocs_]; ++i)
	{
		this->sendInd_[i] = recvInd[i];
	}
	// 本进程发送给其他进程的向量元素个数
	this->sendCount_ = new int[this->nprocs_];
	for (int i = 0; i < this->nprocs_; ++i)
	{
		this->sendCount_ = recvCount[i];
	}
	// 本进程收到其他进程的ghost向量元素个数
	this->recvCount_ = new int[this->nprocs_];
	for (int i = 0; i < this->nprocs_; ++i)
	{
		this->recvCount_[i] = sendCount[i];
	}
	// 本进程收到其他进程的ghost向量元素相对于首地址偏移量
	this->recvDisp_ = new int[this->nprocs_+1];
	for (int i = 0; i < this->nprocs_+1; ++i)
	{
		this->recvDisp_[i] = sendDisp[i];
	}
}

template<typename T>
void Vector<T>::destroyVec()
{
	~Vector();
}

template<typename T>
void Vector<T>::setVals(int n, int* ind, T* value)
{
	int sendCount[this->nprocs_];
	Array<Array<int> > sendIndVec(this->nprocs_);
	Array<Array<T> >   sendValVec(this->nprocs_);
	for(int i=0; i<this->nprocs_;i++) sendCount[i] = 0;
	for (int i = 0; i < n; ++i)
	{
		bool found = false;
		for (int irank = 0; irank < this->nprocs_; ++irank)
		{
			if(ind[i]>=this->indice_[irank] && ind[i]<this->indice_[irank+1])
			{
				sendCount[irank]++;
				sendIndVec[irank].push_back(ind[i]);
				sendValVec[irank].push_back(value[i]);
				found = true;
				break;
			}
		}
		if(!found) Terminate("setVals", "the index exceeds range");
	}

	int recvCount[this->nprocs_];
	int recvDisp[this->nprocs_+1];
	recvDisp[0] = 0;
	MPI_Allgather(sendCount, 1, MPI_INT, recvCount, 1, MPI_INT, this->comm_);
	for (int i = 0; i < this->nprocs_; ++i)
	{
		recvDisp[i+1]  += recvCount[i];
	}
	int recvInd[recvDisp[this->nprocs_]];
	T   recvVal[recvDisp[this->nprocs_]];
	ArrayArray<int> sendInd;
	ArrayArray<T> sendVal;
	transformArray(sendIndVec, sendInd);
	transformArray(sendValVec, sendVal);

	MPI_Allgatherv(
		sendInd.data,
		sendCount,
		MPI_INT,
		recvInd,
		recvCount,
		recvDisp,
		MPI_INT,
		this->comm_);

	MPI_Allgatherv(
		sendVal.data,
		sendCount,
		this->dataType_,
		recvVal,
		recvCount,
		recvDisp,
		this->dataType_,
		this->comm_);

	for (int i = 0; i < recvDisp[this->nprocs_]; ++i)
	{
		this->data_[recvInd[i]-this->indice_[this->rank_]] = recvVal[i];
	}
}

template<typename T>
void Vector<T>::setAllVal(T value)
{
	for (int i = 0; i < this->localSize_; ++i)
	{
		this->data_[i] = value;
	}
	MPI_Barrier(this->comm_);
}

template<typename T>
void Vector<T>::getVals (int n, int* ind, T* val)
{
	for (int i = 0; i < n; ++i)
	{
		val[i] = this->data_[ind[i]-this->indice_[this->rank_]];
	}
}

template<typename T>
void Vector<T>::ghostUpdate()
{
	T* sendVal = new T[this->sendNum_];
	for (int i = 0; i < this->sendNum_; ++i)
	{
		sendVal[i] = this->data_[this->sendInd_[i]-this->indice_[this->rank_]];
	}

	MPI_Allgatherv(
		sendVal,
		this->sendCount_,
		this->dataType_,
		this->ghosts_,
		this->recvCount_,
		this->recvDisp_,
		this->dataType_,
		this->comm_);

	DELETE_POINTER(sendVal);
}

template<typename T>
void Vector<T>::clone(Vector<T>& vec)
{
	this->localSize_ = vec.localSize_;
	this->nbghosts_  = vec.nbghosts_;
	this->rank_      = vec.rank_;
	this->nprocs_    = vec.nprocs_;
	this->comm_      = vec.comm_;
	this->dataType_  = vec.dataType_;
	this->sendNum_   = vec.sendNum_;

	this->data_ = new T[this->localSize_+this->nbghosts_];
	for (int i = 0; i < this->localSize_+this->nbghosts_; ++i)
	{
		this->data_[i] = vec.data_[i];
	}

	this->ghosts_ = new int[this->nbghosts_];
	for (int i = 0; i < this->nbghosts_; ++i)
	{
		this->ghosts_[i] = vec.ghosts_[i];
	}

	this->indice_ = new int[this->nprocs_+1];
	for (int i = 0; i < this->nprocs_+1; ++i)
	{
		this->indice_[i] = vec.indice_[i];
	}

	this->sendCount_ = new int[this->nprocs_];
	this->recvCount_ = new int[this->nprocs_];
	for (int i = 0; i < this->nprocs_; ++i)
	{
		this->sendCount_[i] = vec.sendCount_[i];
		this->recvCount_[i] = vec.recvCount_[i];
	}

	this->sendInd_ = new int[this->sendNum_];
	for (int i = 0; i < this->sendNum_; ++i)
	{
		this->sendInd_[i] = vec.sendInd_[i];
	}

	this->recvDisp_ = new int[this->nprocs_+1];
	for (int i = 0; i < this->nprocs_+1; ++i)
	{
		this->recvDisp_[i] = vec.recvDisp_[i];
	}
}

template<typename T>
T Vector<T>::norm2()
{
	T square = 0;
	for (int i = 0; i < this->localSize_; ++i)
	{
		square += this->data_[i]*this->data_[i];
	}
	return sqrt(square);
}

} // end namespace HSF
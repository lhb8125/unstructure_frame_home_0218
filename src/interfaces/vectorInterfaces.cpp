/*
* @author: liu hongbin
* @brief: 
* @email: lhb8134@foxmail.com
* @date:   2020-02-22 12:21:50
* @last Modified by:   lhb8125
* @last Modified time: 2020-02-23 19:51:47
*/

#include "vectorInterfaces.h"
#include "hsf_vector.hpp"
#include "utilities.hpp"

using namespace HSF;

#define HSF_Vec Vector<double>


void HSF_VecConstrSeq(Vec* v, int n)
{
	HSF_Vec *hsf_vec = new HSF_Vec();
	hsf_vec->createVecSeq(n);
	v->hsf_vec = hsf_vec;
}

void HSF_VecConstrPar(Vec* v, int n, int nbghosts, int* ghosts)
{
	HSF_Vec *hsf_vec = new HSF_Vec();
	hsf_vec->createVecPar(n, HSF_COMM, nbghosts, ghosts);
	v->hsf_vec = hsf_vec;
}

void HSF_VecDestr(Vec* v)
{
	HSF_Vec *vec = (HSF_Vec*)v->hsf_vec;
	vec->destroyVec();
}

void HSF_VecSetCmp(Vec* v, int ind, double value)
{
	HSF_Vec *vec = (HSF_Vec*)v->hsf_vec;
	vec->setVals(1, &ind, &value);
}

void HSF_VecSetAllCmp (Vec* v, double value)
{
	HSF_Vec *vec = (HSF_Vec*)v->hsf_vec;
	vec->setAllVal(value);
}

double HSF_VecGetCmp (Vec* v, int ind)
{
	HSF_Vec *vec = (HSF_Vec*)v->hsf_vec;
	double result;
	vec->getVals(1, &ind, &result);
	return result;
}

void HSF_VecGhostGetLocalForm(Vec* vg, Vec* vl)
{
	vl = vg;
}

void HSF_VecGhostRestoreLocalForm(Vec* vg, Vec* vl)
{

}

void HSF_VecGhostUpdate(Vec* v)
{
	HSF_Vec *vec = (HSF_Vec*)v->hsf_vec;
	vec->ghostUpdate();
}

void HSF_VecCopy(Vec* vfrom, Vec* vto)
{
	HSF_Vec *vecTo = (HSF_Vec*)vto->hsf_vec;
	HSF_Vec *vecFrom = (HSF_Vec*)vfrom->hsf_vec;
	vecTo->clone(*vecFrom);
}

void HSF_VecNorm2(Vec* v, double *norm2)
{
	HSF_Vec *vec = (HSF_Vec*)v->hsf_vec;
	norm2[0] = vec->norm2();
}

void HSF_VecWAXPY(Vec* w, double alpha, Vec* x, Vec* y)
{
	HSF_Vec *w_vec = (HSF_Vec*)w->hsf_vec;
	HSF_Vec *x_vec = (HSF_Vec*)x->hsf_vec;
	HSF_Vec *y_vec = (HSF_Vec*)y->hsf_vec;

	double *w_arr = w_vec->getArray();
	double *x_arr = x_vec->getArray();
	double *y_arr = y_vec->getArray();

	int w_size = w_vec->getLocalSize();
	int x_size = x_vec->getLocalSize();
	int y_size = y_vec->getLocalSize();

	if(w_size!=x_size || w_size!=y_size)
		Terminate("VecWAXPY", "the sizes do not match");

	for (int i = 0; i < w_size; ++i)
	{
		w_arr[i] = alpha*x_arr[i]+y_arr[i];
	}
}

void HSF_VecAXPY(Vec* y, double alpha, Vec* x)
{
	HSF_Vec *x_vec = (HSF_Vec*)x->hsf_vec;
	HSF_Vec *y_vec = (HSF_Vec*)y->hsf_vec;

	double *x_arr = x_vec->getArray();
	double *y_arr = y_vec->getArray();

	int x_size = x_vec->getLocalSize();
	int y_size = y_vec->getLocalSize();

	if(x_size!=y_size)
		Terminate("VecAXPY", "the sizes do not match");

	for (int i = 0; i < x_size; ++i)
	{
		y_arr[i] = alpha*x_arr[i]+y_arr[i];
	}
}

void HSF_VecPointwiseMult(Vec* w, Vec* x, Vec* y)
{
	HSF_Vec *w_vec = (HSF_Vec*)w->hsf_vec;
	HSF_Vec *x_vec = (HSF_Vec*)x->hsf_vec;
	HSF_Vec *y_vec = (HSF_Vec*)y->hsf_vec;

	double *w_arr = w_vec->getArray();
	double *x_arr = x_vec->getArray();
	double *y_arr = y_vec->getArray();

	int w_size = w_vec->getLocalSize();
	int x_size = x_vec->getLocalSize();
	int y_size = y_vec->getLocalSize();

	if(w_size!=x_size || w_size!=y_size)
		Terminate("VecWAXPY", "the sizes do not match");

	for (int i = 0; i < w_size; ++i)
	{
		w_arr[i] = x_arr[i]*y_arr[i];
	}
}

void HSF_VecMax(Vec* v, int *loc, double *val)
{
	HSF_Vec *vec = (HSF_Vec*)v->hsf_vec;
	loc[0] = 0;
	double *arr = vec->getArray();
	val[0] = arr[0];
	int size = vec->getLocalSize();
	for (int i = 1; i < size; ++i)
	{
		if(arr[i]>val[0])
		{
			loc[0] = i;
			val[0] = arr[i];
		}
	}
}
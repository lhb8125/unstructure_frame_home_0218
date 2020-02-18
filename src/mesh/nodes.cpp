/**
* @file: nodes.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-14 16:47:22
* @last Modified by:   lenovo
* @last Modified time: 2019-12-18 10:45:02
*/
#include <cstdio>
#include "nodes.hpp"

namespace HSF
{

/*
* @brief get the count of nodes
*/
label Nodes::size()
{
	return this->x_.size();
}

// label Nodes::getStart()
// {
// 	return this->start_;
// }

// label Nodes::getEnd()
// {
// 	return this->end_;
// }

// void Nodes::setStart(const label start)
// {
// 	this->start_ = start;
// }

// void Nodes::setEnd(const label end)
// {
// 	this->end_ = end;
// }

/**
* @brief default constructor
*/
Nodes::Nodes() : xyz_()
{
	refCount_ = new RefCounted();
	x_.clear();
	y_.clear();
	z_.clear();
	start_ = 0;
	end_   = 0;
}

/**
* @brief default deconstructor
*/
Nodes::~Nodes()
{
	this->x_.clear();
	this->y_.clear();
	this->z_.clear();
	if(refCount_->DecRefCount()<=0)
	{
		// printf("%d\n", refCount_->GetRefCount());
		DELETE_POINTER(refCount_);
	}
}

Nodes::Nodes(const Nodes& node)
{
	this->refCount_ = node.refCount_;
	this->xyz_   = node.xyz_;
	this->x_     = node.x_;
	this->y_     = node.y_;
	this->z_     = node.z_;
	this->refCount_->incRefCount();
	this->start_ = node.start_;
	this->end_   = node.end_;
}

/*
* @param x Coordinate X
* @param y Coordinate Y
* @param z Coordinate Z
*/
Nodes::Nodes(Array<scalar>& x, Array<scalar>& y, Array<scalar>& z)
{
	refCount_ = new RefCounted();
	label num = x.size();
	for (int i = 0; i < num; ++i)
	{
		x_.push_back(x[i]);
		y_.push_back(y[i]);
		z_.push_back(z[i]);
	}
	xyz_.num = num;
	xyz_.startIdx = new label[num+1];
	xyz_.startIdx[0] = 0;
	xyz_.data = new scalar[num*3];
	for (int i = 0; i < num; ++i)
	{
		xyz_.startIdx[i+1] = xyz_.startIdx[i]+3;
		xyz_.data[i*3+0] = x_[i];
		xyz_.data[i*3+1] = y_[i];
		xyz_.data[i*3+2] = z_[i];
	}
}
/*
* @param x Coordinate X
* @param y Coordinate Y
* @param z Coordinate Z
*/
Nodes::Nodes(scalar* x, scalar* y, scalar* z, label num)
{
	refCount_ = new RefCounted();
	for (int i = 0; i < num; ++i)
	{
		x_.push_back(x[i]);
		y_.push_back(y[i]);
		z_.push_back(z[i]);
	}
	xyz_.num = num;
	xyz_.startIdx = new label[num+1];
	xyz_.startIdx[0] = 0;
	xyz_.data = new scalar[num*3];
	for (int i = 0; i < num; ++i)
	{
		xyz_.startIdx[i+1] = xyz_.startIdx[i]+3;
		xyz_.data[i*3+0] = x_[i];
		xyz_.data[i*3+1] = y_[i];
		xyz_.data[i*3+2] = z_[i];
	}
	start_ = 0;
	end_ = 0;
}

void Nodes::copy(Nodes* nodes)
{
	int num = nodes->size();
	Array<scalar> x = nodes->getX();
	Array<scalar> y = nodes->getY();
	Array<scalar> z = nodes->getZ();
	for (int i = 0; i < num; ++i)
	{
		x_.push_back(x[i]);
		y_.push_back(y[i]);
		z_.push_back(z[i]);

	}
	xyz_.num = num;
	xyz_.startIdx = new label[num+1];
	xyz_.startIdx[0] = 0;
	xyz_.data = new scalar[num*3];
	for (int i = 0; i < num; ++i)
	{
		xyz_.startIdx[i+1] = xyz_.startIdx[i]+3;
		xyz_.data[i*3+0] = x_[i];
		xyz_.data[i*3+1] = y_[i];
		xyz_.data[i*3+2] = z_[i];
	}
	// this->refCount_ = nodes->refCount_;
	// this->refCount_->incRefCount();
}

void Nodes::add(Nodes* nodes)
{
	int num = nodes->size();
	Array<scalar> x = nodes->getX();
	Array<scalar> y = nodes->getY();
	Array<scalar> z = nodes->getZ();
	for (int i = 0; i < num; ++i)
	{
		x_.push_back(x[i]);
		y_.push_back(y[i]);
		z_.push_back(z[i]);

	}
	xyz_.num += num;
	label* tmp = xyz_.startIdx;
	xyz_.startIdx = new label[xyz_.num+1];
	for (int i = 0; i < xyz_.num-num; ++i)
	{
		xyz_.startIdx[i] = tmp[i];
	}
	if(tmp)
	{
		DELETE_POINTER(tmp);
	}
	else 
	{
		xyz_.startIdx[0] = 0;
	}
	xyz_.data = new scalar[xyz_.num*SOL_DIM];
	for (int i = xyz_.num-num; i < xyz_.num; ++i)
	{
		xyz_.startIdx[i+1] = xyz_.startIdx[i]+SOL_DIM;
		xyz_.data[i*SOL_DIM+0] = x_[i];
		xyz_.data[i*SOL_DIM+1] = y_[i];
		xyz_.data[i*SOL_DIM+2] = z_[i];
	}
}

const Array<scalar>& Nodes::getX()
{
	return this->x_;
}

const Array<scalar>& Nodes::getY()
{
	return this->y_;
}

const Array<scalar>& Nodes::getZ()
{
	return this->z_;
}

const ArrayArray<scalar>& Nodes::getXYZ()
{
	return this->xyz_;
}

const scalar* Nodes::getXYZ(label idx)
{
	return &this->xyz_.data[this->xyz_.startIdx[idx]];
}


} // end namespace HSF
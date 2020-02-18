/**
* @file: compute.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-11-28 10:39:09
* @last Modified by:   lenovo
* @last Modified time: 2019-11-28 10:57:11
*/
#ifndef COMPUTE_HPP
#define COMPUTE_HPP

#include <cmath>
#include "utilities.hpp"

namespace HSF
{

scalar calculateQUADArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z);

scalar calculateTRIArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z);

scalar calculateFaceArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes)
{
	switch(nnodes)
	{
		case 3: return calculateTRIArea(x, y, z);
		case 4: return calculateQUADArea(x, y, z);
		default:
			Terminate("calculateFaceArea", "unrecognized face type");
	}
};


}

#endif
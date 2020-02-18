/**
* @file: geometry.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-11-28 10:57:45
* @last Modified by:   lenovo
* @last Modified time: 2020-01-06 17:44:22
*/

#include "geometry.hpp"

namespace HSF
{

scalar calculateQUADArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z)
{
	scalar v1[3], v2[3], v3[3];

    /// v1 = coord(1)-coord(0)
    v1[0] = x[1] - x[0];
    v1[1] = y[1] - y[0];
    v1[2] = z[1] - z[0];

    /// v2 = coord(3)-coord(0)
    v2[0] = x[3] - x[0];
    v2[1] = y[3] - y[0];
    v2[2] = z[3] - z[0];

    /// v3 = v1xv2
    v3[0] = v1[1]*v2[2]-v2[1]*v1[2];
    v3[1] = v1[2]*v2[0]-v2[2]*v1[0];
    v3[2] = v1[0]*v2[1]-v2[0]*v1[1];
    // v3(1) = v1(2)*v2(3)-v2(2)*v1(3)
    // v3(2) = v1(3)*v2(1)-v1(1)*v2(3)
    // v3(3) = v1(1)*v2(2)-v2(1)*v1(2)

    return sqrt(v3[0]*v3[0]+v3[1]*v3[1]+v3[2]*v3[2]);
}

scalar calculateTRIArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z)
{
	return calculateQUADArea(x, y, z)/2;
}

scalar calculateFaceArea(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes)
{
	switch(nnodes)
	{
		case 3: return calculateTRIArea(x, y, z); break;
		case 4: return calculateQUADArea(x, y, z); break;
		default:
			Terminate("calculateFaceArea", "unrecognized face type");
	}
}

void calculateQUADNormVec(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, scalar* normVec)
{
	scalar v1[3], v2[3], v3[3];

	// for (int i = 0; i < x.size(); ++i)
	// {
	// 	printf("%f, %f, %f\n", x[i], y[i], z[i]);
	// }
    /// v1 = coord(1)-coord(0)
    v1[0] = x[1] - x[0];
    v1[1] = y[1] - y[0];
    v1[2] = z[1] - z[0];

    /// v2 = coord(3)-coord(0)
    v2[0] = x[3] - x[0];
    v2[1] = y[3] - y[0];
    v2[2] = z[3] - z[0];	

    /// v3 = v1xv2
    v3[0] = v1[1]*v2[2]-v2[1]*v1[2];
    v3[1] = v1[2]*v2[0]-v2[2]*v1[0];
    v3[2] = v1[0]*v2[1]-v2[0]*v1[1];

    scalar norm = sqrt(v3[0]*v3[0]+v3[1]*v3[1]+v3[2]*v3[2]);
    normVec[0] = v3[0]/norm;
    normVec[1] = v3[1]/norm;
    normVec[2] = v3[2]/norm;
    // printf("normVec: %f, %f, %f\n", normVec[0], normVec[1], normVec[2]);
}

void calculateTRINormVec(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, scalar* normVec)
{
	scalar v1[3], v2[3], v3[3];

    /// v1 = coord(1)-coord(0)
    v1[0] = x[1] - x[0];
    v1[1] = y[1] - y[0];
    v1[2] = z[1] - z[0];

    /// v2 = coord(2)-coord(0)
    v2[0] = x[2] - x[0];
    v2[1] = y[2] - y[0];
    v2[2] = z[2] - z[0];	

    /// v3 = v1xv2
    v3[0] = v1[1]*v2[2]-v2[1]*v1[2];
    v3[1] = v1[2]*v2[0]-v2[2]*v1[0];
    v3[2] = v1[0]*v2[1]-v2[0]*v1[1];

    scalar norm = sqrt(v3[0]*v3[0]+v3[1]*v3[1]+v3[2]*v3[2]);
    normVec[0] = v3[0]/norm;
    normVec[1] = v3[1]/norm;
    normVec[2] = v3[2]/norm;
    // printf("normVec: %f, %f, %f\n", normVec[0], normVec[1], normVec[2]);
}

void calculateFaceNormVec(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes, scalar* normVec)
{
	switch(nnodes)
	{
		case 3: calculateTRINormVec(x, y, z, normVec); break;
		case 4: calculateQUADNormVec(x, y, z, normVec); break;
		default:
			Terminate("calculateFaceNormVec", "unrecognized face type");
	}
}

void calculateFaceCenter(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes, scalar* center)
{
	center[0] = 0;
	center[1] = 0;
	center[2] = 0;
	for (int i = 0; i < nnodes; ++i)
	{
		center[0] += x[i];
		center[1] += y[i];
		center[2] += z[i];
	}
	center[0] /= nnodes;
	center[1] /= nnodes;
	center[2] /= nnodes;
}

scalar calculateCellVol(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes)
{
	switch(nnodes)
	{
		case 4: return calculateTETRAVol(x, y, z); break;
		/// TODO
		case 5: return calculateTETRAVol(x, y, z); break;
		case 8: return calculateHEXAVol(x, y, z); break;
		default:
			Terminate("calculateCellVol", "unrecognized cell type");
	}	
}

scalar calculateHEXAVol(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z)
{
	scalar v1[3], v2[3], v3[3];

	v1[0] = x[1] - x[0];
	v1[1] = y[1] - y[0];
	v1[2] = z[1] - z[0];

	v2[0] = x[3] - x[0];
	v2[1] = y[3] - y[0];
	v2[2] = z[3] - z[0];

	v3[0] = x[4] - x[0];
	v3[1] = y[4] - y[0];
	v3[2] = z[4] - z[0];

	scalar vol;

    vol =     v3[0]*(v1[1]*v2[2]-v2[1]*v1[2]);
    vol = vol+v3[1]*(v1[2]*v2[0]-v1[0]*v2[2]);
    vol = vol+v3[2]*(v1[0]*v2[1]-v2[0]*v1[1]);

    return vol;	
}

scalar calculateTETRAVol(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z)
{
	scalar v1[3], v2[3], v3[3];

	v1[0] = x[1] - x[0];
	v1[1] = y[1] - y[0];
	v1[2] = z[1] - z[0];

	v2[0] = x[2] - x[0];
	v2[1] = y[2] - y[0];
	v2[2] = z[2] - z[0];

	v3[0] = x[3] - x[0];
	v3[1] = y[3] - y[0];
	v3[2] = z[3] - z[0];

	scalar vol;

    vol =     v3[0]*(v1[1]*v2[2]-v2[1]*v1[2]);
    vol = vol+v3[1]*(v1[2]*v2[0]-v1[0]*v2[2]);
    vol = vol+v3[2]*(v1[0]*v2[1]-v2[0]*v1[1]);
    vol = vol/6;

    return vol;	
}

void calculateCellCenter(const Array<scalar>& x, const Array<scalar>& y,
	const Array<scalar>& z, label nnodes, scalar* center)
{
	calculateFaceCenter(x, y, z, nnodes, center);
}

}
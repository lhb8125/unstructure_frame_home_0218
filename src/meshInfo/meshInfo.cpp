/**
* @file: meshInfo.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-11-28 09:25:37
* @last Modified by:   lenovo
* @last Modified time: 2019-11-28 16:16:03
*/

#include "meshInfo.hpp"
#include "geometry.hpp"

namespace HSF
{

// constructor
MeshInfo::MeshInfo()
:
scalarFieldTabPtr_(NULL),
labelFieldTabPtr_(NULL)
{
}

// deconstructor
MeshInfo::~MeshInfo()
{
}

void MeshInfo::init(Mesh& mesh)
{
	this->addFaceInfo(mesh);
	this->addCellInfo(mesh);
	this->getMeshInfo<scalar>("cell", "center");
}

void MeshInfo::addFaceInfo(Mesh& mesh)
{
	// printf("hello world\n");
	/// compute area of internal faces
	label faceNum = mesh.getTopology().getInnFacesNum();
	const ArrayArray<label> face2Node = mesh.getTopology().getFace2Node();
	Array<scalar> x = mesh.getOwnNodes().getX();
	Array<scalar> y = mesh.getOwnNodes().getY();
	Array<scalar> z = mesh.getOwnNodes().getZ();
	scalar* area    = new scalar[faceNum];
	scalar* normVec = new scalar[faceNum*3];
	scalar* center  = new scalar[faceNum*3];

	for (int i = 0; i < faceNum; ++i)
	{
		label nnodes = face2Node.startIdx[i+1]-face2Node.startIdx[i];
		Array<scalar> tmpX, tmpY, tmpZ;
		for (int j = face2Node.startIdx[i]; j < face2Node.startIdx[i+1]; ++j)
		{
			label idx = face2Node.data[j]-1;
			tmpX.push_back(x[idx]);
			tmpY.push_back(y[idx]);
			tmpZ.push_back(z[idx]);
		}
		area[i]  = calculateFaceArea(tmpX, tmpY, tmpZ, nnodes);
		calculateFaceNormVec(tmpX, tmpY, tmpZ, nnodes, &normVec[i*3]);
		calculateFaceCenter(tmpX, tmpY, tmpZ, nnodes, &center[i*3]);
		// printf("%f\n", area);
	}
	Field<scalar>* fnew = new Field<scalar>("face", 1, faceNum, area);
	this->addField<scalar>("face", "area", fnew);
	fnew = new Field<scalar>("face", 3, faceNum, normVec);
	this->addField<scalar>("face", "normal vector", fnew);
	fnew = new Field<scalar>("face", 3, faceNum, center);
	this->addField<scalar>("face", "center", fnew);

	// for (int i = 0; i < faceNum; ++i)
	// {
	// 	printf("The %dth face: ", i);
	// 	for (int j = face2Node.startIdx[i]; j < face2Node.startIdx[i+1]; ++j)
	// 	{
	// 		printf("%d, ", face2Node.data[j]);
	// 	}
	// 	printf("\n");
	// }
}

void MeshInfo::addCellInfo(Mesh& mesh)
{
	// printf("hello world\n");
	/// compute area of internal faces
	label cellNum = mesh.getTopology().getCellsNum();
	const ArrayArray<label> cell2Node = mesh.getTopology().getCell2Node();
	Array<scalar> x = mesh.getOwnNodes().getX();
	Array<scalar> y = mesh.getOwnNodes().getY();
	Array<scalar> z = mesh.getOwnNodes().getZ();
	scalar* vol    = new scalar[cellNum];
	scalar* center = new scalar[cellNum*3];
	for (int i = 0; i < cellNum; ++i)
	{
		label nnodes = cell2Node.startIdx[i+1]-cell2Node.startIdx[i];
		Array<scalar> tmpX, tmpY, tmpZ;
		for (int j = cell2Node.startIdx[i]; j < cell2Node.startIdx[i+1]; ++j)
		{
			label idx = cell2Node.data[j]-1;
			tmpX.push_back(x[idx]);
			tmpY.push_back(y[idx]);
			tmpZ.push_back(z[idx]);
		}
		vol[i]  = calculateCellVol(tmpX, tmpY, tmpZ, nnodes);
		calculateCellCenter(tmpX, tmpY, tmpZ, nnodes, &center[i*3]);
	}
	Field<scalar>* fnew = new Field<scalar>("cell", 1, cellNum, vol);
	this->addField<scalar>("cell", "volume", fnew);
	fnew = new Field<scalar>("cell", 3, cellNum, center);
	this->addField<scalar>("cell", "center", fnew);
}

template<typename T>
T* MeshInfo::getMeshInfo(Word setType, Word name)
{
	Field<T>& fieldI = this->getField<T>(setType, name);
	// T* tmp = fieldI.getLocalData();
	// label dims = fieldI.getDim();
	// printf("%s\n", name.c_str());
	// for (int i = 0; i < fieldI.getSize(); ++i)
	// {
	// 	std::cout<<i<<": ";
	// 	for (int j = 0; j < dims; ++j)
	// 	{
	// 		std::cout<<tmp[i*dims+j]<<" ";
	// 	}
	// 	std::cout<<std::endl;
	// }
	return fieldI.getLocalData();
}

}
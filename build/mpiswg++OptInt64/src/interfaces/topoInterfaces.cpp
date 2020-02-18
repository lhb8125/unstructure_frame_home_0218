/**
* @file: interfaces.hpp
* @author: Liu Hongbin
* @brief: fortran function interfaces
* @date:   2019-11-11 10:56:28
* @last Modified by:   lenovo
* @last Modified time: 2020-01-07 10:31:19
*/
#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <unistd.h>
#include "cstdlib"
#include "mpi.h"
#include "parameter.hpp"
#include "topoInterfaces.hpp"
#include "loadBalancer.hpp"

using namespace HSF;

#define REGION regs[0]
//extern "C"
//{
/**
* @brief fortran function interfaces
*/
/*****************************************************************************/

void init_(char* configFile)
{
	LoadBalancer *lb = new LoadBalancer();
	// std::cout<<"start initializing ......"<<std::endl;

	para.setParaFile(configFile);

	// printf("%s\n", configFile);

	// int numproces, rank;
	// MPI_Init(NULL, NULL);
	// MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// MPI_Comm_size(MPI_COMM_WORLD, &numproces);

	int nPara = 4;
	// char meshFile[100];
	Array<char*> mesh_file(10);
	for (int i = 0; i < mesh_file.size(); ++i)
	{
		mesh_file[i] = new char[CHAR_DIM];
	}
	// para.getPara(&nPara, meshFile, "char*", "domain1", "region", "0", "path");
	para.getPara<char>(mesh_file, nPara, "domain1", "region", "0", "path");
	char resultFile[CHAR_DIM];
	para.getPara<char>(resultFile, nPara, "domain1", "region", "0", "resPath");

	/// initialization before load balance
	Region reg;
	// regs.resize(1);
	regs.push_back(reg);
	REGION.initBeforeBalance(mesh_file);

	/// load balance in region
	lb->LoadBalancer_3(regs);

	/// initialization after load balance
	REGION.initAfterBalance();

}

void clear_()
{
	regs.clear();
}

void get_elements_num_(label* eleNum)
{
	eleNum[0] = REGION.getMesh().getTopology().getCellsNum();
}


void get_faces_num_(label* faceNum)
{
	faceNum[0] = REGION.getMesh().getTopology().getFacesNum();
}


void get_inner_faces_num_(label* innFaceNum)
{
	innFaceNum[0] = REGION.getMesh().getTopology().getInnFacesNum();
}


void get_bnd_faces_num_(label* bndFaceNum)
{
	bndFaceNum[0] = REGION.getMesh().getTopology().getBndFacesNum();
}


void get_nodes_num_(label* nodeNum)
{
	nodeNum[0] = REGION.getMesh().getOwnNodes().size();
}

/*****************************************************************************/

void get_ele_2_face_pos_(label* pos)
{
	label* tmp = REGION.getMesh().getTopology().getCell2Face().startIdx;
	label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_ele_2_face_(label* ele2Face)
{
	label* tmpData = REGION.getMesh().getTopology().getCell2Face().data;
	label* tmp = REGION.getMesh().getTopology().getCell2Face().startIdx;
	label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			ele2Face[j] = tmpData[j]+1;
		}
	}
}


void get_ele_2_ele_pos_(label* pos)
{
	label* tmp = REGION.getMesh().getTopology().getCell2Cell().startIdx;
	label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_ele_2_ele_(label* ele2Ele)
{
	label* tmpData = REGION.getMesh().getTopology().getCell2Cell().data;
	label* tmp = REGION.getMesh().getTopology().getCell2Cell().startIdx;
	label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			ele2Ele[j] = tmpData[j];
		}
	}
}


void get_ele_2_node_pos_(label* pos)
{
	label* tmp = REGION.getMesh().getTopology().getCell2Node().startIdx;
	label eleNum = REGION.getMesh().getTopology().getCellsNum();
	for (int i = 0; i < eleNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_ele_2_node_(label* ele2Node)
{
	label* tmpData = REGION.getMesh().getTopology().getCell2Node().data;
	label* tmp = REGION.getMesh().getTopology().getCell2Node().startIdx;
	label eleNum = REGION.getMesh().getTopology().getCellsNum();
	Table<label, label> coordMap = REGION.getMesh().getCoordMap();
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			ele2Node[j] = coordMap[tmpData[j]-1]+1;
		}
	}
}


void get_inn_face_2_node_pos_(label* pos)
{
	label* tmp = REGION.getMesh().getTopology().getFace2Node().startIdx;
	label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_inn_face_2_node_(label* face2Node)
{
	label* tmpData = REGION.getMesh().getTopology().getFace2Node().data;
	label* tmp = REGION.getMesh().getTopology().getFace2Node().startIdx;
	label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	Table<label, label> coordMap = REGION.getMesh().getCoordMap();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Node[j] = coordMap[tmpData[j]-1]+1;
		}
	}
}


void get_bnd_face_2_node_pos_(label* pos)
{
	label* tmp = REGION.getBoundary().getTopology().getFace2Node().startIdx;
	label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_bnd_face_2_node_(label* face2Node)
{
	label* tmpData = REGION.getBoundary().getTopology().getFace2Node().data;
	label* tmp = REGION.getBoundary().getTopology().getFace2Node().startIdx;
	label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Node[j] = tmpData[j];
		}
	}
}


void get_inn_face_2_ele_pos_(label* pos)
{
	label* tmp = REGION.getMesh().getTopology().getFace2Cell().startIdx;
	label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_inn_face_2_ele_(label* face2Ele)
{
	label* tmpData = REGION.getMesh().getTopology().getFace2Cell().data;
	label* tmp = REGION.getMesh().getTopology().getFace2Cell().startIdx;
	label faceNum = REGION.getMesh().getTopology().getInnFacesNum();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Ele[j] = tmpData[j]+1;
		}
	}
}


void get_bnd_face_2_ele_pos_(label* pos)
{
	label* tmp = REGION.getBoundary().getTopology().getFace2Cell().startIdx;
	label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum+1; ++i)
	{
		pos[i] = tmp[i]+1;
	}
}
void get_bnd_face_2_ele_(label* face2Ele)
{
	label* tmpData = REGION.getBoundary().getTopology().getFace2Cell().data;
	label* tmp = REGION.getBoundary().getTopology().getFace2Cell().startIdx;
	label faceNum = REGION.getBoundary().getTopology().getFacesNum();
	for (int i = 0; i < faceNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face2Ele[j] = tmpData[j]+1;
		}
	}
}

/******************************************************************************/
void get_ele_type_(label* eleType)
{
	Array<label> tmp = REGION.getMesh().getTopology().getCellType();
	for (int i = 0; i < tmp.size(); ++i)
	{
		eleType[i] = tmp[i];
	}
}

void get_coords_(scalar* coords)
{
	scalar* tmpData = REGION.getMesh().getOwnNodes().getXYZ().data;
	label* tmp     = REGION.getMesh().getOwnNodes().getXYZ().startIdx;
	label nodeNum = REGION.getMesh().getOwnNodes().size();
	for (int i = 0; i < nodeNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			coords[j] = tmpData[j];
		}
	}
}

/*****************************************************************************/

// }
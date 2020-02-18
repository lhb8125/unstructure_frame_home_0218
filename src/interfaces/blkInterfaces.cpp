/**
* @file: blkInterfaces.cpp
* @author: Liu Hongbin
* @brief: block topology interfaces
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
#include "blkInterfaces.hpp"
// #include "loadBalancer.hpp"

using namespace HSF;

#define REGION regs[0]

void get_ele_block_num_(label *ele_blk_num)
{
	ele_blk_num[0] = REGION.getMesh().getBlockTopology().getCellType().size();
}

void get_ele_blk_pos_(label *ele_blk_pos)
{
	Array<label> tmp
		= REGION.getMesh().getBlockTopology().getCellBlockStartIdx();
	for (int i = 0; i < tmp.size(); ++i)
	{
		ele_blk_pos[i] = tmp[i]+1;
	}
}

void get_ele_blk_type_(label *ele_blk_type)
{
	Array<label> tmp
		= REGION.getMesh().getBlockTopology().getCellType();
	for (int i = 0; i < tmp.size(); ++i)
	{
		ele_blk_type[i] = tmp[i];
	}
}

void get_ele_2_ele_blk_(label *ele_2_ele)
{
	label* tmp
		= REGION.getMesh().getBlockTopology().getCell2Cell().startIdx;
	label* tmpData
		= REGION.getMesh().getBlockTopology().getCell2Cell().data;
	label eleNum
		= REGION.getMesh().getBlockTopology().getCell2Cell().num;
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[j+1]; ++j)
		{
			ele_2_ele[j] = tmpData[j]+1;
		}
	}	
}

void get_ele_2_face_blk_(label *ele_2_face)
{
	label* tmp
		= REGION.getMesh().getBlockTopology().getCell2Face().startIdx;
	label* tmpData
		= REGION.getMesh().getBlockTopology().getCell2Face().data;
	label eleNum
		= REGION.getMesh().getBlockTopology().getCell2Face().num;
	for (int i = 0; i < eleNum; ++i)
	{
		for (int j = tmp[i]; j < tmp[j+1]; ++j)
		{
			ele_2_face[j] = tmpData[j]+1;
		}
	}	
}

void get_ele_2_node_blk_(label *ele_2_node)
{
	label* tmp
		= REGION.getMesh().getBlockTopology().getCell2Node().startIdx;
	label* tmpData
		= REGION.getMesh().getBlockTopology().getCell2Node().data;
	label eleNum
		= REGION.getMesh().getBlockTopology().getCell2Node().num;
	Table<label, label> coordMap = REGION.getMesh().getCoordMap();
	for (int i = 0; i < eleNum; ++i)
	{
		// printf("The %dth element from %d to %d: ", i, tmp[i], tmp[i+1]);
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			ele_2_node[j] = coordMap[tmpData[j]-1]+1;
			// printf("%d, ", j);
		}
		// printf("\n");
	}	
}

void get_face_block_num_(label *face_blk_num)
{
	face_blk_num[0] = REGION.getMesh().getBlockTopology().getFaceType().size();
}

void get_face_blk_pos_(label *face_blk_pos)
{
	Array<label> tmp
		= REGION.getMesh().getBlockTopology().getFaceBlockStartIdx();
	for (int i = 0; i < tmp.size(); ++i)
	{
		face_blk_pos[i] = tmp[i]+1;
	}
}

void get_face_blk_type_(label *face_blk_type)
{
	Array<label> tmp
		= REGION.getMesh().getBlockTopology().getFaceType();
	for (int i = 0; i < tmp.size(); ++i)
	{
		face_blk_type[i] = tmp[i];
	}
}

void get_face_2_node_blk_(label *face_2_node)
{
	label* tmp
		= REGION.getMesh().getBlockTopology().getFace2Node().startIdx;
	label* tmpData
		= REGION.getMesh().getBlockTopology().getFace2Node().data;
	label eleNum
		= REGION.getMesh().getBlockTopology().getFace2Node().num;
	Table<label, label> coordMap = REGION.getMesh().getCoordMap();
	for (int i = 0; i < eleNum; ++i)
	{
		// printf("The %dth element from %d to %d: ", i, tmp[i], tmp[i+1]);
		for (int j = tmp[i]; j < tmp[i+1]; ++j)
		{
			face_2_node[j] = coordMap[tmpData[j]-1]+1;
			// printf("%d, ", j);
		}
		// printf("\n");
	}	
}
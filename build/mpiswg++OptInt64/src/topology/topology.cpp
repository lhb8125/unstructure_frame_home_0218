/**
* @file: topology.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-14 16:27:19
* @last Modified by:   lenovo
* @last Modified time: 2020-01-09 16:55:39
*/
#include <cstdio>
#include <assert.h>
#include "mpi.h"
#include "topology.hpp"
#include "section.hpp"
#include "loadBalancer.hpp"

namespace HSF
{

/**
* @brief constructor
*/
Topology::Topology()
{
}

/**
* @brief deconstructor
*/
Topology::~Topology()
{
	face2NodeBnd_.clear();
	face2CellPatch_.clear();
	face2CellBnd_.clear();
	cellType_.clear();
	faceType_.clear();
}

/**
* @brief Construct from the connectivity after load balance
*/
void Topology::constructTopology()
{
	int rank,nprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	ArrayArray<label> cell2Node = this->cell2Node_;
	// printf("cell2Node_: %d, cellType_: %d\n", cell2Node_.size(), cellType_.size());
	// if(rank==0)
	// {
	// for (int i = 0; i < cell2Node.num; ++i)
	// {
	// 	printf("The %dth element: ", i+cellStartId_);
	// 	for (int j = cell2Node.startIdx[i]; j < cell2Node.startIdx[i+1]; ++j)
	// 	{
	// 		printf("%d, ", cell2Node.data[j]);
	// 	}
	// 	printf("\n");
	// }
	// }
	label cellNum = cell2Node.size();
	this->cellNum_ = cell2Node.size();
	label *cellStartIdTmp = new label[nprocs+1];
	MPI_Allgather(&cellNum, 1, MPI_LABEL, &cellStartIdTmp[1], 1, MPI_LABEL, MPI_COMM_WORLD);
	cellStartIdTmp[0] = 0;
	for (int i = 1; i <= nprocs; ++i)
	{
		cellStartIdTmp[i] += cellStartIdTmp[i-1];
	}
	this->cellStartId_ = cellStartIdTmp[rank];
	Array<Array<label> > faces2NodesTmp;
	Array<Array<label> > cell2Cell(cellNum);
	for (int i = 0; i < cellNum; ++i)
	{
		label faceNumTmp = Section::facesNumForEle(cellType_[i]);
		// printf("%d, %d, %d\n", rank, i, cellType_[i]);
		for (int j = 0; j < faceNumTmp; ++j)
		{
			Array<label> face2NodeTmp = Section::faceNodesForEle(
				&cell2Node.data[cell2Node.startIdx[i]], cellType_[i], j);
			sort(face2NodeTmp.begin(), face2NodeTmp.end());
			face2NodeTmp.push_back(i+cellStartId_);
			faces2NodesTmp.push_back(face2NodeTmp);
		}
	}
	// printf("faceNum: %d, cellNum: %d\n", faces2NodesTmp.size(), cell2Cell.size());
	// 对所有面单元进行排序
	Array<label> weight;
	for (int i = 0; i < faces2NodesTmp.size(); ++i)
	{
		weight.push_back(faces2NodesTmp[i][0]);
	}
	sort(faces2NodesTmp, weight);
	// quicksortArray(faces2NodesTmp, 0, faces2NodesTmp.size()-1);
	// 由面与node之间关系寻找cell与cell之间关系
	// 删除重复的内部面
	// 找到面与cell之间关系
	bool* isInner = new bool[faces2NodesTmp.size()];
	Array<Array<label> > face2NodeInn, face2NodeBnd;
	Array<Array<label> > face2CellInn, face2CellBnd;
	Array<Array<label> > cell2CellArr(cellNum);
	for (int i = 0; i < faces2NodesTmp.size(); ++i){isInner[i] = false;}
	for (int i = 0; i < faces2NodesTmp.size(); ++i)
	{
		if(isInner[i]) continue;
		int end = i+1;
		// 默认两个面不相等
		bool isEqual = false;
		// par_std_out_("%d, ", i);
		while(end<faces2NodesTmp.size() && 
			faces2NodesTmp[i][0] == faces2NodesTmp[end][0])
		{
			// 第一个node相等时，默认相等
			isEqual = true;
			// 如果维度不相等，则两个面不相等
			if(faces2NodesTmp[i].size()!=faces2NodesTmp[end].size())
			{
				isEqual = false;
				break;
			}
			// 比较各个维度，不相等则跳出，标记不相等
			for (int j = 0; j < faces2NodesTmp[i].size()-1; ++j)
			{
				if(faces2NodesTmp[i][j]!=faces2NodesTmp[end][j])
				{
					isEqual = false;
					break;
				}
			}
			if(isEqual)
			{
				// 本面对应cell编号为owner，相等面对应cell编号为neighbor
				label ownerId 
					= faces2NodesTmp[i][faces2NodesTmp[i].size()-1]-cellStartId_;
				label neighborId
					= faces2NodesTmp[end][faces2NodesTmp[end].size()-1]-cellStartId_;
				cell2CellArr[ownerId].push_back(neighborId);
				cell2CellArr[neighborId].push_back(ownerId);
				// 记录面单元左右cell编号
				Array<label> face2CellTmp;
				face2CellTmp.push_back(ownerId+cellStartId_);
				face2CellTmp.push_back(neighborId+cellStartId_);
				face2CellInn.push_back(face2CellTmp);
				face2NodeInn.push_back(faces2NodesTmp[i]);
				// 删除相等面
				// faces2NodesTmp.erase(end+faces2NodesTmp.begin());
				isInner[i] = true;
				isInner[end] = true;
				// face2CellTmp.push_back(-1);
				break;
			}
			end++;
		}
		label cellId = faces2NodesTmp[i][faces2NodesTmp[i].size()-1];
		// 删除面单元中最后一个元素，即cell编号
		faces2NodesTmp[i].pop_back();
		// 记录边界面单元对应cell编号，外部进程单元记-1
		if(!isEqual)
		{
			Array<label> face2CellTmp;
			face2CellTmp.push_back(cellId);
			face2CellTmp.push_back(-1);
			face2CellBnd.push_back(face2CellTmp);
			face2NodeBnd.push_back(faces2NodesTmp[i]);
		}
	}

	// par_std_out_("start setting patch infomation ...\n");
	setPatchInfo(face2NodeInn, face2NodeBnd, face2CellInn, face2CellBnd);
	// par_std_out_("finish setting patch infomation ...\n");

	// localization of cell index
	for (int i = 0; i < face2CellInn.size(); ++i)
	{
		for (int j = 0; j < face2CellInn[i].size(); ++j)
		{
			face2CellInn[i][j] -= cellStartId_;
		}
	}

	// 根据节点个数确定网格面类型，并将网格面分开
	Array<label> faceNodeNum;
	for (int i = 0; i < face2NodeInn.size(); ++i)
	{
		bool isExist = false;
		// 最后一个元素为网格单元编号，删去
		face2NodeInn[i].pop_back();
		for (int j = 0; j < faceNodeNum.size(); ++j)
		{
			if(face2NodeInn[i].size()==faceNodeNum[j])
			{
				isExist=true;
				break;
			}
		}
		if(!isExist)
		{ 
			faceNodeNum.push_back(face2NodeInn[i].size());
		}
	}
	// 根据网格面类型对网格面拓扑进行重排，使相同类型网格面连续存储
	Array<Array<Array<label> > > face2NodeWithType(faceNodeNum.size());
	Array<Array<Array<label> > > face2CellWithType(faceNodeNum.size());
	for (int i = 0; i < face2NodeInn.size(); ++i)
	{
		bool isExist = false;
		for (int j = 0; j < faceNodeNum.size(); ++j)
		{
			if(face2NodeInn[i].size()==faceNodeNum[j])
			{
				isExist=true;
				face2NodeWithType[j].push_back(face2NodeInn[i]);
				face2CellWithType[j].push_back(face2CellInn[i]);
			}
		}
		if(!isExist) Terminate("sort the faces according to the type", "unrecognized type");
	}
	// printf("There are %d types of faces, and we have %d faces\n", faceNodeNum.size(), face2NodeInn.size());
	face2NodeInn.clear();
	face2CellInn.clear();
	for (int i = 0; i < faceNodeNum.size(); ++i)
	{
		face2NodeInn.insert(face2NodeInn.end(), face2NodeWithType[i].begin(), face2NodeWithType[i].end());
		face2CellInn.insert(face2CellInn.end(), face2CellWithType[i].begin(), face2CellWithType[i].end());
	}
	transformArray(face2CellInn, this->face2Cell_);
	this->faceNum_i_ = face2NodeInn.size();
	this->faceNum_b_ = face2NodeBnd.size();
	this->faceNum_   = this->faceNum_i_+this->faceNum_b_;

	// 将重排后的face2node拓扑关系复位，原始顺序从cell2node中获取
	for (int i = 0; i < face2NodeInn.size(); ++i)
	{
		label cellIdx = face2CellInn[i][0];
		// printf("2: %d, %d, %d\n", rank, cellIdx, cellType_[cellIdx]);
		label faceNumTmp = Section::facesNumForEle(cellType_[cellIdx]);
		// printf("2: %d, %d, %d\n", rank, cellIdx, cellType_[cellIdx]);
		for (int j = 0; j < faceNumTmp; ++j)
		{
			Array<label> face2NodeTmp = Section::faceNodesForEle(
				&cell2Node.data[cell2Node.startIdx[cellIdx]],
				cellType_[cellIdx], j);
			Array<label> tmp;
			tmp.assign(face2NodeTmp.begin(), face2NodeTmp.end());
			sort(face2NodeTmp.begin(), face2NodeTmp.end());
			if(compareArray(face2NodeTmp, face2NodeInn[i]))
				face2NodeInn[i].swap(tmp);
		}
	}
	transformArray(face2NodeInn, this->face2Node_);
	// if(rank==0)
	// {
	// 	for (int i = 0; i < face2NodeInn.size(); ++i)
	// 	{
	// 		printf("The %dth face: ", i);
	// 		for (int j = 0; j < face2NodeInn[i].size(); ++j)
	// 		{
	// 			printf("%d, ", face2NodeInn[i][j]);
	// 		}
	// 		printf("\n");
	// 	}
	// }

	// 获取cell2face的拓扑关系
	Array<Array<label> > cell2FaceArr(cell2CellArr.size());
	for (int i = 0; i < face2CellInn.size(); ++i)
	{
		for (int j = 0; j < face2CellInn[i].size(); ++j)
		{
			cell2FaceArr[face2CellInn[i][j]].push_back(i);
		}
	}
	// for (int i = 0; i < face2CellBnd.size(); ++i)
	// {
	// 	cell2FaceArr[face2CellBnd[i][0]-cellStartId_].push_back(i+faceNum_i_);
	// }
	transformArray(cell2FaceArr, this->cell2Face_);
	// if(rank==0)
	// {
	// 	for (int i = 0; i < cell2FaceArr.size(); ++i)
	// 	{
	// 		printf("The %dth element: ", i);
	// 		for (int j = 0; j < cell2FaceArr[i].size(); ++j)
	// 		{
	// 			printf("%d, ", cell2FaceArr[i][j]);
	// 		}
	// 		printf("\n");
	// 	}
	// }

	// printf("There are %d types of faces, and we have %d faces\n", faceNodeNum.size(), face2NodeInn.size());
	this->face2CellBnd_.swap(face2CellBnd);
	transformArray(cell2CellArr, this->cell2Cell_);
	this->face2NodeBnd_.swap(face2NodeBnd);

	this->genEdgeTopo();

	DELETE_POINTER(cellStartIdTmp);
	DELETE_POINTER(isInner);
}

void Topology::setPatchInfo(Array<Array<label> > face2NodeInn,
	Array<Array<label> > face2NodeBnd, Array<Array<label> > face2CellInn,
	Array<Array<label> > face2CellBnd)
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	Array<Array<label> > face2CellArr, face2NodeArr;
	face2CellArr.clear();
	face2CellArr.insert(face2CellArr.end(), face2CellBnd.begin(), face2CellBnd.end());
	face2CellArr.insert(face2CellArr.end(), face2CellInn.begin(), face2CellInn.end());
	// printf("%d, %d, %d\n", face2CellArr.size(), face2CellInn.size(), face2CellBnd.size());
	face2NodeArr.clear();
	face2NodeArr.insert(face2NodeArr.end(), face2NodeBnd.begin(), face2NodeBnd.end());
	face2NodeArr.insert(face2NodeArr.end(), face2NodeInn.begin(), face2NodeInn.end());

	Array<label> face2CellTmp;
	for (int i = 0; i < face2CellArr.size(); ++i)
	{
		face2CellTmp.push_back(face2CellArr[i][0]);
		// 对于内部面，令其为-1
		if(face2CellArr[i][1]!=-1) face2CellTmp[i] = -1;
	}
	ArrayArray<label> face2NodeBndTmp;
	transformArray(face2NodeBnd, face2NodeBndTmp);
	// if(rank==1)
	// {
	// 	for (int i = 0; i < face2NodeBndTmp.size(); ++i)
	// 	{
	// 		printf("%d, %d, %d\n", i, face2NodeBndTmp.startIdx[i], face2NodeBndTmp.startIdx[i+1]);
	// 	}
	// }
	Array<label> face2CellNew;
	face2CellNew = LoadBalancer::collectNeighborCell(face2NodeBndTmp,
		face2NodeBnd, face2CellTmp);

	for (int i = 0; i < face2CellNew.size(); ++i)
	{
		if(face2CellNew[i]>=0)
		{
			Array<label> face2CellPatchTmp;
			face2CellPatchTmp.push_back(face2CellArr[i][0]);
			face2CellPatchTmp.push_back(face2CellNew[i]);
			face2CellPatch_.push_back(face2CellPatchTmp);
		}
	}
	// printf("rank %d, patch: %d\n", rank, face2CellPatch_.size());
	// if(rank==1)
	// {
	// 	for (int i = 0; i < face2CellPatch_.size(); ++i)
	// 	{
	// 		printf("The %dth face: %d, %d\n", i, face2CellPatch_[i][0], face2CellPatch_[i][1]);
	// 	}
	// }
}

void Topology::constructTopology(Array<Section>& secs)
{
	label secNum = secs.size();
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	/// record the type of cells
	cellNum_ = 0;
	Array<label> tmp;
	for (int i = 0; i < secNum; ++i)
	{
		cellNum_ += secs[i].num;
		// printf("%d, %d, %d\n", i, secs[i].num, secs[i].type);
		for (int j = 0; j < secs[i].num; ++j)
		{
			// printf("%d, %d, %d\n", j, secs[i].num, secs[i].type);
			this->cellType_.push_back(secs[i].type);
		}
	}

	// / construct the topology array: cell2Node
	cell2Node_.num = cellNum_;
	cell2Node_.startIdx = new label[cellNum_+1];
	cell2Node_.startIdx[0] = 0;
	label k=0;
	for (int i = 0; i < secNum; ++i)
	{
		for (int j = 0; j < secs[i].num; ++j)
		{
			cell2Node_.startIdx[k+1]
				= cell2Node_.startIdx[k] + Section::nodesNumForEle(secs[i].type);
			k++;
		}
	}
	cell2Node_.data = new label[cell2Node_.startIdx[cellNum_]];
	k=0;
	for (int i = 0; i < secNum; ++i)
	{
		int l = 0;
		for (int j = 0; j < secs[i].num; ++j)
		{
			memcpy(&cell2Node_.data[cell2Node_.startIdx[k]],
				&secs[i].conn[l],
				Section::nodesNumForEle(secs[i].type)*sizeof(label));
			k++;
			l += Section::nodesNumForEle(secs[i].type);
		}
	}

// for (int i = 0; i < cellNum_; ++i)
// {
// 	for (int j = cell2Node_.startIdx[i]; j < cell2Node_.startIdx[i+1]; ++j)
// 	{
// 		printf("%d, ", cell2Node_.data[j]);
// 	}
// 	printf("\n");
// }
	/// construct the topology array: face2Node
	// Array<Array<label> > faces2NodesBndTmp;
#if 0
	Array<Array<label> > faces2NodesTmp;
	for (int i = 0; i < cellNum_; ++i)
	{
		label faceNumTmp = Section::facesNumForEle(cellType_[i]);
		// if(rank==0) printf("cellIdx: %d, faceNumTmp: %d, cellType: %d\n", i, faceNumTmp, cellType_[i]);
		// if the face num equals to 0, then the cell is considered as face.
		// if(faceNumTmp==0)
		// {
		// 	Array<label> faceEleTmp;
		// 	for (int j = cell2Node_.startIdx[i]; j < cell2Node_.startIdx[i+1]; ++j)
		// 	{
		// 		faceEleTmp.push_back(cell2Node_.data[j]);
		// 	}
		// 	faces2NodesBndTmp.push_back(faceEleTmp);
		// }
		for (int j = 0; j < faceNumTmp; ++j)
		{
			faces2NodesTmp.push_back(
				Section::faceNodesForEle(
					&cell2Node_.data[cell2Node_.startIdx[i]], cellType_[i], j));
		}
		// assert(faceNumTmp==4);
	}
	// eliminate the duplicate faces and seperate the boundary face and internal face
	// faceNum_b_  = filterArray(faces2NodesBndTmp);
	label *faceNum = filterArray(faces2NodesTmp);
	faceNum_ = faceNum[0]+faceNum[1];
	faceNum_b_ = faceNum[0];
	faceNum_i_ = faceNum[1];
#endif
	// MPI_Send(&)
// for (int i = 0; i < faces2NodesTmp.size(); ++i)
// {
// 	printf("The %dth face: ", i);
// 	for (int j = 0; j < faces2NodesTmp[i].size(); ++j)
// 	{
// 		printf("%d, ", faces2NodesTmp[i][j]);
// 	}
// 	printf("\n");
// }
#if 0
	face2Node_ = transformArray(faces2NodesTmp);

	// construct the topology array: cell2Face
	Array<Array<label> > cells2FacesTmp;
	for (int i = 0; i < cellNum_; ++i)
	{
		Array<label> cell2FacesTmp;
		label faceIdx;
		label faceNumTmp = Section::facesNumForEle(cellType_[i]);
		for (int j = 0; j < faceNumTmp; ++j)
		{
			Array<label> face2NodesTmp
				= Section::faceNodesForEle(
					&cell2Node_.data[cell2Node_.startIdx[i]], cellType_[i], j);
			sort(face2NodesTmp.begin(), face2NodesTmp.end());
			/// find the index of face matching this cell
			bool findFace = false;
			// printf("faceNum_: %d\n", faceNum_);
			for (int k = 0; k < faceNum_; ++k)
			{
				// printf("The %dth face\n", k);
				if(compareArray(faces2NodesTmp[k], face2NodesTmp))
				{
					findFace = true;
					faceIdx = k;
					// if(rank==0) printf("The matching face index of element %d is %d\n", i, faceIdx);
				}
			}
			if(!findFace) 
			{
				printf("%d\n", i);
				Terminate("find faces of elements", "can not find the matching faces");
			}
			cell2FacesTmp.push_back(faceIdx);
		}
		cells2FacesTmp.push_back(cell2FacesTmp);
	}
	cell2Face_ = transformArray(cells2FacesTmp);
#endif
// for (int i = 0; i < cell2Face_.num; ++i)
// {
// 	printf("The %dth cell: ", i);
// 	for (int j = cell2Face_.startIdx[i]; j < cell2Face_.startIdx[i+1]; ++j)
// 	{
// 		printf("%d, ", cell2Face_.data[j]);
// 	}
// 	printf("\n");
// }
#if 0
	// construct the topology array: face2Cell
	face2Cell_.startIdx = new label[faceNum_+1];
	face2Cell_.num = faceNum_;
	face2Cell_.data = new label[faceNum_i_*2+faceNum_b_];
	face2Cell_.startIdx[0] = 0;
	label *bonus = new label[faceNum_];
	for (int i = 0; i < faceNum_; ++i) { bonus[i]=0; }
	for (int i = 0; i < faceNum_b_; ++i)
	{
		face2Cell_.startIdx[i+1] = face2Cell_.startIdx[i]+1;
	}
	for (int i = faceNum_b_; i < faceNum_b_+faceNum_i_; ++i)
	{
		face2Cell_.startIdx[i+1] = face2Cell_.startIdx[i]+2;
	}
	for (int i = 0; i < cellNum_; ++i)
	{
		for (int j = cell2Face_.startIdx[i]; j < cell2Face_.startIdx[i+1]; ++j)
		{
			label idx = cell2Face_.data[j];
			/// if the face is boundary face, then ignore it
			face2Cell_.data[face2Cell_.startIdx[idx]+bonus[idx]] = i;
			bonus[idx]++;
		}
	}
#endif
// for (int i = 0; i < face2Cell_.num; ++i)
// {
// 	printf("The %dth face: ", i);
// 	for (int j = face2Cell_.startIdx[i]; j < face2Cell_.startIdx[i+1]; ++j)
// 	{
// 		printf("%d, ", face2Cell_.data[j]);
// 	}
// 	printf("\n");
// }
	// for (int i = 0; i < faceNum_i_; ++i) { assert(bonus[i]==2); }
	// delete[] bonus;

	/// construct the topology array: node2Cell
	
}

// label Topology::reorderFace2Node(Array<Array<label> >& face2NodeTmp, Array<Array<label> >& face2NodeBndTmp)
// {
// 	Array<Array<label> > result;
// 	// printf("%d,%d\n", face2NodeTmp.size(), face2NodeBndTmp.size());
// 	face2NodeTmp.insert(face2NodeTmp.begin(), face2NodeBndTmp.begin(),face2NodeBndTmp.end());
// 	// printf("%d,%d\n", face2NodeTmp.size(), face2NodeBndTmp.size());
// 	filterArray(face2NodeTmp);
// 	// printf("%d,%d\n", face2NodeTmp.size(), face2NodeBndTmp.size());
// }


void Topology::genEdgeTopo()
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	ArrayArray<label> cell2Node = this->cell2Node_;

	label cellNum = cell2Node.size();
	Array<Array<label> > edges2NodesTmp;
	for (int i = 0; i < cellNum; ++i)
	{
		label edgeNumTmp = Section::edgesNumForEle(this->cellType_[i]);
		for (int j = 0; j < edgeNumTmp; ++j)
		{
			Array<label> edge2NodeTmp = Section::edgeNodesForEle(
				&cell2Node.data[cell2Node.startIdx[i]], cellType_[i], j);
			sort(edge2NodeTmp.begin(), edge2NodeTmp.end());
			edge2NodeTmp.push_back(i+cellStartId_);
			edges2NodesTmp.push_back(edge2NodeTmp);
		}
	}
	// 对所有边单元进行排序
	Array<label> weight;
	for (int i = 0; i < edges2NodesTmp.size(); ++i)
	{
		weight.push_back(edges2NodesTmp[i][0]);
	}
	sort(edges2NodesTmp, weight);
	// quicksortArray(edges2NodesTmp, 0, edges2NodesTmp.size()-1);
	// 找到面与cell之间关系
	bool* isInner = new bool[edges2NodesTmp.size()];
	Array<Array<label> > edge2NodeInn, edge2NodeBnd;
	Array<Array<label> > edge2CellInn, edge2CellBnd;
	for (int i = 0; i < edges2NodesTmp.size(); ++i){isInner[i] = false;}
	for (int i = 0; i < edges2NodesTmp.size(); ++i)
	{
		if(isInner[i]) continue;
		int end = i+1;
		// 默认两个面不相等
		bool isEqual = false;

		Array<label> edge2CellTmp;
		label ownerId = edges2NodesTmp[i][edges2NodesTmp[i].size()-1];
		edge2CellTmp.push_back(ownerId);

		Array<label> edge2NodeTmp;
		edge2NodeTmp = edges2NodesTmp[i];
		edge2NodeTmp.pop_back();

		while(end<edges2NodesTmp.size() && 
			edges2NodesTmp[i][0] == edges2NodesTmp[end][0])
		{
			// 第一个node相等时，默认相等
			isEqual = true;
			/// 如果维度不相等，则两个面不相等
			if(edges2NodesTmp[i].size()!=edges2NodesTmp[end].size())
			{
				isEqual = false;
				end++;
				continue;
			}
			// 比较各个维度，不相等则跳出，标记不相等
			for (int j = 0; j < edges2NodesTmp[i].size()-1; ++j)
			{
				if(edges2NodesTmp[i][j]!=edges2NodesTmp[end][j])
				{
					isEqual = false;
					break;
				}
			}
			if(isEqual)
			{
				label neighborId = edges2NodesTmp[end][edges2NodesTmp[end].size()-1];
				edge2CellTmp.push_back(neighborId);
				// 删除相等面
				isInner[i] = true;
				isInner[end] = true;
			}
			end++;
		}
		if(isInner[i])
		{
			edge2CellInn.push_back(edge2CellTmp);
			edge2NodeInn.push_back(edge2NodeTmp);
		} else
		{
			edge2CellBnd.push_back(edge2CellTmp);
			edge2NodeBnd.push_back(edge2NodeTmp);
		}
	}
	// par_std_out_("inner edge num: %d, boundary edge num: %d\n", edge2NodeInn.size(), edge2CellBnd.size());

	Array<Array<label> > edge2NodeArr;
	edge2NodeArr.insert(edge2NodeArr.end(), edge2NodeInn.begin(), edge2NodeInn.end());
	edge2NodeArr.insert(edge2NodeArr.end(), edge2NodeBnd.begin(), edge2NodeBnd.end());

	Array<Array<label> > edge2CellArr;
	edge2CellArr.insert(edge2CellArr.end(), edge2CellInn.begin(), edge2CellInn.end());
	edge2CellArr.insert(edge2CellArr.end(), edge2CellBnd.begin(), edge2CellBnd.end());

	// for (int i = 0; i < edge2CellInn.size(); ++i)
	// {
	// 	printf("The %dth edge: ", i);
	// 	for (int j = 0; j < edge2CellInn[i].size(); ++j)
	// 	{
	// 		printf("%d, ", edge2CellInn[i][j]);
	// 	}
	// 	printf("\n");
	// }

	Array<Array<label> > cell2EdgeArr(this->cellNum_);
	for (int i = 0; i < edge2CellArr.size(); ++i)
	{
		/// 重排edge2node编号
		label cellIdx = edge2CellArr[i][0]-cellStartId_;
		label edgeNumTmp = Section::edgesNumForEle(cellType_[cellIdx]);
		for (int j = 0; j < edgeNumTmp; ++j)
		{
			Array<label> edge2NodeTmp = Section::edgeNodesForEle(
				&cell2Node.data[cell2Node.startIdx[cellIdx]],
				cellType_[cellIdx], j);
			Array<label> tmp;
			tmp.assign(edge2NodeTmp.begin(), edge2NodeTmp.end());
			sort(edge2NodeTmp.begin(), edge2NodeTmp.end());
			if(compareArray(edge2NodeTmp, edge2NodeArr[i]))
				edge2NodeArr[i].swap(tmp);			
		}
		/// 获取cell2edge
		for (int j = 0; j < edge2CellArr[i].size(); ++j)
		{
			cell2EdgeArr[edge2CellArr[i][j]-cellStartId_].push_back(i);
		}
	}

	transformArray(edge2NodeArr, this->edge2Node_);
	transformArray(edge2CellArr, this->edge2Cell_);
	transformArray(cell2EdgeArr, this->cell2Edge_);

	DELETE_POINTER(isInner);

	// for (int i = 0; i < cell2EdgeArr.size(); ++i)
	// {
	// 	printf("The %dth element: ", i);
	// 	for (int j = 0; j < cell2EdgeArr[i].size(); ++j)
	// 	{
	// 		printf("%d, ", cell2EdgeArr[i][j]);
	// 	}
	// 	printf("\n");
	// }
}

} // end namespace HSF
/**
* @file: loadBalancer.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-23 15:26:27
* @last Modified by:   lenovo
* @last Modified time: 2019-12-17 15:20:20
*/
#include <cstdio>
#include "mpi.h"
#include "parmetis.h"
#include "loadBalancer.hpp"
#include "section.hpp"

namespace HSF
{

/*
* @brief find the process with the least measurement load
* @param procId Id of processes for all regions
*/
label LoadBalancer::findMinProc()
{
	for (int i = 0; i < this->procNum_; ++i)
	{
		procLoadSum_[i] = 0.0;
	}
	label regionNum = procId_.size();
	for (int i = 0; i < regionNum; ++i)
	{
		for (int j = procId_.startIdx[i]; j < procId_.startIdx[i+1]; ++j)
		{
			if(procId_.data[j]<0) continue;
			procLoadSum_[procId_.data[j]]+=procLoad_.data[j];
		}
	}

	// for (int i = 0; i < this->procNum_; ++i)
	// {
	// 	printf("%f\n", procLoadSum_[i]);
	// }

	label minProcIdx = -1;
	scalar minProcLoad = 0x7fffffff;
	for (int i = 0; i < this->procNum_; ++i)
	{
		if(procLoadSum_[i] < minProcLoad)
		{
			minProcIdx = i;
			minProcLoad = procLoadSum_[i];
		}
	}
	// delete[] procLoad;
	return minProcIdx;
}
/*
* @brief find the region with the maximum measurement load
* @param s the measurement load of all regions
*/
label LoadBalancer::findMaxRegion(const Array<scalar> s)
{
	label regionNum = s.size();
	label maxRegionIdx = -1;
	label maxRegionLoad = 0;
	for (int i = 0; i < regionNum; ++i)
	{
		if(s[i] > maxRegionLoad)
		{
			maxRegionIdx = i;
			maxRegionLoad = s[i];
		}
	}
	return maxRegionIdx;
}
/**
* @brief default constructor
*/
LoadBalancer::LoadBalancer() : procLoadSum_(NULL), cellStartId_(NULL){};
/**
* @brief deconstructor
*/
LoadBalancer::~LoadBalancer()
{
	DELETE_POINTER(procLoadSum_);
	DELETE_POINTER(cellStartId_);
};
/*
* @brief Second Level load balance
* @param s measurement for all regions
* @param nei connectivity among all regions
* @param procNum count of processes
*/
void LoadBalancer::LoadBalancer_2(const Array<scalar> s, const ArrayArray<label> nei,
	label procNum)
{
	this->s_   = s;
	this->nei_ = nei;
	this->procNum_ = procNum;
	procLoadSum_ = new scalar[this->procNum_];

	label regionNum = this->s_.size();
	/// ID of processes in each region
	label **procId = new label *[regionNum];
	/// proportion of ID of processes in each region
	scalar **procLoad = new scalar *[regionNum];
	/// count of processes in each region
	label *procNumReg = new label[regionNum];
	scalar S = 0;
	/// sum up the measurements of all regions
	for (int i = 0; i < regionNum; ++i)
	{
		S += this->s_[i];
	}
	// printf("Total measurement: %f\n", S);
	/// average measurement load for each process
	scalar phi = scalar(S/procNum);
	label procIdx = 0;
	/// Select the private processes for each region
	for (int i = 0; i < regionNum; ++i)
	{
		/// total count of processes
		procNumReg[i]   = ceil(this->s_[i]/phi);
		/// calculate the count of private processes
		label procNumLower = floor(this->s_[i]/phi);
		procId[i]   = new label[procNumReg[i]];
		procLoad[i] = new scalar[procNumReg[i]];
		// printf("%d, %d\n", i, procNumLower);
		for (int j = 0; j < procNumReg[i]; ++j) { procId[i][j]=-1; }
		for (int j = 0; j < procNumLower; ++j)
		{
			procId[i][j]   = procIdx++;
			procLoad[i][j] = phi;
		}
		/// remove the load charged for provate processes
		this->s_[i] = this->s_[i]-phi*procNumLower;
	}

	/// construct the processes ID with ArrayArray type
	this->procId_.num = regionNum;
	this->procId_.startIdx = new label[regionNum+1];
	this->procLoad_.num = regionNum;
	this->procLoad_.startIdx = new label[regionNum+1];
	int totalCount = 0;
	this->procId_.startIdx[0] = 0;
	this->procLoad_.startIdx[0] = 0;
	for (int i = 0; i < regionNum; ++i)
	{
		this->procId_.startIdx[i+1]
			= this->procId_.startIdx[i] + procNumReg[i];
		this->procLoad_.startIdx[i+1]
			= this->procLoad_.startIdx[i] + procNumReg[i];
		totalCount += procNumReg[i];
	}
	this->procId_.data = new label[totalCount];
	this->procLoad_.data = new scalar[totalCount];
	int k = 0;
	for (int i = 0; i < regionNum; ++i)
	{
		for (int j = 0; j < procNumReg[i]; ++j)
		{
			this->procId_.data[k] = procId[i][j];
			this->procLoad_.data[k] = procLoad[i][j];
			k++;
		}
	}

	/// Select the public processes for each region
	// for (int i = 0; i < this->s_.size(); ++i)
	int i;
	while(true)
	{
	// printf("************************************\n");
	// for (int k = 0; k < regionNum; ++k)
	// {
	// 	for (int j = procId_.startIdx[k]; j < procId_.startIdx[k+1]; ++j)
	// 	{
	// 		printf("region: %d, pid: %d\n", k, procId_.data[j]);
	// 	}
	// }
		i = findMaxRegion(this->s_);
		if(i==-1) break;
		scalar phiReg = this->s_[i];
		/// The public process must be the last process for each region
		if(procIdx >= procNum)
		{
			/// if all processes are assigned, then find
			/// the process with the least load
			procIdx = findMinProc();
			// printf("%d\n", procIdx);
		}
		this->procId_.data[this->procId_.startIdx[i+1]-1] = procIdx;
		this->procLoad_.data[this->procId_.startIdx[i+1]-1] = phiReg;
		/// 0 indicate the region has been assigned the public process
		this->s_[i] = 0;
		for (int j = this->nei_.startIdx[i]; j < this->nei_.startIdx[i+1]; ++j)
		{
			label regionId = this->nei_.data[j];
			/// if the neighbor region is waiting for assigning the public
			/// process and the load of this public process does not
			/// exceed the average measurement load.
			phiReg += this->s_[regionId];
			if(phiReg < phi && this->s_[regionId] > 0)
			{
				this->procId_.data[this->procId_.startIdx[regionId+1]-1]
					= procIdx;
				this->procLoad_.data[this->procId_.startIdx[regionId+1]-1]
					= this->s_[regionId];
				/// mark the neighbor region as finished region.
				this->s_[regionId] = 0;
			}
		}
		procIdx++;
	}
	findMinProc();

	for (int i = 0; i < regionNum; ++i)
	{
		DELETE_POINTER(procId[i]);
		DELETE_POINTER(procLoad[i]);
	}
	DELETE_POINTER(procId);
	DELETE_POINTER(procLoad);
	DELETE_POINTER(procNumReg);

	// for (int i = 0; i < regionNum; ++i)
	// {
	// 	for (int j = procId_.startIdx[i]; j < procId_.startIdx[i+1]; ++j)
	// 	{
	// 		printf("region: %d, pid: %d\n", i, procId_.data[j]);
	// 	}
	// }
};

/*
* @brief Third Level load balance with ParMETIS
* @param regs the collection of regions owned by this processor
*/
void LoadBalancer::LoadBalancer_3(Array<Region>& regs)
{

	int nprocs,rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	int regNum = regs.size();
	assert(regNum==1);

	for (int iReg = 0; iReg < regNum; ++iReg)
	{
		Array<Section> secs = regs[iReg].getMesh().getSections();
		Nodes& nodes = regs[iReg].getMesh().getNodes();
		label secNum = secs.size();
		// if this region is private for the process
		// then it need not to be partitioned
		// record the type of cells
		label cellNum = 0;
		Array<label> cellType;
		for (int i = 0; i < secNum; ++i)
		{
			cellNum += secs[i].num;
			for (int j = 0; j < secs[i].num; ++j)
			{
				// printf("%d, %d, %d\n", j, secs[i].num, secs[i].type);
				cellType.push_back(secs[i].type);
			}
		}

		cellStartId_ = new label[nprocs+1];
		// printf("rank: %d, %d\n", rank, cellNum);
		MPI_Allgather(&cellNum, 1, MPI_LABEL, &cellStartId_[1], 1, MPI_LABEL, MPI_COMM_WORLD);
		cellStartId_[0] = 0;
		for (int i = 1; i <= nprocs; ++i)
		{
			cellStartId_[i] += cellStartId_[i-1];
		}

		// construct the topology array: cell2Node
		ArrayArray<label> cell2Node;
		cell2Node.num = cellNum;
		cell2Node.startIdx = new label[cellNum+1];
		cell2Node.startIdx[0] = 0;
		label k=0;
		// label *cellStartId_ = new label[cellNum];
		for (int i = 0; i < secNum; ++i)
		{
			for (int j = 0; j < secs[i].num; ++j)
			{
				// cellStartId_[k] = secs[i].iStart-1;
				cell2Node.startIdx[k+1]
					= cell2Node.startIdx[k] + Section::nodesNumForEle(secs[i].type);
				k++;
			}
		}
		cell2Node.data = new label[cell2Node.startIdx[cellNum]];
		k=0;
		for (int i = 0; i < secNum; ++i)
		{
			int l = 0;
			for (int j = 0; j < secs[i].num; ++j)
			{
				memcpy(&cell2Node.data[cell2Node.startIdx[k]],
					&secs[i].conn[l],
					Section::nodesNumForEle(secs[i].type)*sizeof(label));
				k++;
				l += Section::nodesNumForEle(secs[i].type);
			}
		}
#if 1
		Array<Array<label> > faces2NodesTmp;
		Array<label> face2CellTmp;
		Array<Array<label> > cell2Cell(cellNum);
		for (int i = 0; i < cellNum; ++i)
		{
			// printf("before: %d, %d, %d\n", rank, i, cellType[i]);
			label faceNumTmp = Section::facesNumForEle(cellType[i]);
			// printf("after: %d, %d, %d\n", rank, i, cellType[i]);
			for (int j = 0; j < faceNumTmp; ++j)
			{
				Array<label> face2NodeTmp = Section::faceNodesForEle(
					&cell2Node.data[cell2Node.startIdx[i]], cellType[i], j);
				sort(face2NodeTmp.begin(), face2NodeTmp.end());
				// printf("\n");
				// label idx = findArray(faces2NodesTmp, face2NodeTmp);
				// label idx = -1;
				// std::vector<label>::iterator result = find(faces2NodesSquare.begin(), faces2NodesSquare.end(), square);
				// std::vector<label>::iterator result = find(faces2NodesTmp.begin(), faces2NodesTmp.end(), compareArray());
				// if(idx!=-1) printf("find\n");
				// // printf("%d\n", i);
				// if(idx!=-1)
				// {
				// if(rank==0) printf("%d, %d\n", idx, result-faces2NodesSquare.begin());
					// label cellId = face2CellTmp[idx];
					// cell2Cell[i].push_back(cellId);
					// cell2Cell[cellId-cellStartId_[rank]].push_back(i+cellStartId_[rank]);
					// face2CellTmp[idx] = -1;
					// continue;
				// }
				face2NodeTmp.push_back(i+cellStartId_[rank]);
				faces2NodesTmp.push_back(face2NodeTmp);
				// printf("%d, %d\n", i, cellStartId_[i]);
				// face2CellTmp.push_back(i+cellStartId_[rank]);
			}
		// printf("faceNum: %d, cellNum: %d\n", faces2NodesTmp.size(), face2CellTmp.size());
		}
		// printf("faceNum: %d, cellNum: %d\n", faces2NodesTmp.size(), cell2Cell.size());
		// filterArray(faces2NodesTmp);
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
		Array<Array<label> > faces2NodesArr;
		for (int i = 0; i < faces2NodesTmp.size(); ++i){isInner[i] = false;}
		for (int i = 0; i < faces2NodesTmp.size(); ++i)
		{
			if(isInner[i]) continue;
			int end = i+1;
			// 默认两个面不相等
			bool isEqual = false;
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
					// const scalar* i_xyz   = nodes.getXYZ(faces2NodesTmp[i][j]-1-16*rank);
					// const scalar* end_xyz = nodes.getXYZ(faces2NodesTmp[end][j]-1-16*rank);
					// for (int k = 0; k < SOL_DIM; ++k)
					// {
					// 	if(i_xyz[k]!=end_xyz[k])
					// 	{
					// 		isEqual = false;
					// 		break;
					// 	}
					// }
				}
				if(isEqual)
				{
					// 本面对应cell编号为owner，相等面对应cell编号为neighbor
					label ownerId = faces2NodesTmp[i][faces2NodesTmp[i].size()-1];
					label neighborId = faces2NodesTmp[end][faces2NodesTmp[end].size()-1];
					cell2Cell[ownerId-cellStartId_[rank]].push_back(neighborId);
					cell2Cell[neighborId-cellStartId_[rank]].push_back(ownerId);
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
			// 记录面单元对应cell编号，内部面cell编号标记为-1
			if(!isEqual) face2CellTmp.push_back(cellId);
			else face2CellTmp.push_back(-1);
			faces2NodesArr.push_back(faces2NodesTmp[i]);
		}

#endif
#ifdef DEBUG
		printf("faceNum: %d, cellNum: %d\n", face2CellTmp.size(), cell2Cell.size());
		for (int i = 0; i < cell2Cell.size(); ++i)
		{
			printf("The %dth cell: ", i+cellStartId_[rank]);
			for (int j = 0; j < cell2Cell[i].size(); ++j)
			{
				printf("%d, ", cell2Cell[i][j]);
			}
			printf("\n");
		}
#endif
#if 1
		/// 收集边界面与node的关系
		int bndFaces = 0;
		for (int i = 0; i < face2CellTmp.size(); ++i)
		{
			if(face2CellTmp[i]!=-1) bndFaces++;
		}
par_std_out_("boundary faces num: %d\n", bndFaces);

		ArrayArray<label> bndFaceList;
		bndFaceList.num = bndFaces;
		bndFaceList.startIdx = new label[bndFaces+1];
		bndFaceList.startIdx[0] = 0;
		k=0;
		for (int i = 0; i < faces2NodesArr.size(); ++i)
		{
			if(face2CellTmp[i]==-1) continue;
			bndFaceList.startIdx[k+1] 
				= bndFaceList.startIdx[k]+faces2NodesArr[i].size();
			k++;
		}
		bndFaceList.data = new label[bndFaceList.startIdx[bndFaces]];
		k=0;
		for (int i = 0; i < faces2NodesArr.size(); ++i)
		{
			if(face2CellTmp[i]==-1) continue;
			// printf("The %dth face: \n", k);
			for (int j = bndFaceList.startIdx[k]; j < bndFaceList.startIdx[k+1]; ++j)
			{
				bndFaceList.data[j] = faces2NodesArr[i][j-bndFaceList.startIdx[k]];
				// printf("%d, ", bndFaceList.data[j]);
			}
			// printf("\n");
			k++;
		}
#endif
		// MPI通信获取边界面对应其他进程的cell编号
		Array<label> face2CellNew;
		face2CellNew = collectNeighborCell(bndFaceList, faces2NodesArr,
			face2CellTmp);
		// printf("finish collect\n");

		for (int i = 0; i < face2CellNew.size(); ++i)
		{
			// if(rank==1 && face2CellTmp[i]==16) printf("%d, %d\n", i, face2CellNew[i]);
			if(face2CellNew[i]>=0)
			{
				// for (int j = 0; j < cellNum; ++j)
				// {
				// 	if(cellStartId_[rank]+j==face2CellTmp[i]) 
				// 		cell2Cell[j].push_back(face2CellNew[i]);
				// }
				cell2Cell[face2CellTmp[i]-cellStartId_[rank]].push_back(face2CellNew[i]);
			}
		}
#ifdef DEBUG
		for (int i = 0; i < nprocs; ++i)
		{
			if(rank==i)
			{
		for (int i = 0; i < cell2Cell.size(); ++i)
		{
			printf("The %dth cell: ", i+cellStartId_[rank]);
			for (int j = 0; j < cell2Cell[i].size(); ++j)
			{
				printf("%d, ", cell2Cell[i][j]);
			}
			printf("\n");
		}
		}
		MPI_Barrier(MPI_COMM_WORLD);
		}
		MPI_Barrier(MPI_COMM_WORLD);
#endif
		ArrayArray<label> cell2CellArr;
		transformArray(cell2Cell, cell2CellArr);
#ifdef DEBUG_METIS
		for (int i = 0; i < nprocs; ++i)
		{
			if(rank==i)
			{
				printf("rank = %d\n", rank);
				printf("xadj: ");
				for (int i = 0; i <= cell2CellArr.num; ++i)
				{
					printf("%d, ", cell2CellArr.startIdx[i]);
				}
				printf("\n");
				printf("adjncy: ");
				for (int i = 0; i < cell2CellArr.startIdx[cell2CellArr.num]; ++i)
				{
					printf("%d, ", cell2CellArr.data[i]);
				}
				printf("\n");
				printf("vtxlist: ");
				for (int i = 0; i <= nprocs; ++i)
				{
					printf("%d, ", cellStartId_[i]);
				}
				printf("\n");
			}
			MPI_Barrier(MPI_COMM_WORLD);
		}
#endif

		// /// get parameters of ParMetis: vtxlist, xadj, adjncy
		label* xadj    = cell2CellArr.startIdx;
		label* adjncy  = cell2CellArr.data;
		idx_t* vtxlist = new idx_t[nprocs+1];
		for (int i = 0; i <= nprocs; ++i)
		{
			vtxlist[i] = (idx_t)cellStartId_[i];
		}
		// label  cStart = regs[i].getMesh().getCellStart();
		// /// gather the start index of cells from other processes
		// mpi_collective(vtxlist);

		label* vwgt = NULL;
		label* adjwgt = NULL;
		label  wgtflag = 0;
		label  numflag = 0;
		label  ncon = 1;
		label  nparts = nprocs;
		real_t* tpwgts = new real_t[ncon*nparts];
		for (int j = 0; j < nparts; ++j)
		{
			// tpwgts[j] = procLoad_.data[j+procId_.startIdx[regIdx]];
			tpwgts[j] = (real_t)1/(real_t)nparts;
			// tpwgts[j] = 1.0/3.0;
		}
		real_t  ubvec = 1.05;
		label  options[3] = {0, 0, 0};
		label  edgecut;
		label* parts = new label[cellNum];
		MPI_Comm comm = MPI_COMM_WORLD;
		ParMETIS_V3_PartKway(vtxlist, xadj, adjncy, vwgt, adjwgt, &wgtflag,
			&numflag, &ncon, &nparts, tpwgts, &ubvec, options, &edgecut,
			parts, &comm);
		MPI_Barrier(MPI_COMM_WORLD);
#ifdef DEBUG_METIS
		for (int i = 0; i < nprocs; ++i)
		{
			if(i==rank)
			{
				printf("rank: %d\n", i);
				for (int j = 0; j < cellNum; ++j)
				{
					printf("(%d-->%d), ", j+cellStartId_[i], parts[j]);
				}
				printf("\n");
			}
			MPI_Barrier(MPI_COMM_WORLD);
		}
#endif

		ArrayArray<label> cell2NodeNew = distributeCellsToProcs(cell2Node, parts);
		// printf("distributeCellsToProcs finish!!!\n");
		// MPI_Barrier(MPI_COMM_WORLD);

		Array<label> cellTypeNew = distributeCellInfoToProcs(cellType, parts);
		// printf("distributeCellInfoToProcs finish!!!\n");
		// MPI_Barrier(MPI_COMM_WORLD);

		// printf("%d, %d, %d\n", rank, cell2NodeNew.size(), cellTypeNew.size());
		// 输入网格负载均衡后的拓扑
		regs[iReg].getMesh().setLoadBalancerResult(cell2NodeNew, cellTypeNew);
		// 输出原始网格拓扑
		// regs[i].getMesh().setLoadBalancerResult(cell2Node,  cellType);
		// printf("%d\n", cell2NodeNew.size());
		// for (int i = 0; i < cellNum; ++i)
		// {
		// 	if(parts[i]>nprocs) printf("rank: %d, cellIdx: %d, value: %d\n", rank, i, parts[i]);
		// }

		DELETE_POINTER(isInner);
		DELETE_POINTER(vtxlist);
		DELETE_POINTER(tpwgts);
		DELETE_POINTER(parts);
		// return parts;
	}
}

/*
* @brief collect the neighbor cell index through mpi
*/
Array<label> LoadBalancer::collectNeighborCell(ArrayArray<label>& bndFaceList,
	Array<Array<label> >& face2NodeArr, Array<label>& face2CellArr)
{
	// Array<label> face2CellNew;
	int bndFaces = bndFaceList.num;

	int nprocs,rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	int* bndFaces_mpi = new int[nprocs];
	MPI_Allgather(&bndFaces, 1, MPI_INT, bndFaces_mpi, 1, MPI_INT, MPI_COMM_WORLD);
	int bndFacesSum = 0;
	int ownerLoc = 0;
	int countIdx_r[nprocs], dispIdx_r[nprocs];
	for (int i = 0; i < nprocs; ++i)
	{
		// if(i==rank) ownerLoc = bndFacesSum+rank;
		dispIdx_r[i] = bndFacesSum+i;
		countIdx_r[i] = bndFaces_mpi[i]+1;
		bndFacesSum += bndFaces_mpi[i];
		// printf("rank: %d, bndFaces: %d\n", i, bndFaces_mpi[i]);
	}
	label* startIdx_mpi = new label[bndFacesSum+nprocs];
#if 1
	MPI_Allgatherv(bndFaceList.startIdx, bndFaces+1, MPI_LABEL, startIdx_mpi, countIdx_r, dispIdx_r, MPI_LABEL, MPI_COMM_WORLD);
	int bndNodesSum = 0;
	int countData_r[nprocs], dispData_r[nprocs];
	for (int i = 0; i < nprocs; ++i)
	{
		countData_r[i] = startIdx_mpi[dispIdx_r[i]+countIdx_r[i]-1];
		dispData_r[i] = bndNodesSum;
		bndNodesSum += countData_r[i];
		// if(rank==0) printf("count: %d, displacement: %d\n", countData_r[i], dispData_r[i]);
	}
	label* data_mpi = new label[bndNodesSum];
	MPI_Allgatherv(bndFaceList.data, bndFaceList.startIdx[bndFaces],
		MPI_LABEL, data_mpi, countData_r, dispData_r, MPI_LABEL, MPI_COMM_WORLD);
	label* neighborCellIdx_mpi = new label[bndFacesSum+nprocs];
#endif
	for (int i = 0; i < bndFacesSum+nprocs; ++i) { neighborCellIdx_mpi[i] = -1; }
	// if(rank==2)
	{
		for (int i = 0; i < nprocs; ++i)
		{
			if(rank==i) continue;
			// printf("rank: %d, start from: %d, read %d\n", i, dispIdx_r[i], countIdx_r[i]);
			label* startIdx = &startIdx_mpi[dispIdx_r[i]];
			for (int j = 0; j < countIdx_r[i]-1; ++j)
			{
				// printf("The %dth element: ", j);
				Array<label> face2NodeTmp;
				for (int k = startIdx[j]; k < startIdx[j+1]; ++k)
				{
					// printf("%d, ", data_mpi[dispData_r[i]+k]);
					face2NodeTmp.push_back(data_mpi[dispData_r[i]+k]);

				}
				label idx = findSortedArray(face2NodeArr, face2NodeTmp, 0, face2NodeArr.size()-1);
				// label idx = findArray(bndFaceArr, face2NodeTmp);
				// label idx = -1;
				if(idx!=-1) neighborCellIdx_mpi[dispIdx_r[i]+j] = face2CellArr[idx];
				else neighborCellIdx_mpi[dispIdx_r[i]+j]=-1;
				// if(idx==-1) printf("not found\n");
				// else printf("%d\n", idx);
			}
			// printf("\n");
		}
	}
#if 1

	label* neighborCellIdx_local = new label[(bndFaces+1)*nprocs];
	int countCellIdx_r[nprocs], dispCellIdx_r[nprocs];
	for (int i = 0; i < nprocs; ++i)
	{
		countCellIdx_r[i] = bndFaces+1;
		dispCellIdx_r[i] = i*(bndFaces+1);
	}
	MPI_Alltoallv(neighborCellIdx_mpi, countIdx_r, dispIdx_r, MPI_LABEL, 
		neighborCellIdx_local, countCellIdx_r, dispCellIdx_r, MPI_LABEL,
		MPI_COMM_WORLD);
	Array<label> face2CellNew;
	// if(rank==1)
	{
		for (int i = 1; i < nprocs; ++i)
		{
			int k=0;
			for (int j = 0; j < bndFaces+1; ++j)
			{
				if(neighborCellIdx_local[i*(bndFaces+1)+j]!=-1)
					neighborCellIdx_local[j] = neighborCellIdx_local[i*(bndFaces+1)+j];
			}
		}
		for (int i = 0; i < face2CellArr.size(); ++i)
		{
			face2CellNew.push_back(face2CellArr[i]);
			// printf("%d, ", face2CellArr[i]);
		}
		// for (int i = 0; i < nprocs; ++i)
		// {
			int k=0;
		// 	printf("rank: %d, received from %d: ", rank, i);
			for (int j = 0; j < bndFaces+1; ++j)
			{
				// printf("%d, ", neighborCellIdx_local[j]);
				if(neighborCellIdx_local[j]==-1)
					neighborCellIdx_local[j] = -2;
				while(face2CellNew[k]==-1) k++;
				// if(neighborCellIdx_local[j]!=0)
					// cell2CellArr[face2CellArr[k]].push_back(neighborCellIdx_local[j])
				face2CellNew[k++] = neighborCellIdx_local[j];
			}
			// printf("\n");
		// }
		// for (int i = 0; i < face2CellNew.size(); ++i)
		// {
		// 	printf("%d, ", face2CellNew[i]);
		// }
		// printf("\n");
	}

	DELETE_POINTER(neighborCellIdx_local);
	DELETE_POINTER(neighborCellIdx_mpi);
	DELETE_POINTER(data_mpi);
	DELETE_POINTER(startIdx_mpi);
	DELETE_POINTER(bndFaces_mpi);
	// face2Cell
	// >= 0 process boundary face
	// ==-1 inner face
	// ==-2 physical boundary face
	return face2CellNew;
#endif
	// return face2CellNew;
}

// void LoadBalancer::distributeCellsToProcs(const ArrayArray<label>& cell2Node,
	// const label* parts, ArrayArray<label>& cell2NodeNew)
ArrayArray<label> LoadBalancer::distributeCellsToProcs(const ArrayArray<label>& cell2Node,
	const label* parts)
{
	int nprocs,rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	Array<ArrayArray<label> > sendBuff;
	label* sendCount = new label[nprocs];
	for (int i = 0; i < nprocs; ++i) {sendCount[i]=0;}

	for (int i = 0; i < cell2Node.size(); ++i)
	{
		sendCount[parts[i]]++;
	}

	Array<Array<Array<label> > > sendBuffTmp(nprocs);

	for (int i = 0; i < cell2Node.size(); ++i)
	{
		// if(rank==0) printf("cell: %d belongs to %d\n", i, parts[i]);
		Array<label> face2NodeTmp;
		for (int j = cell2Node.startIdx[i]; j < cell2Node.startIdx[i+1]; ++j)
		{
			face2NodeTmp.push_back(cell2Node.data[j]);
		}
		sendBuffTmp[parts[i]].push_back(face2NodeTmp);
	}

	// if(rank==1)
	// {
	// for (int i = 0; i < nprocs; ++i)
	// {
	// 		printf("rank: %d\n", i);
	// 		for (int j = 0; j < sendBuffTmp[i].size(); ++j)
	// 		{
	// 			// printf("send to %d %d cells\n", i, sendBuff[i].size());
	// 			printf("The %dth cell: ", j);
	// 			for (int k = 0; k < sendBuffTmp[i][j].size(); ++k)
	// 			{
	// 				printf("%d, ", sendBuffTmp[i][j][k]);
	// 			}
	// 			printf("will send to processor %d\n", i);
	// 		}
	// 	// MPI_Barrier(MPI_COMM_WORLD);
	// }
	// }

	for (int i = 0; i < nprocs; ++i)
	{
		// sendBuff.push_back(transformArray(sendBuffTmp[i]));
		ArrayArray<label> tmp;
		transformArray(sendBuffTmp[i], tmp);
		// printf("%d\n", tmp.size());
		sendBuff.push_back(tmp);
	}

	label* recvCount = new label[nprocs+1];
	MPI_Alltoall(sendCount, 1, MPI_LABEL, &recvCount[1], 1, MPI_LABEL, MPI_COMM_WORLD);
	label recvSum = 0;
	recvCount[0] = 0;
	for (int i = 1; i <= nprocs; ++i)
	{
		// printf("rank %d receive %d cells from processor %d\n", rank, recvCount[i], i);
		recvSum += recvCount[i];
		recvCount[i] += recvCount[i-1];
	}
	MPI_Request request[nprocs*2];
	int iRequest=0;
	MPI_Status status[nprocs*2];
	for (int i = 0; i < nprocs; ++i)
	{
		if(rank==i) continue;
		MPI_Isend(sendBuff[i].startIdx, sendCount[i]+1,
			MPI_LABEL, i, rank, MPI_COMM_WORLD, &request[iRequest++]);
	}

	// Array<ArrayArray<label> > recvBuff(nprocs);
	ArrayArray<label> recvBuff;
	recvBuff.num = recvSum;
	recvBuff.startIdx = new label[recvSum+nprocs];
	for (int i = 0; i < nprocs; ++i)
	{
		if(rank==i)
		{
			memcpy(&recvBuff.startIdx[recvCount[i]+i],
				sendBuff[i].startIdx,
				(sendCount[i]+1)*sizeof(label));
			continue;
		}
		MPI_Irecv(&recvBuff.startIdx[recvCount[i]+i],
			recvCount[i+1]-recvCount[i]+1,
			MPI_LABEL, i, i, MPI_COMM_WORLD, &request[iRequest++]);
	}
	MPI_Waitall(iRequest, request, status);
	label* recvDataCount = new label[nprocs];
	label* recvDataDisp  = new label[nprocs+1];
	recvDataDisp[0] = 0;
	for (int i = 0; i < nprocs; ++i)
	{
		recvDataCount[i] = recvBuff.startIdx[recvCount[i+1]+i];
		recvDataDisp[i+1] = recvDataDisp[i]+recvDataCount[i];
		// printf("rank %d receive %d cells from processor %d\n", rank, recvDataCount[i], i);
		// for (int j = recvCount[i]+i; j < recvCount[i+1]+i+1; ++j)
		// {
			// printf("%d, ", recvBuff.startIdx[j]);
		// }
		// printf("\n");
	}

	ArrayArray<label> cell2NodeNew;
	cell2NodeNew.num = recvSum;
	cell2NodeNew.startIdx = new label[recvSum+1];
	cell2NodeNew.startIdx[0] = 0;
	label k=0;
	for (int i = 0; i < recvSum+nprocs; ++i)
	{
		if(recvBuff.startIdx[i]==0) continue;
		cell2NodeNew.startIdx[k+1] = cell2NodeNew.startIdx[k]
			+ recvBuff.startIdx[i]-recvBuff.startIdx[i-1];
		k++;
	}
	cell2NodeNew.data = new label[cell2NodeNew.startIdx[recvSum]];

	iRequest = 0;
	for (int i = 0; i < nprocs; ++i)
	{
		if (rank==i) continue;
		MPI_Isend(sendBuff[i].data, sendBuff[i].startIdx[sendBuff[i].size()],
			MPI_LABEL, i, rank, MPI_COMM_WORLD, &request[iRequest++]);
	}
	for (int i = 0; i < nprocs; ++i)
	{
		if(rank==i)
		{
			memcpy(&cell2NodeNew.data[recvDataDisp[i]],
				sendBuff[i].data, recvDataCount[i]*sizeof(label));
			continue;
		}
		MPI_Irecv(&cell2NodeNew.data[recvDataDisp[i]], recvDataCount[i],
			MPI_LABEL, i, i, MPI_COMM_WORLD, &request[iRequest++]);
	}
	MPI_Waitall(iRequest, request, status);

	// if(rank==1)
	// {
	// 	printf("rank: %d\n", rank);
	// 	for (int i = 0; i < cell2NodeNew.size(); ++i)
	// 	{
	// 		printf("The %dth elements: ", i);
	// 		for (int j = cell2NodeNew.startIdx[i]; j < cell2NodeNew.startIdx[i+1]; ++j)
	// 		{
	// 			printf("%d, ", cell2NodeNew.data[j]);
	// 		}
	// 		printf("\n");
	// 	}
	// }

	DELETE_POINTER(recvDataDisp);
	DELETE_POINTER(recvDataCount);
	DELETE_POINTER(recvCount);
	DELETE_POINTER(sendCount);

	return cell2NodeNew;
}

Array<label> LoadBalancer::distributeCellInfoToProcs(
	Array<label>& cellInfo, label* parts)
{
	Array<label> cellInfoNew;

	int nprocs,rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	Array<Array<label> > sendBuff(nprocs);
	label* sendCount = new label[nprocs];
	for (int i = 0; i < nprocs; ++i) {sendCount[i]=0;}

	for (int i = 0; i < cellInfo.size(); ++i)
	{
		sendCount[parts[i]]++;
		sendBuff[parts[i]].push_back(cellInfo[i]);
	}

	Array<label*> sendBuffTmp(nprocs);
	for (int i = 0; i < sendBuff.size(); ++i)
	{
		sendBuffTmp[i] = new label[sendBuff[i].size()];
		for (int j = 0; j < sendBuff[i].size(); ++j)
		{
			sendBuffTmp[i][j] = sendBuff[i][j];
		}
	}

	label* recvCount = new label[nprocs+1];
	MPI_Alltoall(sendCount, 1, MPI_LABEL, &recvCount[1], 1, MPI_LABEL, MPI_COMM_WORLD);
	label recvSum = 0;
	recvCount[0] = 0;
	for (int i = 1; i <= nprocs; ++i)
	{
		// printf("rank %d receive %d cells from processor %d\n", rank, recvCount[i], i);
		recvSum += recvCount[i];
		recvCount[i] += recvCount[i-1];
	}

	MPI_Request request[nprocs*2];
	int iRequest=0;
	MPI_Status status[nprocs*2];
	for (int i = 0; i < nprocs; ++i)
	{
		if(rank==i) continue;
		MPI_Isend(sendBuffTmp[i], sendCount[i],
			MPI_LABEL, i, rank, MPI_COMM_WORLD, &request[iRequest++]);
	}

	// Array<ArrayArray<label> > recvBuff(nprocs);
	label* recvBuff = new label[recvSum];
	for (int i = 0; i < nprocs; ++i)
	{
		if(rank==i)
		{
			memcpy(&recvBuff[recvCount[i]],
				sendBuffTmp[i],
				sendCount[i]*sizeof(label));
			continue;
		}
		MPI_Irecv(&recvBuff[recvCount[i]],
			recvCount[i+1]-recvCount[i],
			MPI_LABEL, i, i, MPI_COMM_WORLD, &request[iRequest++]);
	}
	MPI_Waitall(iRequest, request, status);

	for (int i = 0; i < recvSum; ++i)
	{
		cellInfoNew.push_back(recvBuff[i]);
	}

	DELETE_POINTER(recvBuff);
	DELETE_POINTER(recvCount);
	for (int i = 0; i < sendBuffTmp.size(); ++i)
	{
		DELETE_POINTER(sendBuffTmp[i]);
	}
	DELETE_POINTER(sendCount);
	
	return cellInfoNew;
}

} // end namespace HSF
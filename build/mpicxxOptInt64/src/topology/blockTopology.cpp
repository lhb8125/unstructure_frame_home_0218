/**
* @file: blockTopology.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2020-01-06 16:12:11
* @last Modified by:   lenovo
* @last Modified time: 2020-01-10 17:14:43
*/

#include <algorithm>
#include "blockTopology.hpp"
#include "section.hpp"

namespace HSF
{

BlockTopology::BlockTopology()
{
}

BlockTopology::~BlockTopology()
{

}

void BlockTopology::constructBlockTopology(Topology& topo)
{
	// printf("constructBlockTopology\n");
	Array<label> cellMap = this->reorderCellTopo(topo);
	Array<label> faceMap = this->reorderFaceTopo(topo);
}

Array<label> BlockTopology::reorderFaceTopo(Topology& topo)
{
	int rank, nprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	const ArrayArray<label> face2Node = topo.getFace2Node();
	Array<label> faceType;
	for (int i = 0; i < face2Node.size(); ++i)
	{
		int nodeNum = face2Node.startIdx[i+1]-face2Node.startIdx[i];
		faceType.push_back(Section::getFaceType(nodeNum));
	}
	int blockNum = 0;
	this->faceType_.clear();
	vector<label>::iterator it;
	Array<label> faceNumInBlk;
	faceNumInBlk.push_back(0);

	// 确定网格类型数目
	par_std_out_("确定网格类型数目\n");
	for (int i = 0; i < faceType.size(); ++i)
	{
		it = find(this->faceType_.begin(), this->faceType_.end(), faceType[i]);
		if(it!=this->faceType_.end())
		{
			faceNumInBlk[it-this->faceType_.begin()+1]++;
		}
		if(it==this->faceType_.end())
		{
			this->faceType_.push_back(faceType[i]);
			faceNumInBlk.push_back(1);
			ElementType_t type = (ElementType_t)faceType[i];
			par_std_out_("add cell type: %s\n", Section::typeToWord(type));
			blockNum++;
		}
	}

	// 从所有进程中找出最大block数目
	int* blockNum_mpi = new int[nprocs];
	MPI_Gather(&blockNum, 1, MPI_INT, blockNum_mpi, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int maxBlockNum = blockNum;
	int irank = 0;
	for (int i = 0; i < nprocs; ++i)
	{
		if(maxBlockNum < blockNum_mpi[i])
		{
			maxBlockNum = blockNum_mpi[i];
			irank = i;
		}
	}
	MPI_Bcast(&maxBlockNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&irank, 1, MPI_INT, 0, MPI_COMM_WORLD);
	par_std_out_("The maximum block num is %d at rank %d\n", maxBlockNum, irank);

	// 将最大进程的网格单元类型排布广播到所有进程
	label *buffer;
	buffer = new label[maxBlockNum];
	if(rank == irank)
	{
		for (int i = 0; i < maxBlockNum; ++i)
		{
			buffer[i] = this->faceType_[i];
		}
	}
	MPI_Bcast(buffer, maxBlockNum, MPI_LABEL, irank, MPI_COMM_WORLD);

	// 按照最大进程网格单元排布重排本进程block
	Array<label> newFaceType, newFaceNumInBlk;
	newFaceNumInBlk.push_back(0);
	for (int i = 0; i < maxBlockNum; ++i)
	{
		bool flag;
		for (int j = 0; j < blockNum; ++j)
		{
			flag = this->faceType_[j]==buffer[i];
			// 如果该block与最大进程排布相同，则压入该block信息
			if(flag)
			{
				newFaceType.push_back(this->faceType_[j]);
				newFaceNumInBlk.push_back(faceNumInBlk[j+1]);
				break;
			}
		}
		// 如果本进程不存在该block，则数目为0，类型置为-1
		if(!flag)
		{
			newFaceType.push_back(buffer[i]);
			newFaceNumInBlk.push_back(0);
		}
	}
	par_std_out_("Now I have %d blocks\n", newFaceType.size());

	this->faceType_.swap(newFaceType);
	faceNumInBlk.swap(newFaceNumInBlk);
	for (int i = 0; i < faceNumInBlk.size(); ++i)
	{
		faceNumInBlk[i+1] += faceNumInBlk[i];
	}

	Array<Array<Array<label> > > face2NodeBlk(maxBlockNum);
	Array<label> faceMap;

	// 将不同类型网格分装到不同容器中
	for (int i = 0; i < faceType.size(); ++i)
	{
		it = find(faceType_.begin(), faceType_.end(), faceType[i]);
		// 如果找到该网格所属block，则压入相应block内
		if(it!=faceType_.end())
		{
			// printf("%d,%d\n", i,it-cellType_.begin());
			Array<label> tmp;
			for (int j = face2Node.startIdx[i]; j < face2Node.startIdx[i+1]; ++j)
			{
				tmp.push_back(face2Node.data[j]);
			}
			label blockIdx = it-faceType_.begin();
			faceMap.push_back(face2NodeBlk[blockIdx].size()+faceNumInBlk[blockIdx]);
			face2NodeBlk[blockIdx].push_back(tmp);
		}
	}

	// 将不同类型网格合并到一起
	Array<Array<label> > face2NodeBlkNew;
	for (int i = 0; i < face2NodeBlk.size(); ++i)
	{
		face2NodeBlkNew.insert(face2NodeBlkNew.end(),
			face2NodeBlk[i].begin(), face2NodeBlk[i].end());
	}

	// face2Node
	transformArray(face2NodeBlkNew, this->face2Node_);

	this->faceBlockStartIdx_.assign(faceNumInBlk.begin(), faceNumInBlk.end());

	// face2Cell
	const ArrayArray<label> face2Cell = topo.getFace2Cell();

	reorderOtherTopo(faceMap, face2Cell, this->face2Cell_);

	return faceMap;
}

Array<label> BlockTopology::reorderCellTopo(Topology& topo)
{
	int rank, nprocs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	const ArrayArray<label> cell2Cell = topo.getCell2Cell();
	const Array<label> cellType       = topo.getCellType();
	int blockNum = 0;
	this->cellType_.clear();
	vector<label>::iterator it;
	Array<label> cellNumInBlk;
	cellNumInBlk.push_back(0);

	// 确定网格类型数目
	for (int i = 0; i < cellType.size(); ++i)
	{
		it = find(cellType_.begin(), cellType_.end(), cellType[i]);
		if(it!=cellType_.end())
		{
			cellNumInBlk[it-cellType_.begin()+1]++;
		}
		if(it==cellType_.end())
		{
			this->cellType_.push_back(cellType[i]);
			cellNumInBlk.push_back(1);
			ElementType_t type = (ElementType_t)cellType[i];
			par_std_out_("add cell type: %s\n", Section::typeToWord(type));
			blockNum++;
		}
	}

	// 从所有进程中找出最大block数目
	int* blockNum_mpi = new int[nprocs];
	MPI_Gather(&blockNum, 1, MPI_INT, blockNum_mpi, 1, MPI_INT, 0, MPI_COMM_WORLD);
	int maxBlockNum = blockNum;
	int irank = 0;
	for (int i = 0; i < nprocs; ++i)
	{
		if(maxBlockNum < blockNum_mpi[i])
		{
			maxBlockNum = blockNum_mpi[i];
			irank = i;
		}
	}
	MPI_Bcast(&maxBlockNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&irank, 1, MPI_INT, 0, MPI_COMM_WORLD);
	par_std_out_("The maximum block num is %d at rank %d\n", maxBlockNum, irank);

	// 将最大进程的网格单元类型排布广播到所有进程
	label *buffer;
	buffer = new label[maxBlockNum];
	if(rank == irank)
	{
		for (int i = 0; i < maxBlockNum; ++i)
		{
			buffer[i] = this->cellType_[i];
		}
	}
	MPI_Bcast(buffer, maxBlockNum, MPI_LABEL, irank, MPI_COMM_WORLD);

	// 按照最大进程网格单元排布重排本进程block
	Array<label> newCellType, newCellNumInBlk;
	newCellNumInBlk.push_back(0);
	for (int i = 0; i < maxBlockNum; ++i)
	{
		bool flag;
		for (int j = 0; j < blockNum; ++j)
		{
			flag = this->cellType_[j]==buffer[i];
			// 如果该block与最大进程排布相同，则压入该block信息
			if(flag)
			{
				newCellType.push_back(this->cellType_[j]);
				newCellNumInBlk.push_back(cellNumInBlk[j+1]);
				break;
			}
		}
		// 如果本进程不存在该block，则数目为0，类型置为-1
		if(!flag)
		{
			newCellType.push_back(buffer[i]);
			newCellNumInBlk.push_back(0);
		}
	}
	par_std_out_("Now I have %d blocks\n", newCellType.size());

	this->cellType_.swap(newCellType);
	cellNumInBlk.swap(newCellNumInBlk);
	for (int i = 0; i < cellNumInBlk.size(); ++i)
	{
		// printf("%d\n", cellNumInBlk[i]);
		cellNumInBlk[i+1] += cellNumInBlk[i];
	}

	Array<Array<Array<label> > > cell2CellBlk(maxBlockNum);
	Array<label> cellMap;

	// 将不同类型网格分装到不同容器中
	for (int i = 0; i < cellType.size(); ++i)
	{
		it = find(cellType_.begin(), cellType_.end(), cellType[i]);
		// 如果找到该网格所属block，则压入相应block内
		if(it!=cellType_.end())
		{
			// printf("%d,%d\n", i,it-cellType_.begin());
			Array<label> tmp;
			for (int j = cell2Cell.startIdx[i]; j < cell2Cell.startIdx[i+1]; ++j)
			{
				tmp.push_back(cell2Cell.data[j]);
			}
			label blockIdx = it-cellType_.begin();
			cellMap.push_back(cell2CellBlk[blockIdx].size()+cellNumInBlk[blockIdx]);
			cell2CellBlk[blockIdx].push_back(tmp);
		}
	}

	// 将不同类型网格合并到一起
	Array<Array<label> > cell2CellBlkNew;
	for (int i = 0; i < cell2CellBlk.size(); ++i)
	{
		cell2CellBlkNew.insert(cell2CellBlkNew.end(),
			cell2CellBlk[i].begin(), cell2CellBlk[i].end());
	}

	// 根据重排前后的映射调整cell2cell的值
	for (int i = 0; i < cell2CellBlkNew.size(); ++i)
	{
		// par_std_out_("The %dth element: ", i);
		for (int j = 0; j < cell2CellBlkNew[i].size(); ++j)
		{
			cell2CellBlkNew[i][j] = cellMap[cell2CellBlkNew[i][j]];
			// par_std_out_("%d, ", cell2CellBlkNew[i][j]);
		}
		// par_std_out_("\n");
	}

	// cell2cell
	par_std_out_("cell2cell\n");
	transformArray(cell2CellBlkNew, this->cell2Cell_);

	this->cellBlockStartIdx_.assign(cellNumInBlk.begin(), cellNumInBlk.end());

	// cell2node
	const ArrayArray<label> cell2Node = topo.getCell2Node();

	reorderOtherTopo(cellMap, cell2Node, this->cell2Node_);

	// cell2face
	const ArrayArray<label> cell2Face = topo.getCell2Face();

	reorderOtherTopo(cellMap, cell2Face, this->cell2Face_);

	return cellMap;
}

void BlockTopology::reorderOtherTopo(const Array<label>& map,
	const ArrayArray<label>& oldTopo, ArrayArray<label>& newTopo)
{
	newTopo.num = oldTopo.num;
	newTopo.startIdx = new label[oldTopo.num+1];
	newTopo.startIdx[0] = 0;
	for (int i = 0; i < oldTopo.size(); ++i)
	{
		newTopo.startIdx[map[i]+1]
			= oldTopo.startIdx[i+1]-oldTopo.startIdx[i];
	}
	for (int i = 0; i < oldTopo.size(); ++i)
	{
		newTopo.startIdx[i+1] += newTopo.startIdx[i];
	}

	newTopo.data = new label[newTopo.startIdx[oldTopo.num]];
	for (int i = 0; i < oldTopo.size(); ++i)
	{
		int startIdx = newTopo.startIdx[map[i]];
		int bonus = 0;
		for (int j = oldTopo.startIdx[i]; j < oldTopo.startIdx[i+1]; ++j)
		{
			newTopo.data[startIdx+bonus] = oldTopo.data[j];
			bonus++;
		}
	}	
}

}
/**
* @file: boundary.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-26 09:25:10
* @last Modified by:   lenovo
* @last Modified time: 2020-01-10 14:59:02
*/
#include "mpi.h"
#include "boundary.hpp"
// #include "loadBalancer.hpp"
#include "pcgnslib.h"

namespace HSF
{
    
void Boundary::readBoundaryCondition(const char* filePtr)
{
	int iFile;

    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
	// open cgns file
    if(cgp_open(filePtr, CG_MODE_READ, &iFile))
    	Terminate("readGridCGNS", cg_get_error());
    label iBase = 1;
    label iZone = 1;

    // if(cg_goto(iFile, iBase, "Zone", 1, "ZoneBC_t", 1, "BC_t", 1, "end"))
    	// Terminate("goZoneBC", cg_get_error());

    // read boundary condition
    int nBocos;
    if(cg_nbocos(iFile, iBase, iZone, &nBocos))
        Terminate("readNBocos", cg_get_error());

    char bocoName[CHAR_DIM];
    int normalIndex[3];
    PointSetType_t ptsetType[1];
    cgsize_t normalListSize;
    int nDataSet;
    DataType_t normDataType;
    for(int iBoco=1; iBoco<=nBocos; iBoco++)
    {
        BCSection BCSec;
        /* Read the info for this boundary condition. */
        if(cg_boco_info(iFile, iBase, iZone, iBoco, BCSec.name, &BCSec.type,
            BCSec.ptsetType, &BCSec.nBCElems, &normalIndex[0], &normalListSize, &normDataType,
            &nDataSet))
            Terminate("readBocoInfo", cg_get_error());
        if(BCSec.ptsetType[0]!=PointRange || BCSec.ptsetType[0]!=PointList)
        {
            if(BCSec.ptsetType[0]==ElementRange) BCSec.ptsetType[0] = PointRange;
            else if(BCSec.ptsetType[0]==ElementList) BCSec.ptsetType[0] = PointList;
            if(cg_boco_gridlocation_read(iFile, iBase, iZone, iBoco, &BCSec.location))
                Terminate("readGridLocation", cg_get_error());
            if(BCSec.location==CellCenter)
            {
                par_std_out_("The boundary condition is defined on CellCenter\n");
            }
            else if(BCSec.location==EdgeCenter)
            {
                Terminate("readGridLocation","The boundary condition is defined on EdgeCenter\n");
            }
            else if(BCSec.location==Vertex)
            {
                BCSec.location = CellCenter;
                par_std_out_("The boundary condition is defined on Vertex\n");
            }
            else if(BCSec.location==FaceCenter)
            {
                BCSec.location = FaceCenter;
                par_std_out_("The boundary condition is defined on FaceCenter\n");
            }
            else
            {
                par_std_out_("!!!!Error!!!!vertex: %d, FaceCenter: %d, "
                    "EdgeCenter: %d, location: %d\n", Vertex, FaceCenter,
                    EdgeCenter, BCSec.location);
            }
        }
        BCSec.BCElems = new cgsize_t[BCSec.nBCElems];
        if(cg_boco_read(iFile, iBase, iZone, iBoco, BCSec.BCElems, NULL))
        {
            Terminate("readBocoInfo", cg_get_error());
        }
        if(BCSec.ptsetType[0]==PointRange)
        {
            par_std_out_("PointRange: iBoco: %d, name: %s, type: %s, "
                "nEles: %d, start: %d, end: %d\n", iBoco, BCSec.name,
                BCSection::typeToWord(BCSec.type), BCSec.nBCElems,
                BCSec.BCElems[0], BCSec.BCElems[1]);
        }
        else if(BCSec.ptsetType[0]==PointList)
        {
            par_std_out_("PointList: iBoco: %d, name: %s, type: %s, "
                "nEles: %d\n", iBoco, BCSec.name,
                BCSection::typeToWord(BCSec.type), BCSec.nBCElems);
        }
        this->BCSecs_.push_back(BCSec);
    }    
    if(cg_close(iFile))
        Terminate("closeCGNSFile", cg_get_error());
}

void Boundary::writeBoundaryCondition(const char* filePtr)
{
    int iFile;
    int rank,nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    // open cgns file
    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
    // open cgns file
    if(cgp_open(filePtr, CG_MODE_MODIFY, &iFile))
        Terminate("readGridCGNS", cg_get_error());
    label iBase = 1;
    label iZone = 1;

    // 获取内部单元总数
    char zoneName[CHAR_DIM];
    int nZones;
    cgsize_t sizes[3];
    ZoneType_t zoneType;
    if(cg_nzones(iFile, iBase, &nZones) ||
        cg_zone_read(iFile, iBase, iZone, zoneName, sizes))
        Terminate("readZoneInfo", cg_get_error());

    // 获取内部单元总数
    int nSecs;
    if(cg_nsections(iFile, iBase, iZone, &nSecs))
        Terminate("readNSections", cg_get_error());
    int iSec;
    label nInnEles=0;
    for (int iSec = 1; iSec <= nSecs; ++iSec)
    {
        char secName[CHAR_DIM];
        cgsize_t start, end;
        ElementType_t type;
        int nBnd, parentFlag;
        if(cg_section_read(iFile, iBase, iZone, iSec, secName, 
            &type, &start, &end, &nBnd, &parentFlag))
            Terminate("readSectionInfo", cg_get_error());
        // par_std_out_("%d\n", this->meshType_);
        if(Section::compareEleType(type, Inner)) nInnEles = MAX(nInnEles, end);
    }
    if(sizes[1]!=nInnEles)
        WARNING("getInnerElementsCount", "the num doesn't match");

    // 获得每个进程在各个block中的全局起始编号
    int nBocos = this->BCSecs_.size();
    int iBoco;
    Array<label> BCType = this->getBlockTopology().getBCType();
    Array<label> faceBlockStartIdx
        = this->getBlockTopology().getFaceBlockStartIdx();
    int blockNum = faceBlockStartIdx.size()-1;
    label* allFaceBlockStartIdx = new label[blockNum*nprocs+1];
    allFaceBlockStartIdx[0] = 0;
    for (int i = 0; i < blockNum; ++i)
    {
        label eleNumInBlk = faceBlockStartIdx[i+1]-faceBlockStartIdx[i];
        if(nprocs==1)
        {
            allFaceBlockStartIdx[i*nprocs+1] = eleNumInBlk;
        } else
        {
            MPI_Allgather(&eleNumInBlk, 1, MPI_LABEL,
                &allFaceBlockStartIdx[i*nprocs+1], 1, MPI_LABEL,
                MPI_COMM_WORLD);
        }
    }
    for (int i = 0; i < blockNum*nprocs; ++i)
    {
        allFaceBlockStartIdx[i+1] += allFaceBlockStartIdx[i];
    }

    Array<label> pointListArr;
    Array<label> npnts;
    for (int iBC = 0; iBC < nBocos; ++iBC)
    {
        for (int iBlk = 0; iBlk < blockNum; ++iBlk)
        {
            label blkStartIdx
                = nInnEles+allFaceBlockStartIdx[iBlk*nprocs+rank]+1;
            for (int iEle = faceBlockStartIdx[iBlk];
                iEle < faceBlockStartIdx[iBlk+1]; ++iEle)
            {
                if((BCType_t)BCType[iEle]==BCSecs_[iBC].type)
                {
                    label iEle_g = blkStartIdx+iEle-faceBlockStartIdx[iBlk];
                    // printf("iEle: %d, iEle_g: %d, type: %s\n", iEle, iEle_g, BCSection::typeToWord(BCSecs_[iBC].type));
                    pointListArr.push_back(iEle_g);
                }
                
            }
        }

        label* pointList = new label[pointListArr.size()];
        int nBCElems = pointListArr.size();
        for (int i = 0; i < pointListArr.size(); ++i)
        {
            pointList[i] = pointListArr[i];
        }
        // 获取各个进程在该边界条件下的边界单元数量
        int *nBCElems_mpi = new int[nprocs];
        int *disp = new int[nprocs+1];
        MPI_Allgather(&nBCElems, 1, MPI_INT, nBCElems_mpi, 1, MPI_INT, MPI_COMM_WORLD);
        int totBCElems;
        label *BCElems;
        totBCElems = 0;
        disp[0] = 0;
        for (int i = 0; i < nprocs; ++i)
        {
            par_std_out_("rank %d have %d elements\n", i, nBCElems_mpi[i]);
            totBCElems += nBCElems_mpi[i];
            disp[i+1] = disp[i] + nBCElems_mpi[i];
        }
        par_std_out_("rank %d now have %d elements\n", rank, totBCElems);
        BCElems = new label[totBCElems];
        MPI_Allgatherv(pointList, nBCElems, MPI_LABEL, BCElems, nBCElems_mpi,
            disp, MPI_LABEL, MPI_COMM_WORLD);

        // if(pointListArr.size()==0) {continue;}
        this->BCSecs_[iBC].nBCElems = (cgsize_t)totBCElems;
        this->BCSecs_[iBC].BCElems  = new cgsize_t[totBCElems];
        for (int i = 0; i < totBCElems; ++i)
        {
            this->BCSecs_[iBC].BCElems[i] = (cgsize_t)BCElems[i];
        }
        par_std_out_("write Boundary condition with ElementRange type\n");
        Word str = BCSection::typeToWord(BCSecs_[iBC].type);
        // str.append("_bc");
        // printf("%s\n", str.c_str());
        strcpy(this->BCSecs_[iBC].name, str.c_str());
        if(cg_boco_write(iFile, iBase, iZone, this->BCSecs_[iBC].name,
            this->BCSecs_[iBC].type, PointList, 
            this->BCSecs_[iBC].nBCElems, this->BCSecs_[iBC].BCElems, &iBoco))
            Terminate("writeBC", cg_get_error());
        par_std_out_("finish write the %dth BC\n", iBC);
        if(cg_boco_gridlocation_write(iFile, iBase, iZone, iBoco,
            this->BCSecs_[iBC].location))
            Terminate("writeGridLocation",cg_get_error());
        pointListArr.clear();
        MPI_Barrier(MPI_COMM_WORLD);
    }
    if(cg_close(iFile))
        Terminate("closeCGNSFile", cg_get_error());
}

void Boundary::exchangeBoundaryElements(Topology& innerTopo)
{
    int rank,nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    label BCSecStart;
    if(rank==0) BCSecStart = this->getSections()[0].iStart;
    MPI_Bcast(&BCSecStart, 1, MPI_LABEL, 0, MPI_COMM_WORLD);


    ArrayArray<label> cell2Node = innerTopo.getCell2Node();
    Array<label> cellType = innerTopo.getCellType();

    label cellStartId = innerTopo.getCellStartId();

    ArrayArray<label> face2NodeBnd = getTopology().getCell2Node();
    Array<label> faceType = getTopology().getCellType();
    // par_std_out_("%d,%d\n", getSections().size(),this->secs_.size());
    Array<Array<label> > face2NodeBndArr;
    transformArray(face2NodeBnd, face2NodeBndArr);



    label cellNum = cell2Node.size();
    Array<Array<label> > faces2NodesTmp;
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
            // 局部编号
            face2NodeTmp.push_back(i+cellStartId);
            faces2NodesTmp.push_back(face2NodeTmp);
        }
    }

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
    Array<Array<label> > face2NodeOwn, face2NodeNei;
    Array<label> face2CellOwn;
    // for (int i = 0; i < faces2NodesTmp.size(); ++i){isInner[i] = false;}
    Array<label> BCTypeTmp, faceTypeTmp;
    BCTypeTmp.swap(BCType_);
    faceTypeTmp.swap(faceType);
    for (int i = 0; i < face2NodeBndArr.size(); ++i)
    {
        Array<label> tmp;
        tmp.assign(face2NodeBndArr[i].begin(), face2NodeBndArr[i].end());
        sort(tmp.begin(), tmp.end());
        // if(isInner[i]) continue;
        // int end = 0;
        int end = findSortedArray(faces2NodesTmp, tmp, 1, faces2NodesTmp.size()-1);
        // printf("%d, %d\n", i, end);
        if(end==-1)
        {
            // 将边界条件类型添加到cell2node数组后面准备发送
            // 将边界单元类型添加到cell2node数组后面准备发送
            face2NodeBndArr[i].push_back(BCTypeTmp[i]);
            face2NodeBndArr[i].push_back(faceTypeTmp[i]);
            face2NodeNei.push_back(face2NodeBndArr[i]);
        } else
        {
            label ownerId 
                = faces2NodesTmp[end][faces2NodesTmp[end].size()-1];
            // 局部编号
            face2CellOwn.push_back(ownerId);
            // 本地存储原始face2node顺序关系
            face2NodeOwn.push_back(face2NodeBndArr[i]); 
            // 将本地边界条件添加到边界条件数组中
            // 将本地边界类型添加到边界类型数组中
            BCType_.push_back(BCTypeTmp[i]);           
            faceType.push_back(faceTypeTmp[i]);
        }
    }


    // par_std_out_("rank: %d, owner face: %d, neighbor face: %d\n", rank, face2NodeOwn.size(), face2NodeNei.size());

    Array<label> face2CellNew;
    ArrayArray<label> face2NodeNeiTmp;
    transformArray(face2NodeNei, face2NodeNeiTmp);
    Array<Array<label> > face2NodeBndOwn = innerTopo.getFace2NodeBnd();
    Array<Array<label> > face2CellBndOwn = innerTopo.getFace2CellBnd();
    Array<label> face2CellBndOwnTmp;
    // par_std_out_("rank: %d, boundary face: %d\n", rank, face2NodeBndOwn.size());
    for (int i = 0; i < face2CellBndOwn.size(); ++i)
    {
        face2CellBndOwnTmp.push_back(face2CellBndOwn[i][0]);
    }

    // 将不属于本进程的边界面发送给相应进程
    int bndFaces = face2NodeNeiTmp.num;

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
        // if(rank==0) par_std_out_("rank: %d, bndFaces: %d\n", i, bndFaces_mpi[i]);
    }
    label* startIdx_mpi = new label[bndFacesSum+nprocs];
#if 1
    MPI_Allgatherv(face2NodeNeiTmp.startIdx, bndFaces+1, MPI_LABEL, startIdx_mpi, countIdx_r, dispIdx_r, MPI_LABEL, MPI_COMM_WORLD);
    int bndNodesSum = 0;
    int countData_r[nprocs], dispData_r[nprocs];
    for (int i = 0; i < nprocs; ++i)
    {
        countData_r[i] = startIdx_mpi[dispIdx_r[i]+countIdx_r[i]-1];
        dispData_r[i] = bndNodesSum;
        bndNodesSum += countData_r[i];
        // if(rank==0) par_std_out_("count: %d, displacement: %d\n", countData_r[i], dispData_r[i]);
    }
    label* data_mpi = new label[bndNodesSum];
    MPI_Allgatherv(face2NodeNeiTmp.data, face2NodeNeiTmp.startIdx[bndFaces],
        MPI_LABEL, data_mpi, countData_r, dispData_r, MPI_LABEL, MPI_COMM_WORLD);
    label* neighborCellIdx_mpi = new label[bndFacesSum+nprocs];
#endif
    for (int i = 0; i < bndFacesSum+nprocs; ++i) { neighborCellIdx_mpi[i] = -1; }
    // if(rank==2)
    {
        for (int i = 0; i < nprocs; ++i)
        {
            if(rank==i) continue;
            // par_std_out_("rank: %d, start from: %d, read %d\n", i, dispIdx_r[i], countIdx_r[i]);
            label* startIdx = &startIdx_mpi[dispIdx_r[i]];
            for (int j = 0; j < countIdx_r[i]-1; ++j)
            {
                // par_std_out_("The %dth element: ", j);
                Array<label> face2NodeTmp;
                // 最后俩个元素为边界条件类型和边界单元类型，不进行比较
                for (int k = startIdx[j]; k < startIdx[j+1]-2; ++k)
                {
                    // par_std_out_("%d, ", data_mpi[dispData_r[i]+k]);
                    face2NodeTmp.push_back(data_mpi[dispData_r[i]+k]);

                }
                Array<label> tmp;
                tmp.assign(face2NodeTmp.begin(),face2NodeTmp.end());
                sort(tmp.begin(),tmp.end());
                label idx = findSortedArray(face2NodeBndOwn, tmp, 0, face2NodeBndOwn.size()-1);
                // if(rank==1 && idx!=-1) par_std_out_("%d, %d\n", idx, face2CellBndOwnTmp[idx]);
                // // label idx = findArray(bndFaceArr, face2NodeTmp);
                // // label idx = -1;
                // if(idx!=-1) neighborCellIdx_mpi[dispIdx_r[i]+j] = face2CellBndOwnTmp[idx];
                // else neighborCellIdx_mpi[dispIdx_r[i]+j]=-1;

                if(idx!=-1)
                {
                    // 将最后一个元素即边界条件赋值给边界条件数组
                    BCType_.push_back(data_mpi[dispData_r[i]+startIdx[j+1]-2]);
                    faceType.push_back(data_mpi[dispData_r[i]+startIdx[j+1]-1]);

                    face2NodeOwn.push_back(face2NodeTmp);
                    face2CellOwn.push_back(face2CellBndOwnTmp[idx]);
                }
                // if(idx==-1) par_std_out_("not found\n");
                // else par_std_out_("%d\n", idx);
            }
            // par_std_out_("\n");
        }
    }

    // for (int i = 0; i < BCType_.size(); ++i)
    // {
    //     printf("%d, %d\n", i, BCType_[i]);
    // }

    Array<Array<label> > face2CellOwnArr;
    for (int i = 0; i < face2CellOwn.size(); ++i)
    {
        Array<label> tmp;
        tmp.push_back(face2CellOwn[i]);
        face2CellOwnArr.push_back(tmp);
    }

    // 根据节点个数确定网格面类型，并将网格面分开
    Array<label> faceNodeNum;
    for (int i = 0; i < face2NodeOwn.size(); ++i)
    {
        bool isExist = false;
        for (int j = 0; j < faceNodeNum.size(); ++j)
        {
            if(face2NodeOwn[i].size()==faceNodeNum[j])
            {
                isExist=true;
                break;
            }
        }
        if(!isExist)
        { 
            faceNodeNum.push_back(face2NodeOwn[i].size());
        }
    }
    // 根据网格面类型对网格面拓扑进行重排，使相同类型网格面连续存储
    Array<Array<label> > BCTypeWithType(faceNodeNum.size());
    Array<label> FaceTypeWithType(faceNodeNum.size());
    Array<Array<Array<label> > > face2NodeWithType(faceNodeNum.size());
    Array<Array<Array<label> > > face2CellWithType(faceNodeNum.size());
    for (int i = 0; i < face2NodeOwn.size(); ++i)
    {
        bool isExist = false;
        for (int j = 0; j < faceNodeNum.size(); ++j)
        {
            // 通过node数量判断单元类型
            if(face2NodeOwn[i].size()==faceNodeNum[j])
            {
                isExist=true;
                BCTypeWithType[j].push_back(BCType_[i]);
                face2NodeWithType[j].push_back(face2NodeOwn[i]);
                face2CellWithType[j].push_back(face2CellOwnArr[i]);
                FaceTypeWithType[j] = faceType[i];
            }
        }
        if(!isExist) Terminate("sort the faces according to the type", "unrecognized type");
    }
    par_std_out_("There are %d types of faces.\n", faceNodeNum.size());

    // 从所有进程中找出最大block数目
    int blockNum = faceNodeNum.size();
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
            buffer[i] = FaceTypeWithType[i];
        }
    }
    MPI_Bcast(buffer, maxBlockNum, MPI_LABEL, irank, MPI_COMM_WORLD);

    // 按照最大进程网格单元排布重排本进程block
    Array<Array<label> > face2NodeBlk;
    Array<Array<label> > face2CellBlk;
    Array<label> faceTypeBlk;
    Array<label> BCTypeBlk;
    Array<label> faceBlockStartIdx;
    faceBlockStartIdx.push_back(0);
    for (int i = 0; i < maxBlockNum; ++i)
    {
        bool flag;
        for (int j = 0; j < blockNum; ++j)
        {
            flag = FaceTypeWithType[j]==buffer[i];
            par_std_out_("%d, %d, %d, %d\n",i,j,FaceTypeWithType[j],buffer[i]);
            // 如果该block与最大进程排布相同，则压入该block信息
            if(flag)
            {
                faceTypeBlk.push_back(FaceTypeWithType[j]);
                faceBlockStartIdx.push_back(face2NodeWithType[j].size());
                face2NodeBlk.insert(face2NodeBlk.end(), 
                    face2NodeWithType[j].begin(), face2NodeWithType[j].end());
                face2CellBlk.insert(face2CellBlk.end(), 
                    face2CellWithType[j].begin(), face2CellWithType[j].end());
                BCTypeBlk.insert(BCTypeBlk.end(), 
                    BCTypeWithType[j].begin(), BCTypeWithType[j].end());                
                break;
            }
        }
        // 如果本进程不存在该block，则数目为0，类型置为-1
        if(!flag)
        {
            faceTypeBlk.push_back(buffer[i]);
            faceBlockStartIdx.push_back(0);
        }
    }
    par_std_out_("Now I have %d blocks\n", faceTypeBlk.size());

    // 生成block Topology拓扑
    BlockTopology& blkTopo = getBlockTopology();
    for (int i = 0; i < faceBlockStartIdx.size(); ++i)
    {
        faceBlockStartIdx[i+1] += faceBlockStartIdx[i];
        par_std_out_("block: %d, start: %d\n", i, faceBlockStartIdx[i+1]);
    }
    blkTopo.setFace2Node(face2NodeBlk);
    blkTopo.setFace2Cell(face2CellBlk);
    blkTopo.setFaceType(faceTypeBlk);
    blkTopo.setBCType(BCTypeBlk);
    blkTopo.setFaceBlockStartIdx(faceBlockStartIdx);

    this->getTopology().setFace2Node(face2NodeOwn);
    this->getTopology().setFace2Cell(face2CellOwnArr);
    this->getTopology().setCellType(faceType);
    this->getTopology().setCell2Node(face2NodeOwn);
    // printf("%d\n", face2NodeOwn.size());
    // label* cellNum_mpi = new label[nprocs];
    // label cellNum_local = face2NodeOwn.size();
    // par_std_out_("rank: %d, cellNum: %d\n", rank, cellNum_local);
    // MPI_Allgather(&cellNum_local, 1, MPI_INT, cellNum_mpi, 1, MPI_LABEL, MPI_COMM_WORLD);
    // for (int i = 0; i < nprocs-1; ++i)
    // {
    //     if(rank==0) par_std_out_("%d\n", cellNum_mpi[i]);
    //     cellNum_mpi[i+1] += cellNum_mpi[i];
    // }
    // this->getTopology().setCellStartId(cellNum_mpi[rank]);
    DELETE_POINTER(neighborCellIdx_mpi);
    DELETE_POINTER(data_mpi);
    DELETE_POINTER(startIdx_mpi);
    DELETE_POINTER(bndFaces_mpi);
    DELETE_POINTER(isInner);
}

void Boundary::writeMesh(const char* filePtr)
{
    // par_std_out_("This is boundary!!!!!\n");
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    int iFile, nBases;
    int iBase=1, iZone=1;
    char basename[CHAR_DIM];

    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
    if(cgp_open(filePtr, CG_MODE_MODIFY, &iFile))
        Terminate("readBaseInfo", cg_get_error())

    // 读取已有Section网格单元编号
    int nSecs;
    if(cg_nsections(iFile, iBase, iZone, &nSecs))
        Terminate("readNSections", cg_get_error());
    // par_std_out_("nSecs: %d\n", nSecs);
    int iSec;
    cgsize_t start, end;
    for (int iSec = 1; iSec <= nSecs; ++iSec)
    {
        char secName[CHAR_DIM];
        ElementType_t type;
        int nBnd, parentFlag;
        if(cg_section_read(iFile, iBase, iZone, iSec, secName, 
            &type, &start, &end, &nBnd, &parentFlag))          
            Terminate("readSectionInfo", cg_get_error());
    }
    // write connectivity
    ArrayArray<label> conn = this->getBlockTopology().getFace2Node();
    Array<label> faceType = this->getBlockTopology().getFaceType();
    Array<label> faceBlockStartIdx
        = this->getBlockTopology().getFaceBlockStartIdx();
    label *cellStartId = new label[numProcs+1];
    cellStartId[0] = end;
    

    int blockNum = faceBlockStartIdx.size()-1;
    for (int iBlk = 0; iBlk < blockNum; ++iBlk)
    {
        label num = faceBlockStartIdx[iBlk+1]-faceBlockStartIdx[iBlk];
        par_std_out_("%d\n", iBlk);
        ElementType_t eleType = (ElementType_t)faceType[iBlk];
        MPI_Allgather(&num, 1, MPI_LABEL, &cellStartId[1], 1,
            MPI_LABEL, MPI_COMM_WORLD);
        par_std_out_("%d\n", num);
        for (int i = 0; i < numProcs; ++i)
        {
            cellStartId[i+1] += cellStartId[i];
        }
        par_std_out_("write section from %d to %d\n", cellStartId[0]+1, cellStartId[numProcs]);
        if(cgp_section_write(iFile, iBase, iZone,
            Section::typeToWord(eleType), eleType, cellStartId[0]+1,
            cellStartId[numProcs], 0, &iSec))
            Terminate("writeSecInfo", cg_get_error());
        par_std_out_("rank %d write from %d to %d\n", rank, cellStartId[rank]+1, cellStartId[rank+1]);
        cgsize_t *data = (cgsize_t*)&conn.data[conn.startIdx[faceBlockStartIdx[iBlk]]];
        if(num==0) cellStartId[rank+1] = cellStartId[rank]+1;
        if(cgp_elements_write_data(iFile, iBase, iZone, iSec, cellStartId[rank]+1,
            cellStartId[rank+1], data))
            Terminate("writeSecConn", cg_get_error());
        cellStartId[0] = cellStartId[numProcs];
    }

    if(cgp_close(iFile))
        Terminate("closeCGNSFile",cg_get_error());

    DELETE_POINTER(cellStartId);
}

void generateBlockTopo()
{

}

void Boundary::initBoundaryConditionType()
{

    BCSecs_[0].type = BCWall;
    BCSecs_[1].type = BCOutflow;
    BCSecs_[2].type = BCInflow;

    Array<Section> secs = this->getSections();
    for (int i = 0; i < secs.size(); ++i)
    {
        printf("%d, %d\n", secs[i].iStart, secs[i].iEnd);
        for (int j = 0; j < secs[i].num; ++j)
        {
            bool flag;
            for (int k = 0; k < BCSecs_.size(); ++k)
            {
                flag = BCSecs_[k].findBCType(j+secs[i].iStart);
                if(flag)
                {
                    BCType_.push_back(BCSecs_[k].type);
                    break;
                }
            }
            if(!flag)
                Terminate("findElementType", "the element type can not be found");
        }
    }
    // for (int i = 0; i < BCType_.size(); ++i)
    // {
    //     printf("%d, %d\n", i, BCType_[i]);
    // }
}

} // end namespace HSF

    // for (int iBlk = 0; iBlk < blockNum; ++iBlk)
    // {
    //     Array<Array<label> > tmpConnArr;
    //     for (int iEle = 0; iEle < BCType.size(); ++iEle)
    //     {
    //         if((BCType_t)BCType[iEle]==BCSecs_[iBC].type)
    //         {
    //             Array<label> tmp;
    //             for (int i = conn.startIdx[iEle]; i < conn.startIdx[iEle+1]; ++i)
    //             {
    //                 tmp.push_back(conn.data[i]);
    //             }
    //             tmpConnArr.push_back(tmp);
    //             eleType = (ElementType_t)cellType[iEle];
    //         }
    //     }
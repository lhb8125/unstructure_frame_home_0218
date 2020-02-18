/**
* @file: region.cpp
* @author: Liu Hongbin
* @brief:
* @date:   2019-10-14 09:17:17
* @last Modified by:   lenovo
* @last Modified time: 2020-01-09 17:17:46
*/
#include <algorithm>
#include "region.hpp"

using namespace std;

namespace HSF
{

bool compare(pair<label, pair<label, label> > a, pair<label, pair<label, label> > b)
{
    if(a.first == b.first)
    {
        if(a.second.first ==  b.second.first)
        {
            return a.second.second <  b.second.second;
        }

        return a.second.first < b.second.first;
    }

    return a.first < b.first;
}


void Region::initBeforeBalance(Array<char*> meshFile)
{
	// strncpy(meshFile_, meshFile, sizeof(meshFile_));
	// this->meshFile_[sizeof(meshFile)-1]='/0';
    meshFile_.assign(meshFile.begin(), meshFile.end());
	par_std_out_("start reading mesh ...\n");
	this->getMesh().readMesh(meshFile);
	par_std_out_("finish reading mesh ...\n");
	MPI_Barrier(MPI_COMM_WORLD);
}

void Region::initAfterBalance()
{
	par_std_out_("start constructing topology ...\n");
	this->getMesh().fetchNodes(this->meshFile_);
    // this->getMesh().fetchNodes(this->meshFile_[1]);
	this->getMesh().getTopology().constructTopology();
	par_std_out_("finish constructing topology ...\n");
	MPI_Barrier(MPI_COMM_WORLD);

    /// 创建通信对
	par_std_out_("start creating interfaces ...\n");
    label cellNum = this->getMesh().getTopology().getCellsNum();
    Array<Array<label> > faceCells
        = this->getMesh().getTopology().getFace2CellPatch();

    createInterFaces(faceCells, cellNum);
	par_std_out_("finish creating interfaces ...\n");

	par_std_out_("start reading boundary mesh ...\n");
	this->getBoundary().readMesh(this->meshFile_);
    this->getBoundary().readBC(this->meshFile_);
	par_std_out_("finish reading boundary mesh ...\n");
	Topology innerTopo = this->getMesh().getTopology();
	par_std_out_("start constructing boundary topology ...\n");
	this->getBoundary().exchangeBoundaryElements(innerTopo);
	par_std_out_("finish constructing boundary topology ...\n");

    par_std_out_("start initialize mesh information ...\n");
    this->meshInfo_.init(mesh_);
    par_std_out_("finish initialize mesh information ...\n");

    par_std_out_("start generate block topology ...\n");
    this->getMesh().generateBlockTopo();
    // this->getBoundary().generateBlockTopo();
    par_std_out_("finish generate block topology ...\n");

}

void Region::writeMesh(char* meshFile)
{
    par_std_out_("start write inner mesh ...\n");
	this->getMesh().writeMesh(meshFile);
    par_std_out_("finish write inner mesh ...\n");
    par_std_out_("start write boundary mesh ...\n");
	this->getBoundary().writeMesh(meshFile);
    par_std_out_("finish write boundary mesh ...\n");
    par_std_out_("start write boundary condition ...\n");
    this->getBoundary().writeBC(meshFile);
    par_std_out_("finish write boundary condition ...\n");
}

/// guhf
void Region::createInterFaces
(
    Array<Array<label> >& faceCells,
    label cellNum
)
{
    int nProcs, myProcNo;

    MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myProcNo);

    label* partitionInfo = new label[nProcs + 1];

    partitionInfo[0] = 0;
    MPI_Allgather
    (
        &cellNum,
        1,
        MPI_LABEL,
        &partitionInfo[1],
        1,
        MPI_LABEL,
        MPI_COMM_WORLD
    );

    for (int i = 0; i < nProcs; ++i)
    {
    	partitionInfo[i+1] += partitionInfo[i];
    }
    // partitionInfo[nProcs] = nCellsAll - 1;
    label nCellsAll = partitionInfo[nProcs];

    if(!patchTabPtr_)
    {
        //- create
        patchTabPtr_ = new Table<Word, Table<Word, Patch*>*>;
    }

    Table<Word, Table<Word, Patch*>*>& allPatchTab = *patchTabPtr_;

    Word patchType = "face";

    allPatchTab[patchType] = new Table<Word, Patch*>;

    Table<label, label> facesInProcessor;
    Array< pair<label, pair<label, label> > > vec;

    for(label i=0; i<faceCells.size(); ++i)
    {
        label IDIn, IDOut;
        IDIn = faceCells[i][0];
        IDOut = faceCells[i][1];

        bool FIND = false;
        //- get the assumed neighbor processor ID,
        //- by assuming that all cells are partitioned uniformly
        label procAssume = IDOut / (nCellsAll / nProcs);

        if(IDOut < nCellsAll)
        {
            if(procAssume > nProcs - 1) procAssume = nProcs - 1;

            do
            {
                if(IDOut >= partitionInfo[procAssume + 1])
                {
                    procAssume++;
                }
                else if(IDOut < partitionInfo[procAssume])
                {
                    procAssume--;
                }
                else
                {
                    //- do nothing
                    FIND = true;
                }
            }while(!FIND);
        }
        else
        {
            printf("Error: cell ID exceeds the total cell number: cell ID = %d, total number = %d!\n", IDOut, nCellsAll);
        }



        //- smaller IDs are placed in the left
        //- thus all processors will follow the same discipline
        //- and produce the same patch faces order
        if(IDIn > IDOut)
        {
            int temp = IDIn;
            IDIn = IDOut;
            IDOut = temp;
        }

        vec.push_back(make_pair(procAssume, make_pair(IDIn, IDOut)));
    }

    //- sort faces in order of neighbor processors
    std::sort(vec.begin(), vec.end(), compare);

    Table<label, Array<label> > patchMap;

    //- split the faces in order of neighbor processors
    for (label i = 0; i < vec.size(); ++i)
    {
        label NbrProcID = vec[i].first;
        label cellID1 = vec[i].second.first;
        label cellID2 = vec[i].second.second;
        label localCellID;

        //- find the cellIDs belonging to current processor
        //- and storage them
        if(cellID1 >= partitionInfo[myProcNo] &&
           cellID1 <  partitionInfo[myProcNo+1])
        {
            localCellID = cellID1;
        }
        else if(cellID2 >= partitionInfo[myProcNo] &&
                cellID2 <  partitionInfo[myProcNo+1])
        {
            localCellID = cellID2;
        }
        else if(cellID2 == partitionInfo[myProcNo+1])
        {
            localCellID = cellID2;
        }
        else
        {
            par_std_out_("Error: cell is not in the target Processor, please check! At proc = %d, elements from %d to %d, cell1 = %d, cell2 = %d\n",
            	myProcNo, partitionInfo[myProcNo], partitionInfo[myProcNo+1], cellID1, cellID2);
            exit(-1);
        }

        patchMap[NbrProcID].push_back(localCellID);
    }

    Table<Word, Patch*>& patches = *allPatchTab[patchType];
    Table<label, Array<label> >::iterator it;
    for(it = patchMap.begin(); it!=patchMap.end(); it++)
    {
        label nbrID = it->first;
        label faceSize = (it->second).size();

        label* face2CellIDs = new label[faceSize];
        Array<label> array1 = it->second;

        for(label j=0; j<faceSize; ++j)
        {
            face2CellIDs[j] = array1[j] - partitionInfo[myProcNo];
        }
        //- construct the patch
        patches[to_string(nbrID)] = new Patch(faceSize, face2CellIDs, nbrID);
        DELETE_POINTER(face2CellIDs);
    }

    DELETE_POINTER(partitionInfo);
}


void Region::initFields(Word fieldName)
{
    // Table<Word, scalarField*>::iterator it = fieldTable_.find(fieldName);
    // if(it == fieldTable_.end())
    // {
    //  cout << "Error: can not find this field: " << fieldName << endl;
    //  ERROR_EXIT;
    // }

    // scalarField* fieldSend = it->second;
    // Word fieldType = fieldSend->getType();

    // //- choose proper interface: face, node

    // for(label i=0; i<interFaces_.size(); ++i)
    // {
    //  Patch* pp = interFaces_[i].second;
    //  Word patchType = pp->getType();
    //  if(fieldType == patchType)
    //  {
    //      pp.send(*fieldSend);
    //  }
    // }
}

void Region::updateFields(Word fieldName)
{
    // Table<Word, scalarField*>::iterator it = fieldTable_.find(fieldName);
    // if(it == fieldTable_.end())
    // {
    //  cout << "Error: can not find this field: " << fieldName << endl;
    //  ERROR_EXIT;
    // }

    // scalarField* fieldSend = it->second;
    // Word fieldType = fieldSend->getType();

}


Region::~Region()
{
    //- free interfaces created

    //- delete Table<Word, Table<Word, Patch*>*>*
    if(patchTabPtr_)
    {
        Table<Word, Table<Word, Patch*>*>& allPatchTab = *patchTabPtr_;
        Table<Word, Table<Word, Patch*>*>::iterator it1;

        //- delete Table<Word, Patch*>*
        for(it1= allPatchTab.begin(); it1!=allPatchTab.end(); it1++)
        {
            Table<Word, Patch*>* patchesPtr = it1->second;
            Table<Word, Patch*>& patches = *patchesPtr;
            Table<Word, Patch*>::iterator it2;
            //- delete Patch*
            for(it2=patches.begin(); it2!=patches.end(); it2++)
            {
                Patch* patchIPtr = it2->second;
                DELETE_OBJECT_POINTER(patchIPtr);
            }
            DELETE_OBJECT_POINTER(patchesPtr)
        }
        DELETE_OBJECT_POINTER(patchTabPtr_);
    }
}

} // end namespace HSF
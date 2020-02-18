/*
The MIT License

Copyright (c) 2019 Hanfeng GU <hanfenggu@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
* @File: regionI.hpp
* @Author: Hanfeng
* @Email:
* @Date:   2019-11-28 14:09:06
* @Last Modified by:   Hanfeng
* @Last Modified time: 2019-11-30 14:56:17
*/

/*
* @brief: Implementation of template functions in Region class
*/
// #ifndef REGIONI_HPP
// #define REGIONI_HPP

template<typename T>
Field<T>& Region::getField(const Word fieldType, const Word fieldName)
{
    typename Table<Word, Table<Word, Field<T>*>*>::iterator it1;
    typename Table<Word, Table<Word, Field<T>*>*>::iterator it2;

    void* fieldTabPtr = NULL;

    if(typeid(T) == typeid(label))
    {
        fieldTabPtr = labelFieldTabPtr_;
    }
    else if(typeid(T) == typeid(scalar))
    {
        fieldTabPtr = scalarFieldTabPtr_;
    }
    else
    {
        cout << "No this type field yet!" << endl;
        ERROR_EXIT;
    }

    Table<Word, Table<Word, Field<T>*>*>* ft = static_cast<Table<Word, Table<Word, Field<T>*>*>*>(fieldTabPtr);

    it1 = (*ft).find(fieldType);
    it2 = (*ft).end();

    if(it1 == it2)
    {
        cout << "There is no this type in field table: " << fieldType << endl;
        ERROR_EXIT;
    }
    else
    {
        Table<Word, Field<T>*>& fields = *(it1->second);

        typename Table<Word, Field<T>*>::iterator it3 = fields.find(fieldName);

        if(it3 == fields.end())
        {
            cout << "There is no this type in field table: " << fieldName << endl;
            ERROR_EXIT;
        }
        else
        {
            Field<T>& fieldI = *(fields[fieldName]);
            return fieldI;
        }
    }
}


template<typename T>
void Region::addField
(
    Word name,
    Field<T>* f
)
{
    typename Table<Word, Table<Word, Field<T>*>*>::iterator it1;
    typename Table<Word, Table<Word, Field<T>*>*>::iterator it2;

    void* fieldTabPtrPtr;

    if(typeid(T) == typeid(label))
    {
        fieldTabPtrPtr = &labelFieldTabPtr_;
    }
    else if(typeid(T) == typeid(scalar))
    {
        fieldTabPtrPtr = &scalarFieldTabPtr_;
    }
    else
    {
        cout << "No this type field yet!" << endl;
        ERROR_EXIT;
    }

    #define FIELDTABPTR (*(Table<Word, Table<Word, Field<T>*>*>**)fieldTabPtrPtr)

    if(!FIELDTABPTR)
    {
        FIELDTABPTR = new Table<Word, Table<Word, Field<T>*>*>;
    }

    Word setType = f->getType();

    if(!(*FIELDTABPTR)[setType])
    {
        (*FIELDTABPTR)[setType] = new Table<Word, Field<T>*>;
    }

    Table<Word, Field<T>*>& fields = *((*FIELDTABPTR)[setType]);
    fields[name] = f;

    if(patchTabPtr_)
    {
        Table<Word, Table<Word, Patch*>*>::iterator it = (*patchTabPtr_).find(setType);
        if(it != (*patchTabPtr_).end())
        {
            f->setPatchTab((*patchTabPtr_)[setType]);
        }
    }

    #undef FIELDTABPTR
}

template<typename T>
void Region::initField(Word fieldType, Word fieldName)
{
    Field<T>& fieldI = getField<T>(fieldType, fieldName);
    fieldI.initSend();
}


template<typename T>
void Region::updateField(Word fieldType, Word fieldName)
{
    Field<T>& fieldI = getField<T>(fieldType, fieldName);
    fieldI.checkSendStatus();
}


template<typename T>
void Region::deleteField(Word fieldType, Word fieldName)
{
    typename Table<Word, Table<Word, Field<T>*>*>::iterator it1;
    typename Table<Word, Table<Word, Field<T>*>*>::iterator it2;

    void* fieldTabPtr = NULL;

    if(typeid(T) == typeid(label))
    {
        fieldTabPtr = labelFieldTabPtr_;
    }
    else if(typeid(T) == typeid(scalar))
    {
        fieldTabPtr = scalarFieldTabPtr_;
    }

    if(fieldTabPtr)
    {
        Table<Word, Table<Word, Field<T>*>*>* ft = static_cast<Table<Word, Table<Word, Field<T>*>*>*>(fieldTabPtr);

        it1 = (*ft).find(fieldType);
        it2 = (*ft).end();

        if(it1 != it2)
        {
            Table<Word, Field<T>*>& fields = *(it1->second);

            typename Table<Word, Field<T>*>::iterator it3 = fields.find(fieldName);

            if(it3 != fields.end())
            {
                fields.erase(fieldName);
            }
        }
    }
}

template<typename T>
void Region::writeField(const char* resFile,
    const char* fieldName, const char* fieldType)
{
    int rank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    // printf("This is rank %d in %d processes\n", rank, numProcs);

    int iFile, nBases, cellDim, physDim, Cx, Cy, Cz;
    int iBase=1, iZone=1;
    char basename[CHAR_DIM];

    if(cgp_mpi_comm(MPI_COMM_WORLD) != CG_OK)
        Terminate("initCGNSMPI", cg_get_error());
    if(cgp_open(resFile, CG_MODE_MODIFY, &iFile))
        Terminate("writeBaseInfo", cg_get_error());
    MPI_Barrier(MPI_COMM_WORLD);

    Field<T>& fieldI = getField<T>(fieldType, fieldName);
    T* dataPtr = fieldI.getLocalData();
    label ndim = fieldI.getDim();
    label nCells = fieldI.getSize();
    par_std_out_("resFile: %s, fieldName: %s, fieldType: %s, dim: %d, num: %d\n", resFile, fieldName, fieldType, ndim, nCells);

    DataType_t dataType;
    if(typeid(T) == typeid(label))
    {
        if(sizeof(label)==8) dataType = LongInteger;
        else dataType = Integer;
    }
    else if(typeid(T) == typeid(scalar))
    {
        if(sizeof(scalar)==8) dataType = RealDouble;
        else dataType = RealSingle;
    }
    GridLocation_t location;
    char* solName;
    if(strcmp(fieldType, "cell")==0)
    {
        location = CellCenter;
        solName="CellCenterSolution";
    }else if(strcmp(fieldType, "face")==0)
    {
        location = FaceCenter;
        solName = "FaceCenterSolution";
    }else if(strcmp(fieldType, "node")==0)
    {
        location = Vertex;
        solName = "VertexSolution";
    }else
        Terminate("writeField", "unknown field type, it must be cell, face or node");

    /// 流场变量
    int S, Fs, A, nSols;
    if(cg_nsols(iFile, iBase, iZone, &nSols))
        Terminate("readSolutionInfo", cg_get_error());
    S=-1;
    for (int i = 1; i <= nSols; ++i)
    {
        GridLocation_t solLoc;
        char solNameTmp[CHAR_DIM];
        cg_sol_info(iFile, iBase, iZone, i, solNameTmp, &solLoc);
        if(solLoc==location) S=i;
    }
    if(nSols==0 || S==-1)
        cg_sol_write(iFile, iBase, iZone, solName, location, &S);
    // cg_field_write(iFile, iBase, iZone, S, dataType, fieldName, dataPtr, &Fs);
    if(cgp_field_write(iFile, iBase, iZone, S, dataType, fieldName, &Fs))
        Terminate("writeSolutionInfo", cg_get_error());

    label *cellStartId = new label[numProcs+1];
    label num = nCells*ndim;
    MPI_Allgather(&num, 1, MPI_LABEL, &cellStartId[1], 1, MPI_LABEL, MPI_COMM_WORLD);
    cellStartId[0] = 0;
    for (int i = 0; i < numProcs; ++i)
    {
        cellStartId[i+1] += cellStartId[i];
    }
    cgsize_t start = cellStartId[rank]+1;
    cgsize_t end = cellStartId[rank+1];
    // cgsize_t end = start+1;
    // printf("rank: %d, %d, %d\n", rank, start, end);
    // for (int i = 0; i < end-start+1; ++i)
    // {
    //     std::cout<<i<<", "<<dataPtr[i]<<std::endl;
    // }
    if(cgp_field_write_data(iFile, iBase, iZone, S, Fs, &start,
        &end, dataPtr))
        Terminate("writeSolutionData", cg_get_error());

    if(cgp_close(iFile))
        Terminate("closeCGNSFile",cg_get_error());

    DELETE_POINTER(cellStartId);
}
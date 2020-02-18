/**
* @file: region.hpp
* @author: Liu Hongbin
* @brief:
* @date:   2019-10-14 09:17:17
* @last Modified by:   lenovo
* @last Modified time: 2020-01-07 10:40:09
*/
#ifndef REGION_HPP
#define REGION_HPP

#include <typeinfo>
#include "pcgnslib.h"
#include "cgnslib.h"
#include "mpi.h"
#include "mesh.hpp"
#include "meshInfo.hpp"
#include "boundary.hpp"
#include "patch.hpp"
#include "field.hpp"

namespace HSF
{

/**
 * @brief      This class describes a region.
 */
class Region
{
private:
	Array<char*> meshFile_; ///< mesh file name

	Mesh mesh_; ///< internal mesh

    MeshInfo meshInfo_; ///< mesh information

	Boundary boundary_; ///< boundary mesh and condition

	/// guhf
	Table<Word, Table<Word, Patch*>*>* patchTabPtr_; ///< communication topology

    Table<Word, Table<Word, scalarField*>*>* scalarFieldTabPtr_; ///< scalar field

    Table<Word, Table<Word, labelField*>*>* labelFieldTabPtr_; ///< label field

	Word setType_; ///< data set type
public:
    /**
    * @brief constructor
    */
	Region()
    :
    patchTabPtr_(NULL),
    scalarFieldTabPtr_(NULL),
    labelFieldTabPtr_(NULL)
    {}

    /**
    * @brief deconstructor
    */
	~Region();

    /**
    * @brief get the internal mesh
    */
	Mesh& getMesh(){return this->mesh_;};

    /**
    * @brief get the boundary mesh and condition
    */
	Boundary& getBoundary(){return this->boundary_;};

    /**
    * @brief initialization before load balance
    */
	void initBeforeBalance(Array<char*> meshFile);

    /**
    * @brief initialization after load balance
    */
	void initAfterBalance();

	/**
    * @brief      write mesh to CGNS file
    * @param[in]       meshFile  The mesh file
    */
    void writeMesh(char* meshFile);

    /**
    * @brief      write field to CGNS file
    * @param[in]  resFile CGNS file storing fields
    * @param[in]  fieldName field name
    * @param[in]  fieldType field type
    * @tparam T label, scalar
    */
    template<typename T>
    void writeField(const char* resFile, const char* fieldName,
        const char* fieldType);

	// guhf
    /**
    * @brief create the communication topology
    * @param[in]  faceCells topology between faces and cells (owner and neighbor)
    * @param[in]  cellNum the count of elements owned by this process
    */
    void createInterFaces
    (
        Array<Array<label> >& faceCells,
        label cellNum
    );

    /**
     * @brief      Initializes the fields.
     *
     * @param[in]   fieldName The field name
     */
    void initFields(Word fieldName);

    /**
     * @brief      update the fields
     *
     * @param[in]  fieldName  The field name
     */
    void updateFields(Word fieldName);

    /**
     * @brief Gets the field from field table.
     * @param[in]  fieldType field setType: face, node, ...
     * @param[in]  fieldName field name
     * @tparam T label, scalar
     * @return The field.
     */
    template<typename T>
    Field<T>& getField(const Word fieldType, const Word fieldName);

    /**
     * @brief      Adds a field to region.
     * @param[in]  setType  field setType: face, node, ...
     * @param[in]  name  field name
     * @param      f  field pointer
     * @tparam     T          label, scalar
     */
    template<typename T>
    void addField(Word setType, Word name, Field<T>* f);

    /**
     * @brief       delete a named field
     * @param[in]  Word  field setType: face, node, ...
     * @param[in]  Word  field name
     */
    void deleteField(Word, Word);
};

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
    Word setType,
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
    // label nCells = 64;
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
    {
        // printf("create new solution info, solName: %s, location: %d\n", solName, location);
   		if(cg_sol_write(iFile, iBase, iZone, solName, location, &S))
            Terminate("writeSolution", cg_get_error());
    }
	// cg_field_write(iFile, iBase, iZone, S, dataType, fieldName, dataPtr, &Fs);
    if(cgp_field_write(iFile, iBase, iZone, S, dataType, fieldName, &Fs))
        Terminate("writeSolutionInfo", cg_get_error());
    // printf("%d, %d, %d\n", nSols, S, Fs);

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
    // cgsize_t start[1];
    // start[0] = cellStartId[rank]+1;
    // // for(int k=0;k<1;k++) start[k] = cellStartId[rank]+1;
    // cgsize_t end[1];

    // int tmp = (int)cellStartId[rank+1];
    // if(tmp!=64) printf("%d\n", cellStartId[rank+1]);
    // end[0] = cellStartId[rank+1];

    // char* test = (char*)(&ndim);
    // for (int i = 0; i < 8; ++i)
    // {
    //     printf("%c, ", test[i]);
    // }
    // // for(int k=0;k<1;k++) end[k] = 64;
    // // cgsize_t end = start+1;
    // end[0] = (cgsize_t)test[0];
    // end[0] = cellStartId[rank+1];
    // printf("rank: %d, %ld, %ld\n", rank, start[0], end[0]);
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

} //- end namespace HSF
#endif //- REGION_HPP
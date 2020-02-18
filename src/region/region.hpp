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
    template<typename T>
    void initField(const Word, const Word);

    /**
     * @brief      update the fields
     *
     * @param[in]  fieldName  The field name
     */
    template<typename T>
    void updateField(const Word, const Word);

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
     * @param[in]  name  field name
     * @param      f  field pointer
     * @tparam     T          label, scalar
     */
    template<typename T>
    void addField(Word name, Field<T>* f);

    /**
     * @brief       delete a named field
     * @param[in]  Word  field setType: face, node, ...
     * @param[in]  Word  field name
     */
    template<typename T>
    void deleteField(Word, Word);
};

#include "regionI.hpp"

} //- end namespace HSF
#endif //- REGION_HPP
/**
* @file: meshInfo.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-09 14:12:42
* @last Modified by:   lenovo
* @last Modified time: 2019-11-28 16:33:50
*/
#ifndef MESHINFO_HPP
#define MESHINFO_HPP

#include <typeinfo>
#include "utilities.hpp"
#include "field.hpp"
#include "mesh.hpp"

namespace HSF
{
/**
* @brief Geometry information of mesh, storaged as class Field
*/
class MeshInfo
{
private:
	/**
	* @brief mapping between name and label fields
	*/	
	Table<Word, Table<Word, labelField*>*>* labelFieldTabPtr_;

	/**
	* @brief mapping between name and scalar fields
	*/
	Table<Word, Table<Word, scalarField*>*>* scalarFieldTabPtr_;

	Word setType_; ///< data set type
	/**
	* @brief compute face information and register field
	*        finished: area, normal vector, center
	*/
	void addFaceInfo(Mesh& mesh);
	/**
	* @brief compute cell information and register field
	*        finished: vol, center
	*/
	void addCellInfo(Mesh& mesh);
    /**
     * @brief      Adds a field to region.
     * @param[in]  Word  field setType: face, node, ...
     * @param[in]  Word  field name
     * @param      Field<T>*  field pointer
     * @tparam     T          label, scalar
     */
    template<typename T>
    void addField(Word setType, Word name, Field<T>* f);
    /**
     * @brief Gets the field from field table.
     * @param[in]  fieldType field setType: face, node, ...
     * @param[in]  fieldName field name
     * @tparam T label, scalar
     * @return The field.
     */
    template<typename T>
    Field<T>& getField(const Word fieldType, const Word fieldName);
public:
	/**
	* @brief default constructor
	*/
	MeshInfo();
	/**
	* @brief construct from mesh
	* @param mesh Topology
	*/
	MeshInfo(Mesh& mesh)
	{
		// info_.insert({string("area"), Area});
	};
	/**
	* @brief deconstructor
	*/
	~MeshInfo();
	/**
	* @brief initialization
	*/
	void init(Mesh& mesh);
	/**
	* @brief get the face area field
	* @param[in] setType field set type
	* @param[in] name field name
	* @return face area field
	*/
	template<typename T>
	T* getMeshInfo(Word setType, Word name);
};

template<typename T>
void MeshInfo::addField
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

    #undef FIELDTABPTR
}

template<typename T>
Field<T>& MeshInfo::getField(const Word fieldType, const Word fieldName)
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

} // end namespace HSF

#endif
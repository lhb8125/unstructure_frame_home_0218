/**
* @file: blockTopology.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2020-01-06 15:57:25
* @last Modified by:   lenovo
* @last Modified time: 2020-01-10 17:02:23
*/

#ifndef BLOCKTOPOLOGY_HPP
#define BLOCKTOPOLOGY_HPP
#include <string.h>
#include "utilities.hpp"
#include "section.hpp"
#include "cgnslib.h"
#include "topology.hpp"

namespace HSF
{

/**
* @brief Topology information of mesh
*/
class BlockTopology
{
private:
	ArrayArray<label> face2Node_; ///< Connectivity between faces and nodes (finish)
	
	Array<Array<label> > face2NodeBnd_; ///< Connectivity between faces and nodes at the boundary (finish)
	
	ArrayArray<label> face2Cell_; ///< Connectivity between faces and cells (finish)
	
	Array<Array<label> > face2CellPatch_; ///< Connectivity between faces and cells at the process boundary (finish)
	
	Array<Array<label> > face2CellBnd_; ///< Connectivity between faces and cells at the boundary (finish)
	
	ArrayArray<label> cell2Cell_; ///< Connectivity between cells and cells (finish)
	
	ArrayArray<label> cell2Node_; ///< Connectivity between cells and nodes (finish)
	
	ArrayArray<label> cell2Face_; ///< Connectivity between cells and faces (finish)
	
	ArrayArray<label> cell2Edge_; ///< Connectivity between cells and edges
	
	Array<label> cellType_; ///< type of cells
	
	Array<label> faceType_; ///< type of faces

	Array<label> BCType_; ///< type of boundary condition

	Array<label> cellBlockStartIdx_; ///< the start index of each block in the overall cell topology

	Array<label> faceBlockStartIdx_; ///< the start index of each block in the overall face topology

	Array<label> reorderCellTopo(Topology& topo); ///< reorder the cell-related topology

	Array<label> reorderFaceTopo(Topology& topo); ///< reorder the cell-related topology

	void reorderOtherTopo(const Array<label>& map,
		const ArrayArray<label>& oldTopo, ArrayArray<label>& newTopo);

public:
	/**
	* @brief default constructor
	*/
	BlockTopology();

	void constructBlockTopology(Topology& topo);

	/**
	* @brief default deconstructor
	*/
	~BlockTopology();

	/**
	* @brief get the count of cell types over the region
	*/
	Array<label> getCellBlockStartIdx(){return this->cellBlockStartIdx_;};

	/**
	* @brief get the count of face types over the region
	*/
	Array<label> getFaceBlockStartIdx(){return this->faceBlockStartIdx_;};
	/**
	* @brief set the count of face types over the region
	*/
	void setFaceBlockStartIdx(Array<label> faceBlockStartIdx)
	{
		this->faceBlockStartIdx_ = faceBlockStartIdx;
	};

	/**
	* @brief get the cell2node topology
	*/
	ArrayArray<label> getCell2Node(){return this->cell2Node_;};

	/**
	* @brief get the cell2cell topology
	*/
	ArrayArray<label> getCell2Cell(){return this->cell2Cell_;};

	/**
	* @brief get the cell2face topology
	*/
	ArrayArray<label> getCell2Face(){return this->cell2Face_;};

	/**
	* @brief get the face2node topology
	*/
	ArrayArray<label> getFace2Node(){return this->face2Node_;};
	/**
	* @brief set the face2node topology
	*/
	void setFace2Node(Array<Array<label> > face2Node)
	{
		transformArray(face2Node, this->face2Node_);
	};

	/**
	* @brief get the face2Cell topology
	*/
	ArrayArray<label> getFace2Cell(){return this->face2Cell_;};
	/**
	* @brief set the face2Cell topology
	*/
	void setFace2Cell(Array<Array<label> > face2Cell)
	{
		transformArray(face2Cell, this->face2Cell_);
	};

	/**
	* @brief get the cell block type
	*/
	Array<label> getCellType(){return this->cellType_;};

	/**
	* @brief get the face block type
	*/
	Array<label> getFaceType(){return this->faceType_;};
	/**
	* @brief set the face block type
	*/
	void setFaceType(Array<label> faceType)
	{
		this->faceType_ = faceType;
	};

	/**
	* @brief get the boundary condition block type
	*/
	Array<label> getBCType(){return this->BCType_;};
	/**
	* @brief set the boundary condition block type
	*/
	void setBCType(Array<label> BCType)
	{
		this->BCType_ = BCType;
	};



};

}

#endif
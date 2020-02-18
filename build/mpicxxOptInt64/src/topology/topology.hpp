/**
* @file: topology.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-09 15:08:19
* @last Modified by:   lenovo
* @last Modified time: 2020-01-06 16:36:38
*/
#ifndef TOPOLOGY_HPP
#define TOPOLOGY_HPP
#include <string.h>
#include "utilities.hpp"
#include "section.hpp"
#include "cgnslib.h"

namespace HSF
{

/**
* @brief Topology information of mesh
*/
class Topology
{
private:
	
	label nodeNum_; ///< count of nodes;
	
	label cellNum_; ///< count of cells;
	
	label edgeNum_; ///< count of edges;
	
	label faceNum_; ///< count of faces;
	
	label faceNum_b_; ///< count of boundary faces;
	
	label faceNum_i_; ///< count of internal faces;
	
	label cellStartId_; ///< global start index of elements of this processor
	
	ArrayArray<label> node2Node_; ///< Connectivity between nodes and nodes
	
	ArrayArray<label> node2Edge_; ///< Connectivity between nodes and edges
	
	ArrayArray<label> edge2Node_; ///< Connectivity between edges and nodes (finish)
	
	ArrayArray<label> edge2Cell_; ///< Connectivity between edges and cells (finish)
	
	ArrayArray<label> face2Node_; ///< Connectivity between faces and nodes (finish)
	
	Array<Array<label> > face2NodeBnd_; ///< Connectivity between faces and nodes at the boundary (finish)
	
	ArrayArray<label> face2Cell_; ///< Connectivity between faces and cells (finish)
	
	Array<Array<label> > face2CellPatch_; ///< Connectivity between faces and cells at the process boundary (finish)
	
	Array<Array<label> > face2CellBnd_; ///< Connectivity between faces and cells at the boundary (finish)
	
	ArrayArray<label> face2Edge_; ///< Connectivity between faces and edges
	
	ArrayArray<label> cell2Cell_; ///< Connectivity between cells and cells (finish)
	
	ArrayArray<label> cell2Node_; ///< Connectivity between cells and nodes (finish)
	
	ArrayArray<label> cell2Face_; ///< Connectivity between cells and faces (finish)
	
	ArrayArray<label> cell2Edge_; ///< Connectivity between cells and edges (finish)
	
	Array<label> cellType_; ///< type of cells
	
	Array<label> faceType_; ///< type of faces
	// /// reorder the face2Node topology to seperate the boundary faces and internal faces
	// label reorderFace2Node(Array<Array<label> >& face2NodeTmp,
	// 	Array<Array<label> >& face2NodeBndTmp);
	/**
	* @brief generate the face2Cell topology at the boundary face
	* @param[in] face2NodeInn face-to-node topology, sorted for the first node index at least
	* @param[in] face2NodeBnd face-to-node boundary topology, sorted for the first node index at least
	* @param[in] face2CellInn face-to-cell topology, one-to-one corresponding to the face above
	* @param[in] face2CellBnd face-to-cell boundary topology, one-to-one corresponding to the face above
	*/
	void setPatchInfo(Array<Array<label> > face2NodeInn,
		Array<Array<label> > face2NodeBnd, Array<Array<label> > face2CellInn,
		Array<Array<label> > face2CellBnd);
	/**
	* @brief generate the edge-based topology
	*/
	void genEdgeTopo();
public:
	/**
	* @brief default constructor
	*/
	Topology();
	/**
	* @brief Construct from section information of CGNS file
	* @param secs sections storing connectivity between cells and nodes
	*/
	void constructTopology(Array<Section>& secs);
	/**
	* @brief Construct from the connectivity after load balance
	*/
	void constructTopology();
	/**
	* @brief deconstructor
	*/
	~Topology();
	/**
	* @brief get the count of nodes
	* @return the count of nodes
	*/
	label getNodesNum(){return this->nodeNum_;};
	/**
	* @brief get the count of cells
	*/
	label getCellsNum(){return this->cellNum_;};
	/**
	* @brief get the count of faces
	*/
	label getFacesNum(){return this->faceNum_;};
	/**
	* @brief get the count of internal faces
	*/
	label getInnFacesNum(){return this->faceNum_i_;};
	/**
	* @brief get the count of boundary faces
	*/
	label getBndFacesNum(){return this->faceNum_b_;};
	/**
	* @brief get the count of edges
	*/
	label getEdgesNum(){return this->edgeNum_;};

	/**
	* @brief set the topology between cell and node
	*/
	void setCell2Node(ArrayArray<label>& cell2Node)
	{
		this->cell2Node_ = cell2Node;
		this->cellNum_ = cell2Node.size();
		// printf("%d, %d\n", cell2Node_.num, cell2Node.size());
	};

	/**
	* @brief set the topology between cell and node
	*/
	void setCell2Node(Array<Array<label> >& cell2Node)
	{
		transformArray(cell2Node, this->cell2Node_);
		this->cellNum_  = cell2Node.size();
	};

	/**
	* @brief set the topology between face and node
	*/
	void setFace2Node(Array<Array<label> >& face2Node)
	{
		transformArray(face2Node, this->face2Node_);
		this->faceNum_ = face2Node.size();
	};

	/**
	* @brief set the topology between cell and face
	*/
	void setFace2Cell(Array<Array<label> >& face2Cell)
	{
		transformArray(face2Cell, this->face2Cell_);
		this->faceNum_ = face2Cell.size();
	};

	/**
	* @brief set the cell type 
	*/
	void setCellType(Array<label>& cellType)
	{
		this->cellType_.clear();
		this->cellType_.assign(cellType.begin(), cellType.end());
	};

	/**
	* @brief set the start index of cells in this processor
	*/
	void setCellStartId(label cellStartId)
	{
		this->cellStartId_ = cellStartId;
	};


	/**
	* @brief get the topology between face and node on the boundary
	*/
	const Array<Array<label> >& getFace2NodeBnd() {return this->face2NodeBnd_;};

	/**
	* @brief get the topology between face and cell on the boundary
	*/
	const Array<Array<label> >& getFace2CellBnd() {return this->face2CellBnd_;};

	/**
	* @brief get the cell type
	*/
	const Array<label>& getCellType() { return this->cellType_;};

	/**
	* @brief get the topology between face and cell on the processor boundary
	*/
	const Array<Array<label> > getFace2CellPatch() {return this->face2CellPatch_;};

	/**
	* @brief get the start index of cells in this processor
	*/
	label getCellStartId() {return this->cellStartId_;};

	// 拓扑关系输出接口
	/**
	* @brief get the topology between cells and nodes
	*/
	const ArrayArray<label> getCell2Node() {return this->cell2Node_;};
	/**
	* @brief get the topology between cells and faces
	*/
	const ArrayArray<label> getCell2Face() {return this->cell2Face_;};
	/**
	* @brief get the topology between cells and cells
	*/
	const ArrayArray<label> getCell2Cell() {return this->cell2Cell_;};
	/**
	* @brief get the topology between faces and nodes
	*/
	const ArrayArray<label> getFace2Node() {return this->face2Node_;};
	/**
	* @brief get the topology between faces and cells
	*/
	const ArrayArray<label> getFace2Cell() {return this->face2Cell_;};
};

} // end namespace HSF

#endif
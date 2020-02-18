/**
* @file: loadBalancer.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 14:25:14
* @last Modified by:   lenovo
* @last Modified time: 2019-11-27 15:32:31
*/
#ifndef LOADBALANCER_HPP
#define LOADBALANCER_HPP

#include <math.h>
#include <assert.h>
#include <limits.h>
#include "utilities.hpp"
#include "region.hpp"

namespace HSF
{

/**
* @brief load balancer for regions
*/
class LoadBalancer
{
private:
	Array<scalar> s_; ///< measurement of regions;
	
	ArrayArray<label> nei_; ///< neighbors of regions;
	
	ArrayArray<label> procId_; ///< results of assigned processes for all regions
	
	ArrayArray<scalar> procLoad_; ///< proportion of assigned processes for all regions
	
	scalar* procLoadSum_; ///< measurement for all processes
	
	label procNum_; ///< count of processes
	
	label* cellStartId_; ///< start index of cells in every processors
	/**
	* @brief find the process with the least measurement load
	* @return procId Id of processes for all regions
	*/
	label findMinProc();
	/*
	* @brief find the region with the maximum measurement load
	* @param[in] s the measurement load of all regions
	* @return the index of region with maximum measurement
	*/
	label findMaxRegion(const Array<scalar> s);
	/*
	* @brief distribute the cells to other processors according to 
	         the result of PARMETIS
	* @param[in] cell2Node the topology between elements and nodes
	* @param[in] parts the owner of elements according to results of ParMETIS
	* @return the new topology between elements and nodes
	*/
	ArrayArray<label> distributeCellsToProcs(const ArrayArray<label>& cell2Node,
		const label* parts);
	/*
	* @brief distribute the cell infomation (type) to other processors
	* @param[in] cell2Node the elements information including cell type.
	* @param[in] parts the owner of elements according to results of ParMETIS
	* @return the new elements information
	*/
	Array<label> distributeCellInfoToProcs(Array<label>& cellInfo, label* parts);
public:
	/**
	* @brief default constructor
	*/
	LoadBalancer();
	/**
	* @brief deconstructor
	*/
	~LoadBalancer();
	/**
	* @brief Second Level load balance
	* @param[in] s measurement for all regions
	* @param[in] nei connectivity among all regions
	* @param[out] procNum count of processes
	*/
	void LoadBalancer_2(const Array<scalar> s, const ArrayArray<label> nei,
		label procNum);
	/**
	* @brief communicate the region based on the results of loadBalancer_2
	*/
	// void exchangeRegion(Region& regs);
	/**
	* @brief Third Level load balance with parmetis
	* @param[in] reg the collection of regions owned by this processor
	*/
	void LoadBalancer_3(Array<Region>& reg);
	/**
	* @brief collect the neighbor cell index through mpi
	* @param[in] bndFaceList the topology between faces and nodes on the boundary
	* @param[in] face2NodeArr the topology between faces and nodes (sorted)
	* @param[out] face2CellArr the topology between faces and elements,
	         the internal faces are marked as -1.
	*/
	static Array<label> collectNeighborCell(ArrayArray<label>& bndFaceList,
		Array<Array<label> >& face2NodeArr, Array<label>& face2CellArr);

	/**
	* @brief expired function
	*/
	ArrayArray<label>& getProcId() {return this->procId_;}

	/**
	* @brief expired function
	*/
	ArrayArray<scalar>& getProcLoad() {return this->procLoad_;}

	/**
	* @brief expired function
	*/
	scalar* getProcLoadSum() {return this->procLoadSum_;}
};

} // end namespace HSF

#endif
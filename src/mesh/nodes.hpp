/**
* @file: nodes.hpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-09-09 15:08:19
* @last Modified by:   lenovo
* @last Modified time: 2019-12-18 09:49:00
*/
#ifndef NODES_HPP
#define NODES_HPP
#include "utilities.hpp"

namespace HSF
{

/**
* @brief Coordinates of nodes
*/
class Nodes
{
private:
	
	ArrayArray<scalar> xyz_; ///< AoS layout of coordinates
	
	Array<scalar> x_; ///< Coordinate X
	
	Array<scalar> y_; ///< Coordinate Y
	
	Array<scalar> z_; ///< Coordinate Z
	
	label start_; ///< the global start index of nodes
	
	label end_; ///< the global end index of nodes

public:
	RefCounted *refCount_; ///< the count of reference pointers
	/**
	* @brief default constructor
	*/
	Nodes();
	/**
	* @brief constructor
	* @param[in] xyz AoS layout of coordinates
	*/
	Nodes(ArrayArray<scalar>& xyz);
	/** 
	* @brief constructor
	* @param[in] x Coordinate X
	* @param[in] y Coordinate Y
	* @param[in] z Coordinate Z
	*/
	Nodes(Array<scalar>& x, Array<scalar>& y, Array<scalar>& z);
	/** 
	* @brief constructor
	* @param[in] x Coordinate X
	* @param[in] y Coordinate Y
	* @param[in] z Coordinate Z
	*/
	Nodes(scalar* x, scalar* y, scalar* z, label num);
	/**
	* @brief copy constructor
	*/
	Nodes(const Nodes& node);
	/**
	* @brief deconstructor
	*/
	~Nodes();
	/**
	* @brief get the count of nodes
	* @return the count of nodes
	*/
	label size();
	/**
	* @brief copy function
	*/
	void copy(Nodes* nodes);
	/**
	* @brief add function
	*/
	void add(Nodes* nodes);

	/**
	* @brief get X coordinates
	*/
	const Array<scalar>& getX();

	/**
	* @brief get Y coordinates
	*/
	const Array<scalar>& getY();

	/**
	* @brief get Z coordinates
	*/
	const Array<scalar>& getZ();

	/**
	* @brief get XYZ coordinates
	*/
	const ArrayArray<scalar>& getXYZ();

	/**
	* @brief get XYZ coordinates with index
	* @param[in] idx node index
	*/
	const scalar* getXYZ(label idx);

	/**
	* @brief get the global start index of nodes
	* @return global start index of nodes
	*/
	// label getStart();

	/**
	* @brief get the global end index of nodes
	* @return global end index of nodes
	*/
	// label getEnd();

	/**
	* @brief set the global start index of nodes
	* @param[in] start global start index of nodes
	*/
	// void setStart(const label start);

	/**
	* @brief set the global end index of nodes
	* @param[in] end global end index of nodes
	*/
	// void setEnd(const label end);
};

} // end namespace HSF

#endif
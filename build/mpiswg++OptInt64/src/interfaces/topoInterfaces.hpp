/**
* @file: interfaces.hpp
* @author: Liu Hongbin
* @brief: fortran function interfaces
* @date:   2019-11-11 10:56:28
* @last Modified by:   lenovo
* @last Modified time: 2019-12-18 08:31:53
*/
#ifndef TOPOINTERFACES_HPP
#define TOPOINTERFACES_HPP

#include "interface.hpp"
#include "utilities.hpp"
#include "mesh.hpp"

extern "C"
{
/**
* @brief fortran function interfaces
*/
/*****************************************************************************/
/**
* @brief 初始化程序，包括参数、网格读入，负载均衡，拓扑与通信对创建等
* @param[in]  configFile 参数文件
*/
void init_(char* configFile);

void clear_();

/**
* @brief 获取网格单元数目
* @param[out]  eleNum 网格单元数目
*/
void get_elements_num_(label* eleNum);

/**
* @brief 获取所有网格面数目
* @param[out]  faceNum 所有网格面数目
*/
void get_faces_num_(label* faceNum);

/**
* @brief 获取所有内部网格面数目
* @param[out]  innFaceNum 所有内部网格面数目
*/
void get_inner_faces_num_(label* innFaceNum);

/**
* @brief 获取所有边界网格面数目
* @param[out]  bndFaceNum 所有边界网格面数目
*/
void get_bnd_faces_num_(label* bndFaceNum);

/**
* @brief 获取格点node数目
* @param[out]  nodeNum 格点node数目
*/
void get_nodes_num_(label* nodeNum);

/******************************************************************************
/**
* @brief 获取网格单元与网格面的拓扑关系
* @param[out]  pos AoS格式中结构体的起始位置
*/
void get_ele_2_face_pos_(label* pos);

/**
* @brief 获取网格单元与网格面的拓扑关系
* @param[out]  ele2Face 网格单元与网格面的映射关系，AoS格式存储 
*/
void get_ele_2_face_(label* ele2Face);

/**
* @brief 获取网格单元之间的拓扑关系，只存储内部单元
* @param[out]  pos AoS格式中结构体的起始位置
*/
void get_ele_2_ele_pos_(label* pos);

/**
* @brief 获取网格单元之间的拓扑关系，只存储内部单元
* @param[out]  ele2Ele 网格单元之间的映射关系，AoS格式存储 
*/
void get_ele_2_ele_(label* ele2Ele);

/**
* @brief 获取网格单元与格点的拓扑关系
* @param[out]  pos AoS格式中结构体的起始位置
*/
void get_ele_2_node_pos_(label* pos);

/**
* @brief 获取网格单元与格点的拓扑关系
* @param[out]  ele2Node 网格单元与格点的映射关系，AoS格式存储 
*/
void get_ele_2_node_(label* ele2Node);

/**
* @brief 获取内部网格面与格点的拓扑关系
* @param[out]  pos AoS格式中结构体的起始位置
*/
void get_inn_face_2_node_pos_(label* pos);

/**
* @brief 获取内部网格面与格点的拓扑关系
* @param[out]  innFace2Node 内部网格面与格点的映射关系，AoS格式存储 
*/
void get_inn_face_2_node_(label* innFace2Node);

/**
* @brief 获取边界网格面与格点的拓扑关系
* @param[out]  pos AoS格式中结构体的起始位置
*/
void get_bnd_face_2_node_pos_(label* pos);

/**
* @brief 获取边界网格面与格点的拓扑关系
* @param[out]  bndFace2Node 边界网格面与格点的映射关系，AoS格式存储 
*/
void get_bnd_face_2_node_(label* bndFace2Node);

/**
* @brief 获取内部网格面与网格单元的拓扑关系
* @param[out]  pos AoS格式中结构体的起始位置
*/
void get_inn_face_2_ele_pos_(label* pos);

/**
* @brief 获取内部网格面与网格单元的拓扑关系
* @param[out]  innFace2Ele 内部网格面与网格单元的映射关系，AoS格式存储
*/
void get_inn_face_2_ele_(label* innFace2Ele);

/**
* @brief 获取边界网格面与网格单元的拓扑关系
* @param[out]  pos AoS格式中结构体的起始位置
*/
void get_bnd_face_2_ele_pos_(label* pos);

/**
* @brief 获取边界网格面与网格单元的拓扑关系
* @param[out]  bndFace2Ele 边界网格面与网格单元的映射关系，AoS格式存储 
*/
void get_bnd_face_2_ele_(label* bndFace2Ele);

/*****************************************************************************/
/**
* @brief 获取网格单元类型，以整型方式存储
* @param[out]  eleType 网格单元类型
*/
void get_ele_type_(label* eleType);

/**
* @brief 获取格点坐标，以AoS方式存储
* @param[out]  coords 格点坐标
*/
void get_coords_(scalar* coords);

/*****************************************************************************/

void test_c_();
}

#endif

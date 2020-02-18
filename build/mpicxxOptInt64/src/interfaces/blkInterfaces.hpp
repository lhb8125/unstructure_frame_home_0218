/**
* @file: blkInterfaces.hpp
* @author: Liu Hongbin
* @brief: block topology interfaces
* @date:   2019-11-11 10:56:28
* @last Modified by:   lenovo
* @last Modified time: 2019-12-18 08:31:53
*/
#ifndef BLKINTERFACES_HPP
#define BLKINTERFACES_HPP

#include "interface.hpp"
#include "utilities.hpp"
#include "mesh.hpp"

extern "C"
{

void get_ele_block_num_(label *ele_blk_num);

void get_ele_blk_pos_(label *ele_blk_pos);

void get_ele_blk_type_(label *ele_blk_type);

void get_ele_2_ele_blk_(label *ele_2_ele);

void get_ele_2_face_blk_(label *ele_2_face);

void get_ele_2_node_blk_(label *ele_2_node);



void get_face_block_num_(label *face_blk_num);

void get_face_blk_pos_(label *face_blk_pos);

void get_face_blk_type_(label *face_blk_type);

void get_face_2_node_blk_(label *face_2_node);

}

#endif
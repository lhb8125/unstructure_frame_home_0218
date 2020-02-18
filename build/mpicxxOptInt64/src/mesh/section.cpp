/**
* @file: 
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-14 16:22:22
* @last Modified by:   lenovo
* @last Modified time: 2020-01-10 16:26:06
*/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "section.hpp"

namespace HSF
{

/*
* @brief counts of nodes for each element type
*/
bool Section::compareEleType(const label secType, const label meshType_)
{
	bool ltmp = secType==TRI_3 || secType==TRI_6 || secType==TRI_9;
	ltmp = ltmp || secType==QUAD_4 || secType==QUAD_8 || secType==QUAD_9;
	if(ltmp && meshType_==Boco) {  return true;}
	else if(!ltmp && meshType_==Inner) {  return true;}
	else return false;
}

/*
* @brief counts of faces for each element type
*/
label Section::nodesNumForEle(const label eleType)
{
	switch(eleType)
	{
		case TRI_3: return 3;
		case TRI_6: return 6;
		case QUAD_4: return 4;
		case QUAD_8: return 8;
		case QUAD_9: return 9;
		case TETRA_4: return 4;
		case PYRA_5: return 5;
		case PENTA_6: return 6;
		case HEXA_8: return 8;
		case HEXA_27: return 27;
		default:
			Terminate("find nodes count for Elements", "element type is not supported");
	}
}

label Section::facesNumForEle(const label eleType)
{
	switch(eleType)
	{
		case TRI_3: return 0;
		case TRI_6: return 0;
		case QUAD_4: return 0;
		case QUAD_8: return 0;
		case TETRA_4: return 4;
		case PYRA_5: return 5;
		case PENTA_6: return 5;
		case HEXA_8: return 6;
		case HEXA_27: return 6;
		default:
			Terminate("find faces count for Elements", "element type is not supported");
	}
}

label Section::edgesNumForEle(const label eleType)
{
	switch(eleType)
	{
		case TRI_3: return 3;
		case TRI_6: return 3;
		case QUAD_4: return 4;
		case QUAD_8: return 4;
		case TETRA_4: return 6;
		case PYRA_5: return 8;
		case PENTA_6: return 9;
		case HEXA_8: return 12;
		case HEXA_27: return 12;
		default:
			Terminate("find faces count for Elements", "element type is not supported");
	}
}

/*
* @brief whether the section belongs to the entity through the elements type 
*/
Array<label> Section::faceNodesForEle(
	label* conn, const label eleType, const label idx)
{
	Array<label> tmp;
	// //printf("%d, %d\n", eleType, TETRA_4);
	if(eleType==TETRA_4)
	{
		/// the first face
		if(idx==0)
		{
			tmp.push_back(conn[0]);
			tmp.push_back(conn[2]);
			tmp.push_back(conn[1]);
		} else if(idx==1)
		/// the second face
		{
			tmp.push_back(conn[0]);
			tmp.push_back(conn[1]);
			tmp.push_back(conn[3]);
		} else if(idx==2)
		/// the third face
		{
			tmp.push_back(conn[1]);
			tmp.push_back(conn[2]);
			tmp.push_back(conn[3]);
		} else if(idx==3)
		/// the fourth face
		{
			tmp.push_back(conn[2]);
			tmp.push_back(conn[0]);
			tmp.push_back(conn[3]);
		}
	}else if(eleType==PYRA_5)
	{
		/// the first face
		if(idx==0)
		{
			tmp.push_back(conn[0]);
			tmp.push_back(conn[3]);
			tmp.push_back(conn[2]);
			tmp.push_back(conn[1]);
		} else if(idx==1)
		/// the second face
		{
			tmp.push_back(conn[0]);
			tmp.push_back(conn[1]);
			tmp.push_back(conn[4]);
		} else if(idx==2)
		/// the third face
		{
			tmp.push_back(conn[1]);
			tmp.push_back(conn[2]);
			tmp.push_back(conn[4]);
		} else if(idx==3)
		/// the fourth face
		{
			tmp.push_back(conn[2]);
			tmp.push_back(conn[3]);
			tmp.push_back(conn[4]);
		} else if(idx==4)
		{
			tmp.push_back(conn[3]);
			tmp.push_back(conn[0]);
			tmp.push_back(conn[4]);
		}
	} else if(eleType==HEXA_8)
	{
		/// the first face
		if(idx==0)
		{
			tmp.push_back(conn[0]);
			tmp.push_back(conn[3]);
			tmp.push_back(conn[2]);
			tmp.push_back(conn[1]);
		} else if(idx==1)
		/// the second face
		{
			tmp.push_back(conn[0]);
			tmp.push_back(conn[1]);
			tmp.push_back(conn[5]);
			tmp.push_back(conn[4]);
		} else if(idx==2)
		/// the third face
		{
			tmp.push_back(conn[1]);
			tmp.push_back(conn[2]);
			tmp.push_back(conn[6]);
			tmp.push_back(conn[5]);
		} else if(idx==3)
		/// the fourth face
		{
			tmp.push_back(conn[2]);
			tmp.push_back(conn[3]);
			tmp.push_back(conn[7]);
			tmp.push_back(conn[6]);
		} else if(idx==4)
		/// the fourth face
		{
			tmp.push_back(conn[0]);
			tmp.push_back(conn[4]);
			tmp.push_back(conn[7]);
			tmp.push_back(conn[3]);
		} else if(idx==5)
		/// the fourth face
		{
			tmp.push_back(conn[4]);
			tmp.push_back(conn[5]);
			tmp.push_back(conn[6]);
			tmp.push_back(conn[7]);
		}		
	} else if(eleType==HEXA_27)
	{
		/// the first face
		if(idx==0)
		{
			tmp.push_back(conn[0]); tmp.push_back(conn[3]);
			tmp.push_back(conn[2]);	tmp.push_back(conn[1]);
			tmp.push_back(conn[11]); tmp.push_back(conn[10]);
			tmp.push_back(conn[9]);	tmp.push_back(conn[8]);	
			tmp.push_back(conn[20]);
		} else if(idx==1)
		/// the second face
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[1]);
			tmp.push_back(conn[5]);	tmp.push_back(conn[4]);
			tmp.push_back(conn[8]); tmp.push_back(conn[13]);
			tmp.push_back(conn[16]); tmp.push_back(conn[12]);
			tmp.push_back(conn[21]);
		} else if(idx==2)
		/// the third face
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[2]);
			tmp.push_back(conn[6]);	tmp.push_back(conn[5]);
			tmp.push_back(conn[9]); tmp.push_back(conn[14]);
			tmp.push_back(conn[17]); tmp.push_back(conn[13]);
			tmp.push_back(conn[22]);
		} else if(idx==3)
		/// the fourth face
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[3]);
			tmp.push_back(conn[7]);	tmp.push_back(conn[6]);
			tmp.push_back(conn[10]); tmp.push_back(conn[15]);
			tmp.push_back(conn[18]); tmp.push_back(conn[14]);
			tmp.push_back(conn[23]);
		} else if(idx==4)
		/// the fourth face
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[4]);
			tmp.push_back(conn[7]);	tmp.push_back(conn[3]);
			tmp.push_back(conn[12]); tmp.push_back(conn[19]);
			tmp.push_back(conn[15]); tmp.push_back(conn[11]);
			tmp.push_back(conn[24]);
		} else if(idx==5)
		/// the fourth face
		{
			tmp.push_back(conn[4]);	tmp.push_back(conn[5]);
			tmp.push_back(conn[6]);	tmp.push_back(conn[7]);
			tmp.push_back(conn[16]); tmp.push_back(conn[17]);
			tmp.push_back(conn[18]); tmp.push_back(conn[19]);
			tmp.push_back(conn[25]);
		}		
	} else
	{
		Terminate("find face nodes for Elements", "The element type is not supported");
	}
	return tmp;
}

Array<label> Section::edgeNodesForEle(
	label* conn, const label eleType, const label idx)
{
	Array<label> tmp;
	// //printf("%d, %d\n", eleType, TETRA_4);
	if(eleType==TETRA_4)
	{
		if(idx==0)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[1]);
		} else if(idx==1)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[2]);
		} else if(idx==2)
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[0]);
		} else if(idx==3)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[3]);
		} else if(idx==4)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[3]);
		} else if(idx==5)
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[3]);
		}
	} else if(eleType==PYRA_5)
	{
		if(idx==0)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[1]);
		} else if(idx==1)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[2]);
		} else if(idx==2)
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[3]);
		} else if(idx==3)
		{
			tmp.push_back(conn[3]);	tmp.push_back(conn[0]);
		} else if(idx==4)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[4]);
		} else if(idx==5)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[4]);
		} else if(idx==6)
		{
			tmp.push_back(conn[2]); tmp.push_back(conn[4]);
		} else if(idx==7)
		{
			tmp.push_back(conn[3]); tmp.push_back(conn[4]);
		}
	} else if(eleType==HEXA_8)
	{
		if(idx==0)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[1]);
		} else if(idx==1)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[2]);
		} else if(idx==2)
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[3]);
		} else if(idx==3)
		{
			tmp.push_back(conn[3]);	tmp.push_back(conn[0]);
		} else if(idx==4)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[4]);
		} else if(idx==5)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[5]);
		} else if(idx==6)
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[6]);
		} else if(idx==7)
		{
			tmp.push_back(conn[3]);	tmp.push_back(conn[7]);
		} else if(idx==8)
		{
			tmp.push_back(conn[4]);	tmp.push_back(conn[5]);
		} else if(idx==9)
		{
			tmp.push_back(conn[5]);	tmp.push_back(conn[6]);
		} else if(idx==10)
		{
			tmp.push_back(conn[6]);	tmp.push_back(conn[7]);
		} else if(idx==11)
		{
			tmp.push_back(conn[7]);	tmp.push_back(conn[4]);
		}
	} else if(eleType==HEXA_27)
	{
		if(idx==0)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[1]);
			tmp.push_back(conn[8]);
		} else if(idx==1)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[2]);
			tmp.push_back(conn[9]);
		} else if(idx==2)
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[3]);
			tmp.push_back(conn[10]);
		} else if(idx==3)
		{
			tmp.push_back(conn[3]);	tmp.push_back(conn[0]);
			tmp.push_back(conn[11]);
		} else if(idx==4)
		{
			tmp.push_back(conn[0]);	tmp.push_back(conn[4]);
			tmp.push_back(conn[12]);
		} else if(idx==5)
		{
			tmp.push_back(conn[1]);	tmp.push_back(conn[5]);
			tmp.push_back(conn[13]);
		} else if(idx==6)
		{
			tmp.push_back(conn[2]);	tmp.push_back(conn[6]);
			tmp.push_back(conn[14]);
		} else if(idx==7)
		{
			tmp.push_back(conn[3]);	tmp.push_back(conn[7]);
			tmp.push_back(conn[15]);
		} else if(idx==8)
		{
			tmp.push_back(conn[4]);	tmp.push_back(conn[5]);
			tmp.push_back(conn[16]);
		} else if(idx==9)
		{
			tmp.push_back(conn[5]);	tmp.push_back(conn[6]);
			tmp.push_back(conn[17]);
		} else if(idx==10)
		{
			tmp.push_back(conn[6]);	tmp.push_back(conn[7]);
			tmp.push_back(conn[18]);
		} else if(idx==11)
		{
			tmp.push_back(conn[7]);	tmp.push_back(conn[4]);
			tmp.push_back(conn[19]);
		}	
	} else
	{
		Terminate("find face nodes for Elements", "The element type is not supported");
	}
	return tmp;
}

char* Section::typeToWord(ElementType_t eleType)
{
    switch(eleType)
    {
        case HEXA_8 : return "HEXA_8";
        case HEXA_27: return "HEXA_27";
        case QUAD_4 : return "QUAD_4";
        case TETRA_4 : return "TETRA_4";
        case TRI_3: return "TRI_3";
        case QUAD_9: return "QUAD_9";
        case PYRA_5: return "PYRA_5";
        // default: return "unknown type";
        default: Terminate("transform type to string", "unknown type");
    }
}

char* BCSection::typeToWord(BCType_t BCType)
{
    switch(BCType)
    {
        case BCFarfield : return "BCFarfield";
        case BCInflow: return "BCInflow";
        case BCOutflow : return "BCOutflow";
        case BCSymmetryPlane : return "BCSymmetryPlane";
        case BCWall: return "BCWall";
        case BCWallViscousHeatFlux: return "BCWallViscousHeatFlux";
        case BCWallInviscid: return "BCWallInviscid";
        case BCWallViscous: return "BCWallViscous";
        case FamilySpecified: return "FamilySpecified";
        // default: return "unknown type";
        default: Terminate("transform BC type to string", "unknown type");
    }
}

bool BCSection::findBCType(label eleID)
{
	if(ptsetType[0]==PointRange)
	{
		// printf("%d, %d, %d\n", eleID, );
		if(eleID<=BCElems[1] && eleID>=BCElems[0]) return true;
		else return false;
	} else if(ptsetType[1]==PointList)
	{
		for (int i = 0; i < nBCElems; ++i)
		{
			if(eleID==BCElems[i]) return true;
		}
		return false;
	} else
	{
		Terminate("findBCType", "unknown point set type");
	}
}

label Section::getFaceType(int nodeNum)
{
	switch(nodeNum)
	{
		case 3 : return TRI_3;
		case 4 : return QUAD_4;
		case 6 : return TRI_6;
		case 8 : return QUAD_8;
		case 9 : return TRI_9;
		default: Terminate("get the face type", "unknown type");
	}
}

} // end namespace HSF
/**
* @file: test.cpp
* @author: Liu Hongbin
* @brief: 
* @date:   2019-10-09 11:04:42
* @last Modified by:   lenovo
* @last Modified time: 2020-01-06 09:45:08
*/
#include <iostream>
#include <fstream>
#include <string>
#include <yaml-cpp/yaml.h>
#include <assert.h>
#include <unistd.h>
#include "cstdlib"
#include "mpi.h"
#include "utilities.hpp"
#include "loadBalancer.hpp"
#include "parameter.hpp"
#include "cgnslib.h"
#define OUT std::cout
#define IN std::cin
#define ENDL std::endl
#define String std::string

using namespace HSF;

// #define DEBUG_YAML

// void loadRegionTopologyFromYAML(String filePtr, Array<Scalar> &s,
// 	ArrayArray<Label> &nei, Label procNum);
// void operator >> (const YAML::Node& node, Array<Scalar>& s);
// void operator >> (const YAML::Node& node, Array<Array<Label> >& nei);
// void operator >> (const YAML::Node& node, Array<Label>& regionIdx);
// void hdf5ToAdf(char* filePtr, char* desFilePtr);

int main(int argc, char** argv)
{
	LoadBalancer *lb = new LoadBalancer();
	OUT<<"hello world!"<<ENDL;

	Parameter para("./config.yaml");

	/// initialize MPI environment
	printf("initialize MPI environment ......\n");
	// int numproces, rank;
	// MPI_Init(&argc, &argv);
	// MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	// MPI_Comm_size(MPI_COMM_WORLD, &numproces);
	// printf("This is process %d, %d processes are launched\n", rank, numproces);
	init_utility_();

	int nPara = 4;
	// char meshFile[100];
	Array<char*> mesh_file(10);
	for (int i = 0; i < mesh_file.size(); ++i)
	{
		mesh_file[i] = new char[100];
	}
	// para.getPara(&nPara, meshFile, "char*", "domain1", "region", "0", "path");
	para.getPara<char>(mesh_file, nPara, "domain1", "region", "0", "path");
	
	printf("reading CGNS file: ");
	for (int i = 0; i < mesh_file.size(); ++i)
	{
		printf("%s, ", mesh_file[i]);
	}
	printf("\n");

	char resultFile[100];
	nPara = 4;
	para.getPara<char>(resultFile, nPara, "domain1", "region", "0", "resPath");
	printf("writing CGNS file: %s\n", resultFile);
	/// read CGNS file
	Array<Region> regs;
	Region reg;
	regs.push_back(reg);

	regs[0].initBeforeBalance(mesh_file);

	
	/// load balance in region
	lb->LoadBalancer_3(regs);

	regs[0].initAfterBalance();

	regs[0].writeMesh(resultFile);

	// regs[0].getMesh().initMesh(resultFile);

	// MPI_Finalize();
	
	return 0;
}

// void loadRegionTopologyFromYAML(String filePtr, Array<Scalar> &s,
// 	ArrayArray<Label> &nei, Label procNum)
// {
// 	OUT<<"reading YAML file: "<<filePtr<<" ......"<<ENDL;
// 	std::ifstream fin(filePtr.c_str());
// 	YAML::Parser parse(fin);
// 	YAML::Node doc;
// 	parse.GetNextDocument(doc);
// 	const YAML::Node& measurement = doc["measurement"];
// 	for (int i = 0; i < measurement.size(); ++i)
// 	{
// 		measurement[i] >> s;
// 	}
// 	Array<Array<Label> > neiTmp;
// 	Array<Label> regIdxTmp;
// 	const YAML::Node& topology = doc["topology"];
// 	for (int i = 0; i < topology.size(); ++i)
// 	{
// 		topology[i] >> neiTmp;
// 		topology[i] >> regIdxTmp;
// 	}

// 	/// transform the vector<vector<int> > to ArrayArray
// 	nei.num = s.size();
// 	nei.startIdx = new Label[nei.num+1];
// 	nei.startIdx[0] = 0;
// 	for (int i = 0; i < nei.num; ++i)
// 	{
// 		nei.startIdx[i+1] = nei.startIdx[i]+neiTmp[regIdxTmp[i]].size();
// 	}
// 	nei.data = new Label[nei.startIdx[nei.num]];
// 	for (int i = 0; i < nei.num; ++i)
// 	{
// 		Label k = 0;
// 		for (int j = nei.startIdx[i]; j < nei.startIdx[i+1]; ++j)
// 		{
// 			nei.data[j] = neiTmp[regIdxTmp[i]][k];
// 			k++;
// 		}
// 	}
// #ifdef DEBUG_YAML
// 	/// check
// 	for (int i = 0; i < nei.num; ++i)
// 	{
// 		for (int j = nei.startIdx[i]; j < nei.startIdx[i+1]; ++j)
// 		{
// 			OUT<<nei.data[j]<<", ";
// 		}
// 	}
// 	OUT<<ENDL;
// #endif
// }

// void operator >> (const YAML::Node& node, Array<Scalar>& s)
// {
// 	String mea;
// 	node["mea"] >> mea;
// 	s.push_back(std::atof(mea.c_str()));
// #ifdef DEBUG_YAML
// 	for (int i = 0; i < s.size(); ++i)
// 	{
// 		OUT<<s[i]<<", ";
// 	}
// 	OUT<<ENDL;
// #endif
// }

// void operator >> (const YAML::Node& node, Array<Array<Label> >& nei)
// {
// 	// String neighbor;
// 	// node["neighbor"].as<string>() >> neighbor;
// 	Array<Label> neiTmp;
// 	int tmp;
// 	const YAML::Node& neighbor = node["neighbor"];
// 	for (int i = 0; i < neighbor.size(); ++i)
// 	{
// 		neighbor[i] >> tmp;
// 		neiTmp.push_back(tmp);
// 	}
// 	nei.push_back(neiTmp);

// #ifdef DEBUG_YAML
// 	for (int i = 0; i < nei.size(); ++i)
// 	{
// 		OUT<<"(";
// 		for (int j = 0; j < nei[i].size(); ++j)
// 		{
// 			OUT<<nei[i][j]<<", ";
// 		}
// 		OUT<<")";
// 	}
// 	OUT<<ENDL;
// #endif
// }

// void operator >> (const YAML::Node& node, Array<Label>& regionIdx)
// {
// 	Label tmp;
// 	node["regionIdx"] >> tmp;
// 	regionIdx.push_back(tmp);

// #ifdef DEBUG_YAML
// 	for (int i = 0; i < regionIdx.size(); ++i)
// 	{
// 		OUT<<regionIdx[i]<<", ";
// 	}
// 	OUT<<ENDL;
// #endif
// }

// void hdf5ToAdf(char* filePtr, char* desFilePtr)
// {
// 	int iFile;
// 	if(cg_open(filePtr, CG_MODE_MODIFY, &iFile))
// 		Terminate("readCGNSFile", cg_get_error());
// 	if(cg_save_as(iFile,  desFilePtr, CG_FILE_ADF2, 0))
// 		Terminate("transformToADF", cg_get_error());
// }
/**
* @file: parameter.cpp
* @author: Liu Hongbin
* @brief:
* @date:   2019-09-20 14:17:19
* @last Modified by:   lenovo
* @last Modified time: 2019-11-29 17:26:14
*/
#include "parameter.hpp"

namespace HSF
{

// /**
// * @brief operator overloading to struct Equation
// */
// // void operator >> (const YAML::Node& node, char* name) {
// // }
// /**
// * @brief operator overloading to struct Equation
// */
// void operator >> (const YAML::Node& node, paraEquation& equ) {
// 	node["name"] >> equ.name;
// 	node["solver"] >> equ.solver;
// 	node["preconditioner"] >> equ.preconditioner;
// 	// printf("name: %c\n", equ.name);
// 	// std::cout<<"name: "<<equ.name<<", solver: "<<equ.solver<<", preconditioner: "<<equ.preconditioner<<std::endl;
// }
// /**
// * @brief operator overloading to struct Scheme
// */
// void operator >> (const YAML::Node& node, paraScheme& scheme) {
// 	node["name"] >> scheme.name;
// 	// std::cout<<"name: "<<scheme.name<<", format:";
// 	const YAML::Node& format = node["format"];
// 	// node["scheme"] >> scheme.format;
// 	for (int i = 0; i < format.size(); ++i)
// 	{
// 		Word tmp;
// 		format[i] >> tmp;
// 		scheme.format.push_back(tmp);
// 		// std::cout<<" "<<tmp;
// 	}
// 	// std::cout<<std::endl; 
// }
// /**
// * @brief operator overloading to struct Turbulent
// */
// void operator >> (const YAML::Node& node, paraTurbulent& turb) {
// 	node["model"] >> turb.model;
// 	// std::cout<<"model: "<<turb.model<<std::endl;
// }
// /**
// * @brief operator overloading to struct Solve
// */
// void operator >> (const YAML::Node& node, paraSolve& slv) {
// 	node["deltaT"] >> slv.deltaT;
// 	node["startT"] >> slv.startT;
// 	node["endT"] >> slv.endT;
// 	node["writeInterval"] >> slv.writeInterval;
// 	// std::cout<<"deltaT: "<<slv.deltaT<<", startT: "<<slv.startT<<", endT: "<<slv.endT<<", writeInterval: "<<slv.writeInterval<<std::endl;
// }
// /**
// * @brief operator overloading to struct Region
// */
// void operator >> (const YAML::Node& node, paraRegion& reg) {
// 	node["name"] >> reg.name;
// 	node["path"] >> reg.path;
// 	// std::cout<<"name: "<<reg.name<<", path: "<<reg.path<<std::endl;
// 	const YAML::Node& scheme = node["scheme"];
// 	// reg.scheme = new paraScheme[scheme.size()];
// 	for (int i = 0; i < scheme.size(); ++i) {
// 		paraScheme tmp;
// 		scheme[i] >> tmp;
// 		reg.scheme.push_back(tmp);
// 	}
// }
// void operator >> (const YAML::Node& node, paraDomain& dom)
// {
// 	node["name"] >> dom.name;
// 	const YAML::Node& equation = node["equation"];
// 	// printf("equation num: %d\n", equation.size());
// 	for (int i = 0; i < equation.size(); ++i)
// 	{
// 		paraEquation tmpEqu;
// 		equation[i] >> tmpEqu;
// 		dom.equ.push_back(tmpEqu);
// 	}

// 	const YAML::Node& region = node["region"];
// 	// printf("region num: %d\n", region.size());
// 	for (int i = 0; i < region.size(); ++i)
// 	{
// 		paraRegion tmpReg;
// 		region[i] >> tmpReg;
// 		dom.reg.push_back(tmpReg);
// 	}

// 	const YAML::Node& turbulent = node["turbulent"];
// 	turbulent >> dom.turb;

// 	const YAML::Node& solve = node["solve"];
// 	solve >> dom.sol;
// }


// para =getPara(node, name)[][][][][];
/**
* @brief read parameter through yaml-cpp
*/
// void Parameter::readPara(const char* paraFile) {
// 	strcpy(paraFile_, paraFile);
	// YAML::Node config = YAML::LoadFile(paraFile);
	// std::ifstream fin(paraFile);
	// YAML::Parser parser(fin);
	// YAML::Node doc;
	// parser.GetNextDocument(doc);
	// std::cout<<config["domain1"]["equation"]["0"]["name"].as<Word>()<<std::endl;
	// // printf("domain num: %d, %d\n", test.size(),doc.size());
	// for (int i = 0; i < doc.size(); i++) {
	// 	const YAML::Node& domain = doc["domain1"];
	// 	// Word tmp;
	// 	// doc["domain"]["name"] >> tmp;
	// 	// std::cout<<tmp<<std::endl;
	// 	paraDomain tmpDom;
	// 	domain >> tmpDom;
	// 	// doc[i] >> tmpDom;
	// 	// const YAML::Node& equation = doc[i]["equation"];
	// 	// for (int j = 0; j < equation.size(); ++j) {
	// 	// 	paraEquation tmp;
	// 	// 	equation[i] >> tmp;
	// 	// 	tmpDom.equ.push_back(tmp);
	// 	// }

	// 	// const YAML::Node& region = doc[i]["region"];
	// 	// for (int j = 0; j < region.size(); ++j) {
	// 	// 	paraRegion tmp;
	// 	// 	region[i] >> tmp;
	// 	// 	tmpDom.reg.push_back(tmp);
	// 	// }

	// 	// const YAML::Node& turbulent = doc[i]["turbulent"];
	// 	// turbulent >> tmpDom.turb;

	// 	// const YAML::Node& solve = doc[i]["solver"];
	// 	// solve >> tmpDom.sol;

	// 	domain_.push_back(tmpDom);
	// }
// }

// void Parameter::getPara(const int* nPara, void* resVal, const char* type, ...)
// {
// 	// Word configFile(paraFile_, paraFile_+strlen(paraFile_));
// 	// std::ifstream fin(configFile.c_str());
// 	// if(!fin) printf("wrong file: %s\n", configFile.c_str());
// 	// printf("%s\n", configFile.c_str());
// 	Word configFile = "./config.yaml";
// 	YAML::Node config = YAML::LoadFile(configFile);
// 	va_list args;
// 	va_start(args, type);
// 	// printf("parameter num: %d, return type: %s\n", *nPara, type);

// 	char* para;
// 	for (int i = 0; i < *nPara; ++i)
// 	{
// 		// printf("%s, \n", va_arg(args, char*));
// 		para = va_arg(args, char*);
// 		// printf("%s\n", para);
// 		config = config[para];
// 		// paras.push_back(Word(para));
// 	}

// 	va_end(args);
// 	Word res = config.as<Word>();
// 	// std::cout<<config.as<Word>()<<std::endl;
// 	if(strcmp(type, "int")==0)
// 	{
// 		int* tmp = (int*)resVal;
// 		tmp[0] = std::stoi(res);
// 	}else if(strcmp(type, "char*")==0)
// 	{
// 		char* tmp = (char*)resVal;
// 		strcpy(tmp, res.c_str());
// 	}else 
// 	{
// 		Terminate("reading parameters", "the type must be the basic type");
// 	}
// }

// void Parameter::printPara()
// {
// 	for (int i = 0; i < domain_.size(); ++i)
// 	{
// 		std::cout<<"Domain name: "<<domain_[i].name<<std::endl;
// 		for (int j = 0; j < domain_[i].equ.size(); ++j)
// 		{
// 			std::cout<<"Equation name: "<<domain_[i].equ[j].name<<", solver: "<<domain_[i].equ[j].solver<<", preconditioner: "<<domain_[i].equ[j].preconditioner<<std::endl;
// 		}
// 		for (int j = 0; j < domain_[i].reg.size(); ++j)
// 		{
// 			std::cout<<"Region name: "<<domain_[i].reg[j].name<<", path: "<<domain_[i].reg[j].path<<std::endl;
// 			std::cout<<"scheme ";
// 			for (int k = 0; k < domain_[i].reg[j].scheme.size(); ++k)
// 			{
// 				std::cout<<"name: "<<domain_[i].reg[j].scheme[k].name<<", format:";
// 				for (int l = 0; l < domain_[i].reg[j].scheme[k].format.size(); ++l)
// 				{
// 					std::cout<<" "<<domain_[i].reg[j].scheme[k].format[l];
// 				}
// 				std::cout<<std::endl;
// 			}
// 		}
// 		std::cout<<"Turbulent model: "<<domain_[i].turb.model<<std::endl;
// 		std::cout<<"Solve deltaT: "<<domain_[i].sol.deltaT<<", startT: "<<domain_[i].sol.startT<<", endT: "<<domain_[i].sol.endT<<", writeInterval: "<<domain_[i].sol.writeInterval<<std::endl;
// 	}
// }

// paraDomain& Parameter::getDomain(char* name)
// {
// 	for (int i = 0; i < domain_.size(); ++i)
// 	{
// 		if(strcmp(domain_[i].name.c_str(),name)==0) return domain_[i];
// 	}
// 	Terminate("getDomain", "failed to find domain");
// }

// getDomain("domain1").getRegion("region1").getPath();
// getDomain("domain1").getTurbulent().getModel();

} // end namespace HSF
/**
* @file:
* @author: Liu Hongbin
* @brief:
* @date:   2019-09-20 14:22:31
* @last Modified by:   lenovo
* @last Modified time: 2020-01-07 10:39:03
*/
#ifndef PARAMETER_HPP 
#define PARAMETER_HPP

#include <iostream>
#include <fstream>
#include <cstring>
#include <yaml-cpp/yaml.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include "cstdlib"
#include "utilities.hpp"

#define Gauss 1
#define linear 2
#define steadyState 3

#define DIC 1
#define DILU 2

namespace HSF
{

// class paraEquation {
//   public:
// 	Word name;
// 	Word solver;
// 	Word preconditioner;

// 	Word getSolver() {return solver;}
// 	Word getPreconditioner() {return preconditioner;}
// };

// class paraScheme {
//   public:
// 	Word name;
// 	Array<Word> format;
// 	Array<Word>& getFormat() {return format;}	
// };

// class paraRegion {
//   public:
// 	Word name;
// 	Word path;
// 	Array<paraScheme> scheme;

// 	Word getPath() {return path;}
// 	paraScheme& getScheme(char* name)
// 	{
// 		for (int i = 0; i < scheme.size(); ++i)
// 		{
// 			if(strcmp(scheme[i].name.c_str(),name)==0) return scheme[i];
// 		}
// 		Terminate("getScheme", "failed to find scheme");		
// 	}
// };

// class paraTurbulent {
//   public:
// 	Word model;
// 	Word getModel() {return model;}
// };

// class paraSolve {
//   public:
// 	float deltaT;
// 	float startT;
// 	float endT;
// 	int writeInterval;
// 	float getDeltaT() {return deltaT;}
// 	float getStartT() {return startT;}
// 	float getEndT() {return endT;}
// 	int getWriteInterval() {return writeInterval;}
// };

// class paraDomain {
//   public:
// 	Word name;
// 	Array<paraEquation> equ;
// 	Array<paraRegion> reg;
// 	paraTurbulent turb;
// 	paraSolve sol;

// 	paraEquation& getEquation(char* name)
// 	{
// 		for (int i = 0; i < equ.size(); ++i)
// 		{
// 			if(strcmp(equ[i].name.c_str(),name)==0) return equ[i];
// 		}
// 		Terminate("getEquation", "failed to find equation");
// 	}

// 	paraRegion& getRegion(char* name)
// 	{
// 		for (int i = 0; i < reg.size(); ++i)
// 		{
// 			if(strcmp(reg[i].name.c_str(),name)==0) return reg[i];
// 		}
// 		Terminate("getRegion", "failed to find region");
// 	}

// 	paraTurbulent& getTurbulent() {return turb;}
// 	paraSolve& getSolve() {return sol;}
// };
/**
* @brief Control parameter
*/
class Parameter {
  private:
	char paraFile_[CHAR_DIM];
	// Array<paraDomain> domain_;
  public:
	/**
	* @brief default constructor
	*/
	Parameter(const char* paraFile) {
		// strncpy(paraFile_, paraFile, length);
		strcpy(paraFile_, paraFile);
	};
	Parameter() {};
	/**
	* @brief deconstructor
	*/
	~Parameter() {};
	/**
	* @brief set the parameter file
	*/

	void setParaFile(const char* paraFile) {
		// strncpy(paraFile_, paraFile, length);
		strcpy(paraFile_, paraFile);
		// printf("%s, %s, %d, %d\n", paraFile_, paraFile, strlen(paraFile), length);
		// paraFile_[strlen(paraFile)]='\0';
	}
	// /**
	// * @brief read parameter through yaml-cpp
	// */
	// void readPara(const char* paraFile);

	/**
	* @brief get the parameter in config file
	* @param[in] nPara the count of parameters
	* @param[out] resVal return value
	* @param[in] ... other parameter
	* @tparam T char, int, float
	*/
	// void getPara(const int* nPara, void* resVal, const char* type, ...);
	template<typename T>
	void getPara(void* resVal, int nPara, ...);

	template<typename T>
	void getPara(void* resVal, char** strList, const int strLen);

	/**
	* @brief get the parameter in config file
	* @param[in] nPara the count of parameters
	* @param[out] resVal return value
	* @param[in] ... other parameter
	* @tparam T char, int, float
	*/
	// void getPara(const int* nPara, void* resVal, const char* type, ...);
	template<typename T>
	void getPara(Array<T*>& resVal, int nPara, ...);

	// void printPara();

	// paraDomain& getDomain(char* name);
};

template<typename T>
void Parameter::getPara(void* resVal, char** strList, const int strLen)
{
	Word configFile = paraFile_;
	YAML::Node config = YAML::LoadFile(configFile);
	if(!config) 
	{
		resVal = NULL;
		printf("YAML file: %s\n", paraFile_);
		Terminate("loadYAMLFile", "please check the parameter string");
	}

	// printf("%d\n", len);
	for (int i = 0; i < strLen; ++i)
	{
		// printf("%s\n", strList[i]);
		config = config[strList[i]];
		if(!config) 
		{
			resVal = NULL;
			Terminate("getPara", "please check the parameter string");
		}
	}
	Word res = config.as<Word>();
	// std::cout<<config.as<Word>()<<std::endl;
	if(typeid(T)==typeid(int))
	{
		int* tmp = (int*)resVal;
		tmp[0] = std::stoi(res);
	}else if(typeid(T)==typeid(char))
	{
		char* tmp = (char*)resVal;
		strcpy(tmp, res.c_str());
	}else if(typeid(T)==typeid(float))
	{
		float* tmp = (float*)resVal;
		tmp[0] = std::stof(res);
	}else 
	{
		Terminate("reading parameters", "the type must be the basic type");
	}	
}

template<typename T>
void Parameter::getPara(void* resVal, int nPara, ...)
{
	Word configFile = paraFile_;
	YAML::Node config = YAML::LoadFile(configFile);
	if(!config) 
	{
		resVal = NULL;
		printf("YAML file: %s\n", paraFile_);
		Terminate("loadYAMLFile", "please check the parameter string");
	}
	va_list args;
	va_start(args, nPara);
	// printf("parameter num: %d, return type: %s\n", *nPara, type);

	char* para;
	for (int i = 0; i < nPara; ++i)
	{
		// printf("%s, \n", va_arg(args, char*));
		para = va_arg(args, char*);
		// printf("%s\n", para);
		config = config[para];
		if(!config) 
		{
			resVal = NULL;
			Terminate("getPara", "please check the parameter string");
		}
		// printf("%d\n", config.size());
		// paras.push_back(Word(para));
	}

	va_end(args);

	Word res = config.as<Word>();
	// std::cout<<config.as<Word>()<<std::endl;
	if(typeid(T)==typeid(int))
	{
		int* tmp = (int*)resVal;
		tmp[0] = std::stoi(res);
	}else if(typeid(T)==typeid(char))
	{
		char* tmp = (char*)resVal;
		strcpy(tmp, res.c_str());
	}else if(typeid(T)==typeid(float))
	{
		float* tmp = (float*)resVal;
		tmp[0] = std::stof(res);
	}else 
	{
		Terminate("reading parameters", "the type must be the basic type");
	}	
}


template<typename T>
void Parameter::getPara(Array<T*>& resVal, int nPara, ...)
{
	Word configFile = paraFile_;
	YAML::Node config = YAML::LoadFile(configFile);
	if(!config) 
	{
		printf("YAML file: %s\n", paraFile_);
		Terminate("loadYAMLFile", "please check the parameter string");
	}
	va_list args;
	va_start(args, nPara);
	// printf("parameter num: %d, return type: %s\n", *nPara, type);

	char* para;
	for (int i = 0; i < nPara; ++i)
	{
		// printf("%s, \n", va_arg(args, char*));
		para = va_arg(args, char*);
		// printf("%s\n", para);
		config = config[para];
		if(!config) 
		{
			Terminate("getPara", "please check the parameter string");
		}
		// printf("%d\n", config.size());
		// paras.push_back(Word(para));
	}

	va_end(args);

	int nRes = config.size();
	// 如果该节点为标量，则重设数组维度
	if(nRes==0) nRes=1;
	
	resVal.resize(nRes);

	for (int i = 0; i < nRes; ++i)
	{
		Word res;
		if(nRes>1) res = config[i].as<Word>();
		else res = config.as<Word>();

		// std::cout<<res<<std::endl;
		if(typeid(T)==typeid(int))
		{
			int* tmp = (int*)resVal[i];
			tmp[0] = std::stoi(res);
		}else if(typeid(T)==typeid(char))
		{
			char* tmp = (char*)resVal[i];
			strcpy(tmp, res.c_str());
		}else if(typeid(T)==typeid(float))
		{
			float* tmp = (float*)resVal[i];
			tmp[0] = std::stof(res);
		}else 
		{
			Terminate("reading parameters", "the type must be the basic type");
		}			
	}	
}
// void Parameter::readPara(const char* paraFile) {
// 	strcpy(paraFile_, paraFile);
// }

// /**
// * @brief operator overloading to struct Equation
// */
// void operator >> (const YAML::Node& node, paraEquation& equ);
// /**
// * @brief operator overloading to struct Scheme
// */
// void operator >> (const YAML::Node& node, paraScheme& scheme);
// /**
// * @brief operator overloading to struct Turbulent
// */
// void operator >> (const YAML::Node& node, paraTurbulent& turb);
// /**
// * @brief operator overloading to struct Solve
// */
// void operator >> (const YAML::Node& node, paraSolve& slv);
// /**
// * @brief operator overloading to struct Region
// */
// void operator >> (const YAML::Node& node, paraRegion& reg);
// /**
// * @brief operator overloading to struct Domain
// */
// void operator >> (const YAML::Node& node, paraDomain& dom);

} // end namespace HSF

#endif

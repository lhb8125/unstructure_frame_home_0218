/**
* @file: utilInterfaces.cpp
*/
#include <yaml-cpp/yaml.h>
#include <stdarg.h>
#include "interface.hpp"
#include "utilInterfaces.h"

using namespace HSF;

#define REGION regs[0]


/*****************************************utility初始化*****************************************/
void init_utility_()
{
  COMM::init(NULL, NULL, "MPI");
}

/*****************************************进程间数据交换****************************************/
// 获取进程号及全局通信规模
void get_pid_( int * pid )
{
  *pid  = COMM::getGlobalId();
}

void get_commsize_( int * commSize )
{
  *commSize = COMM::getGlobalSize();
}



// 规约整形变量序列
// para1 [inout] label类型变量
// para2 [in] 变量个数
// para3 [in] 操作类型
void all_reduce_labels_(label* data, const label* count, unsigned int* op)
{
  vector<label> sdata(data, data+*count);

  CommData datatype;
  if( sizeof(label)  == 4 ) datatype = COMM_INT32_T;
  else if( sizeof(label)  == 8 ) datatype = COMM_INT64_T;
  else 
  {
    PERR<<" Invalid LABEL type! "<<ENDL;
    exit(-1);
  }
  
  Communicator& gComm = COMM::getGlobalComm();
  gComm.allReduce("all_reduce_labels_", &(sdata[0]), data, *count, datatype, 
      *op);
  gComm.finishTask("all_reduce_labels_");
}

// 规约浮点变量序列
// para1 [inout] scalar类型变量
// para2 [in] 变量个数
// para3 [in] 操作类型
void all_reduce_scalars_(scalar* data, const label* count , unsigned int* op)
{
  vector<scalar> sdata(data, data+*count);

  CommData datatype;
  if( sizeof(scalar)  == 4 ) datatype = COMM_FLOAT;
  else if( sizeof(scalar)  == 8 ) datatype = COMM_DOUBLE;
  else 
  {
    PERR<<" Invalid SCALAR type! "<<ENDL;
    exit(-1);
  }
  
  Communicator& gComm = COMM::getGlobalComm();
  gComm.allReduce("all_reduce_scalars_", &(sdata[0]), data, *count, datatype, 
      *op);
  gComm.finishTask("all_reduce_scalars_");
}

// 主进程广播整形序列
// para1 [inout] 变量序列
// para2 [in] 变量个数
void bcast_labels_(label* data, const label* count)
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.bcast("bcast_labels_", data, sizeof(label)*(*count) );
  gComm.finishTask("bcast_labels_");
}

// 主进程广播浮点型序列
// para1 [inout] 变量序列
// para2 [in] 变量个数
void bcast_scalars_(scalar* data, const label* count)
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.bcast("bcast_scalars_", data, sizeof(scalar)*(*count) );
  gComm.finishTask("bcast_scalars_");
}

// 主进程收集整形序列
// para1 [in] 发送序列
// para2 [inout] 接收序列
// para3 [in] 变量个数
void gather_labels_(label* sdata, label* rdata, const label* count )
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.gather("gather_labels_", sdata, sizeof(label)*(*count), rdata, 
     sizeof(label)*(*count));
  gComm.finishTask("gather_labels_");
}

// 主进程收集浮点序列
// para1 [in] 发送序列
// para2 [inout] 接收序列
// para3 [in] 变量个数
void gather_scalars_(scalar* sdata, scalar* rdata, const label* count)
{
  Communicator& gComm = COMM::getGlobalComm();
  gComm.gather("gather_scalars_", sdata, sizeof(scalar)*(*count), rdata, 
     sizeof(scalar)*(*count));
  gComm.finishTask("gather_scalars_");
}

/*******************************************标准输出*******************************************/
// #include "stdarg.h"
// 所有进程输出到特定文件
// para1-... [in] 变量名，不定参数个数
void par_std_out_(const char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  POUT<<pbuffer<<FLUSH;
}

// 指定进程号输出到屏幕
// para1 [in] 进程编号
// para2-... [in] 变量名，不定参数个数
void proc_std_out_(const label pid, const char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  SOUT(pid)<<pbuffer<<FLUSH;
}

// 主核输出到屏幕
// para1-... [in] 变量名，不定参数个数
void master_std_out_(char* format, ...)
{
  va_list args;
  va_start(args, format);

  // print message
  char pbuffer[4096];
  vsprintf(pbuffer, format, args);

  // disable arg list
  va_end(args);

  COUT<<pbuffer<<FLUSH;
}

// Fortran 字符串输出
void fort_cout_(const char* str, const int* len)
{
  COUT<<Word(str, str+*len)<<ENDL;
}
void fort_pout_(const char* str, const int* len)
{
  POUT<<Word(str, str+*len)<<ENDL;
}
void fort_sout_(const int* pid, const char* str, const int* len)
{
  SOUT(*pid)<<Word(str, str+*len)<<ENDL;
}

/*******************************************数据输出*******************************************/
void write_restart_()
{
  //globalRegion.writeAll();
}

//
// @brief 写出网格到CGNS文件中
// @param meshFile [in] CGNS文件名
//
void write_mesh_()
{
  char resultFile[CHAR_DIM];
  int nPara = 4;
  para.getPara<char>(resultFile, nPara, "domain1", "region", "0", "resPath");
  REGION.writeMesh(resultFile);
}

// // para1 [in] 场变量名，输出指定场信息到结果文件中，不定参数个数
// void write_fields_(const int* nPara, ...)
// {
//   va_list args;
//   va_start(args, nPara);
  
//   vector<Word> paras;

//   char* paraWd;
//   for(int i = 0; i < *nPara; i++)
//   {
//     paraWd = va_arg(args, char * );
//     paras.push_back(Word(paraWd) );
//   }

//   va_end(args);

//   COUT<<"received "<<paras.size()<<" fields:"<<ENDL<<'\t';
//   for(int i = 0; i < paras.size(); i++) COUT<<paras[i]<<", "<<FLUSH;
//   COUT<<ENDL;

//   char resultFile[100];
//   int nnPara = 4;
//   para.getPara(&nnPara, resultFile, "char*", "domain1", "region", "0", "resPath");
//   REGION.writeField<scalar>(resultFile, paras[0].c_str(), paras[1].c_str());
  
//   //globalRegion.writeMesh()
//   //for(int ifield = 0; ifield < paras.size(); ifield++ )
//   //  globalRegion.writeField(paras[ifield]);
// }

void write_label_field_(const char* fieldName, const char* fieldType)
{
  char resultFile[CHAR_DIM];
  int nPara = 4;
  para.getPara<char>(resultFile, nPara, "domain1", "region", "0", "resPath");
  REGION.writeField<label>(resultFile, fieldName, fieldType); 
}

void write_scalar_field_(const char* fieldName, const char* fieldType)
{
  char resultFile[CHAR_DIM];
  int nPara = 4;
  para.getPara<char>(resultFile, nPara, "domain1", "region", "0", "resPath");
  REGION.writeField<scalar>(resultFile, fieldName, fieldType); 
}


void get_label_para_(int* retVal, int* nPara, ...)
{
  char* strList[*nPara];

  va_list args;
  va_start(args, nPara);

  char* str;
  for (int i = 0; i < *nPara; ++i)
  {
    str = va_arg(args, char*);
    strList[i] = str;
  }

  va_end(args);

  para.getPara<int>(retVal, strList, *nPara);
}

void get_scalar_para_(float* retVal, int* nPara, ...)
{
  char* strList[*nPara];

  va_list args;
  va_start(args, nPara);

  char* str;
  for (int i = 0; i < *nPara; ++i)
  {
    str = va_arg(args, char*);
    strList[i] = str;
  }

  va_end(args);

  para.getPara<float>(retVal, strList, *nPara);
}

void get_string_para_(char* retVal, int* str_len, int* nPara, ...)
{
  char* strList[*nPara];
  // printf("%s\n", retVal);

  va_list args;
  va_start(args, nPara);

  char* str;
  for (int i = 0; i < *nPara; ++i)
  {
    str = va_arg(args, char*);
    strList[i] = str;
  }

  va_end(args);

  str_len[0] = 0;
  para.getPara<char>(retVal, strList, *nPara);
  while(*retVal++!='\0') str_len[0]++;
}

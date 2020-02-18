// #include "utility.hpp"
#include "stdarg.h"
#include "interface.hpp"

using namespace HSF;

/*****************************************utility初始化*****************************************/
void init_utility_()
{
  COMM::init(NULL, NULL, "MPI");
}

/*******************************************标准输出*******************************************/
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

  SOUT(0)<<pbuffer<<FLUSH;
}


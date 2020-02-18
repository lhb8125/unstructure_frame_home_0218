/* Copyright (C) 
 * 2019 - Hu Ren, rh890127a@163.com
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */
/**
 * @file communicator.hpp
 * @brief class Communicator that define a independent environment for process
 * communications, primitively implemented with MPI.
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-11
 */

#include "mpi.h"
#include "usingCpp.hpp"
#include "basicFunction.hpp"
#include "OStream.hpp"
#include "multiOStream.hpp"
#include "dummyOStream.hpp"

#ifndef HSF_COMMUNICATOR_HPP
#define HSF_COMMUNICATOR_HPP

// set the max message number in the register in case overflow caused by misuse
// of the communicator
#define COMM_MAX_MSG_NUM 999

namespace HSF
{

typedef std::string TaskName;

class CommunicationManager;

// communiation operations
#define CommOp MPI_Op

#define COMM_SUM MPI_SUM
#define COMM_MIN MPI_MIN
#define COMM_MAX MPI_MAX
#define COMM_PROD MPI_PROD
#define COMM_LAND MPI_LAND
#define COMM_LOR MPI_LOR
#define COMM_BAND MPI_BAND
#define COMM_BOR MPI_BOR
#define COMM_MAXLOC MPI_MAXLOC
#define COMM_MINLOC MPI_MINLOC
#define COMM_LXOR MPI_LXOR
#define COMM_BXOR MPI_BXOR
#define COMM_REPLACE MPI_REPLACE

// communication data types
#define CommData MPI_Datatype

#define COMM_CHAR   MPI_CHAR
#define COMM_UNSIGNED_CHAR   MPI_UNSIGNED_CHAR
#define COMM_SHORT   MPI_SHORT
#define COMM_UNSIGNED_SHORT   MPI_UNSIGNED_SHORT
#define COMM_INT   MPI_INT
#define COMM_UNSIGNED   MPI_UNSIGNED
#define COMM_LONG   MPI_LONG
#define COMM_UNSIGNED_LONG   MPI_UNSIGNED_LONG
#define COMM_LONG_LONG_INT   MPI_LONG_LONG_INT
#define COMM_LONG_LONG   MPI_LONG_LONG
#define COMM_FLOAT   MPI_FLOAT
#define COMM_DOUBLE   MPI_DOUBLE
#define COMM_LONG_DOUBLE   MPI_LONG_DOUBLE
#define COMM_BYTE   MPI_BYTE
#define COMM_WCHAR   MPI_WCHAR
#define COMM_PACKED   MPI_PACKED
#define COMM_LB   MPI_LB
#define COMM_UB   MPI_UB
#define COMM_C_COMPLEX   MPI_C_COMPLEX
#define COMM_C_FLOAT_COMPLEX   MPI_C_FLOAT_COMPLEX
#define COMM_C_DOUBLE_COMPLEX   MPI_C_DOUBLE_COMPLEX
#define COMM_C_LONG_DOUBLE_COMPLEX   MPI_C_LONG_DOUBLE_COMPLEX
#define COMM_2INT   MPI_2INT
#define COMM_C_BOOL   MPI_C_BOOL
#define COMM_SIGNED_CHAR   MPI_SIGNED_CHAR
#define COMM_UNSIGNED_LONG_LONG   MPI_UNSIGNED_LONG_LONG
#define COMM_CHARACTER   MPI_CHARACTER
#define COMM_INTEGER   MPI_INTEGER
#define COMM_REAL   MPI_REAL
#define COMM_LOGICAL   MPI_LOGICAL
#define COMM_COMPLEX   MPI_COMPLEX
#define COMM_DOUBLE_PRECISION   MPI_DOUBLE_PRECISION
#define COMM_2INTEGER   MPI_2INTEGER
#define COMM_2REAL   MPI_2REAL
#define COMM_DOUBLE_COMPLEX   MPI_DOUBLE_COMPLEX
#define COMM_2DOUBLE_PRECISION   MPI_2DOUBLE_PRECISION
#define COMM_2COMPLEX   MPI_2COMPLEX
#define COMM_2DOUBLE_COMPLEX   MPI_2DOUBLE_COMPLEX
#define COMM_REAL2   MPI_REAL2
#define COMM_REAL4   MPI_REAL4
#define COMM_COMPLEX8   MPI_COMPLEX8
#define COMM_REAL8   MPI_REAL8
#define COMM_COMPLEX16   MPI_COMPLEX16
#define COMM_REAL16   MPI_REAL16
#define COMM_COMPLEX32   MPI_COMPLEX32
#define COMM_INTEGER1   MPI_INTEGER1
#define COMM_COMPLEX4   MPI_COMPLEX4
#define COMM_INTEGER2   MPI_INTEGER2
#define COMM_INTEGER4   MPI_INTEGER4
#define COMM_INTEGER8   MPI_INTEGER8
#define COMM_INTEGER16   MPI_INTEGER16
#define COMM_INT8_T   MPI_INT8_T
#define COMM_INT16_T   MPI_INT16_T
#define COMM_INT32_T   MPI_INT32_T
#define COMM_INT64_T   MPI_INT64_T
#define COMM_UINT8_T   MPI_UINT8_T
#define COMM_UINT16_T   MPI_UINT16_T
#define COMM_UINT32_T   MPI_UINT32_T
#define COMM_UINT64_T   MPI_UINT64_T
#define COMM_AINT   MPI_AINT
#define COMM_OFFSET   MPI_OFFSET
#define COMM_FLOAT_INT   MPI_FLOAT_INT
#define COMM_DOUBLE_INT   MPI_DOUBLE_INT
#define COMM_LONG_INT   MPI_LONG_INT
#define COMM_SHORT_INT   MPI_SHORT_INT
#define COMM_LONG_DOUBLE_INT   MPI_LONG_DOUBLE_INT

class Communicator
{
public:
  /**
   * @brief CommunicationManager 
   * Declare the CommunicationManager as friend class to enable deep
   * manipulations on Communicators.
   */
  friend class CommunicationManager;

private:
//--------------------------------------------------------------
// communicator info
//--------------------------------------------------------------
  
  MPI_Comm comm_; ///< communicator handle
  
  int size_; ///< communicator size
   
  int rank_; ///< local rank
  
  string fname_; ///< log file name

//--------------------------------------------------------------
// io object
//--------------------------------------------------------------
  /**
   * @brief Output object linked with local master process
   */
  OStream* log_;

//--------------------------------------------------------------
// task registor
//--------------------------------------------------------------
  /**
   * @brief
   * template parameters 1, taskname
   * template parameters 2, all the request need to be waited 
   */
  map<TaskName, vector<MPI_Request> > taskRequests_;
  
  typedef map<TaskName, vector<MPI_Request> > TaskRequestTable;
  typedef pair<TaskName, vector<MPI_Request> > TaskRequestPair;

  MPI_Request* insertRequest( TaskName task); ///< insert requests
  
  /// generate tag
  Hash32 generateTag( const TaskName& task, const int source, const int dest);

  int unregisterTask( TaskName task); ///< unregister task

  size_t messageNum_; ///< message number statistics
 
public:
//--------------------------------------------------------------
// constructor and deconstructor
//--------------------------------------------------------------
  /**
   * @brief Communicator
   * Consctruct an NULL communicator
   */
  Communicator(const string fname)
    :
      fname_(fname), size_(-1), rank_(-1), log_(NULL),messageNum_(0)
  {
  }

  /**
   * @brief ~Communicator
   * defult deconstructor
   */
  virtual ~Communicator()
  {
    if(comm_)
      MPI_Comm_free(&comm_);
    if(log_)
      delete log_;
  }

  virtual void setLogFile( const string& fname)
  {
    fname_ = fname;
    if(this->getMyId() == 0 )
      this -> log_ = new OStream(fname_ );
    else
      this -> log_ = new DummyOStream();
  }


//--------------------------------------------------------------
// Access
//--------------------------------------------------------------
  virtual int getMyId() const { return rank_; }

  virtual int getMySize() const { return size_; }

//--------------------------------------------------------------
// parallel standard I/O
//--------------------------------------------------------------
  /**
   * @brief operator<< 
   * output stream to the communicator log file
   *
   * @return this Communicator object 
   */
  virtual OStream& log() { return *log_; }

//--------------------------------------------------------------
// Data tranmission
// - All implimented with unblocking version. Unbloking collective
// - communication is supported in MPI version higher than 3.0
// - Triger MPI 3.0 use -DMPIv3
//--------------------------------------------------------------
  
  virtual int bcast(const TaskName task, 
      void* buffer, int count, int root = 0 );

  virtual int gather(const TaskName task,
      const void* sendbuf, int sendcount, void* recvbuff, int recvcount, 
      int root = 0 );
  
  virtual int gatherV(const TaskName task,
      const void* sendbuf, int sendcount, void* recvbuff, const int* recvcounts,
      int root = 0 );

  virtual int allGather(const TaskName task,
      const void* sendbuf, int sendcount, void* recvbuff, int recvcount );
  
  virtual int allGatherV(const TaskName task,
      const void* sendbuf, int sendcount, void* recvbuff, const int* recvcounts 
      );

  virtual int scatter(const TaskName task,
      const void* sendbuf, int sendcount, void* recvbuff, int recvcount,
      int root = 0 );

  virtual int scatterV(const TaskName task,
      const void* sendbuf, const int* sendcounts, void* recvbuff, int recvcount,
      int root = 0 );
  
  virtual int send(const TaskName task,
      const void* sendbuf, int sendcount, int senddest);
  
  virtual int recv(const TaskName task,
      void* recvbuf, int recvcount, int recvdest );
  
  // warning: make sure message between each process pair is unique
  virtual int groupSend(const TaskName task,
      const void* sendbuf, int sendnum, const int* sendcounts, 
      const int* senddests);
  
  // warning: make sure message between each process pair is unique
  virtual int groupRecv(const TaskName task,
      void* recvbuf, int recvnum, const int* recvcounts, const int* recvdests
      );
  
  // warning: make sure message between each process pair is unique
  virtual int exchange(const TaskName task,
      const void* sendbuf, void* recvbuf, int num, const int* counts, 
      const int* dests );
  
  virtual int reduce(const TaskName task,
      const void* sendbuf, void* recvbuf, int count, CommData datatype, 
      CommOp op, int root = 0);

  virtual int allReduce(const TaskName task,
       const void* sendbuf, void* recvbuf, int count, CommData datatype,
       CommOp op);

  virtual int finishTask(const TaskName task);

  virtual int barrier()
  {
    MPI_Barrier(this -> comm_ );
  }

};

} // namespace HSF




#endif // HSF_COMMUNICATOR_HPP

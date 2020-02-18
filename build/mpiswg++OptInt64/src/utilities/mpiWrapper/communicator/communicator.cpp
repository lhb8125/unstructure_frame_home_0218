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
 * @file communicator.cpp
 * @brief Communicator primitively implemented with MPI.
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-11
 */

#include "basicFunction.hpp"
#include "communicator.hpp"

namespace HSF
{

//--------------------------------------------------------------
// Task registration
//--------------------------------------------------------------
  
// insert requests
MPI_Request* Communicator::insertRequest( TaskName task )
{
  TaskRequestTable::iterator iter = taskRequests_.find(task);
  if( iter != taskRequests_.end() ) 
  {
    MPI_Request req;
    iter->second.push_back(req);
  }
  else 
  {
    MPI_Request req;
    taskRequests_.insert( TaskRequestPair( task, 
                                           vector<MPI_Request>(1, req) 
                                         ) 
                        );
    iter = taskRequests_.find(task);
    // count the message
    messageNum_++;
  }
  
  return &(*(iter->second.end() - 1 ) );
}

// generate message tag
Hash32 Communicator::generateTag( const TaskName& task, const int source, 
    const int dest)
{
  return strToHash32( task + to_string(source) + to_string(dest) );
}

// unregister task
int Communicator::unregisterTask( TaskName task)
{
  TaskRequestTable::iterator iter = taskRequests_.find(task);
  if( iter != taskRequests_.end() )
    taskRequests_.erase(task);
  return 0;
}

//--------------------------------------------------------------
// Data tranmission
// - All implimented with unblocking version. Unbloking collective
// - communication is supported in MPI version higher than 3.0
// - Triger MPI 3.0 use -DMPIv3
//--------------------------------------------------------------
  
int Communicator::bcast(const TaskName task, 
    void* buffer, int count, int root )
{
#ifdef MPIv3
  MPI_Ibcast(buffer, count, MPI_BYTE, root, this ->comm_, insertRequest(task));
#else
  MPI_Bcast(buffer, count, MPI_BYTE, root, this ->comm_);
#endif
}

int Communicator::gather(const TaskName task,
    const void* sendbuf, int sendcount, void* recvbuf, int recvcount, 
    int root)
{
#ifdef MPIv3
  MPI_Igather(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcount, MPI_BYTE, 
      root, this ->comm_, insertRequest(task) );
#else
  MPI_Gather(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcount, MPI_BYTE, 
      root, this ->comm_);
#endif
}

int Communicator::gatherV(const TaskName task,
    const void* sendbuf, int sendcount, void* recvbuf, const int* recvcounts,
    int root)
{
  int size = this ->getMySize();
  int displs[size];
  displs[0] = 0;
  for( int i = 1; i < size; i++)
    displs[i] = displs[i - 1] + recvcounts[i - 1];
#ifdef MPIv3
  MPI_Igatherv(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcounts, displs, 
      MPI_BYTE, root, this ->comm_, insertRequest(task) );
#else
  MPI_Gatherv(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcounts, displs, 
      MPI_BYTE, root, this ->comm_ );
#endif
}

int Communicator::allGather(const TaskName task,
    const void* sendbuf, int sendcount, void* recvbuf, int recvcount )
{
#ifdef MPIv3
  MPI_Iallgather(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcount, MPI_BYTE, 
      this ->comm_, insertRequest(task) );
#else
  MPI_Allgather(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcount, MPI_BYTE, 
      this ->comm_);
#endif
}

int Communicator::allGatherV(const TaskName task,
    const void* sendbuf, int sendcount, void* recvbuf, const int* recvcounts)
{
  int size = this ->getMySize();
  int displs[size];
  displs[0] = 0;
  for( int i = 1; i < size; i++)
    displs[i] = displs[i - 1] + recvcounts[i - 1];
#ifdef MPIv3
  MPI_Iallgatherv(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcounts, displs, 
      MPI_BYTE, this -> comm_, insertRequest(task) );
#endif
  MPI_Allgatherv(sendbuf, sendcount, MPI_BYTE, recvbuf, recvcounts, displs, 
      MPI_BYTE, this -> comm_ );
}

int Communicator::scatter(const TaskName task,
    const void* sendbuf, int sendcount, void* recvbuf, int recvcount,
    int root )
{
#ifdef MPIv3
  MPI_Iscatter( sendbuf, sendcount, MPI_BYTE, recvbuf, recvcount, MPI_BYTE, 
      root, this -> comm_, insertRequest(task) );
#else
  MPI_Scatter( sendbuf, sendcount, MPI_BYTE, recvbuf, recvcount, MPI_BYTE, 
      root, this -> comm_ );
#endif
}

int Communicator::scatterV(const TaskName task,
    const void* sendbuf, const int* sendcounts, void* recvbuf, int recvcount,
    int root)
{
  int size = this ->getMySize();
  int displs[size];
  displs[0] = 0;
  for( int i = 1; i < size; i++)
    displs[i] = displs[i - 1] + sendcounts[i - 1];
#ifdef MPIv3
  MPI_Iscatterv(sendbuf, sendcounts, displs, MPI_BYTE, recvbuf, recvcount, 
      MPI_BYTE, root, this -> comm_, insertRequest(task) );
#else
  MPI_Scatterv(sendbuf, sendcounts, displs, MPI_BYTE, recvbuf, recvcount, 
      MPI_BYTE, root, this -> comm_);
#endif
}

int Communicator::send(const TaskName task,
    const void* sendbuf, int sendcount, int senddest)
{
  MPI_Isend(sendbuf, sendcount, MPI_BYTE, senddest, 
      generateTag(task, getMyId(), senddest), this -> comm_, insertRequest(task)
      );
}

int Communicator::recv(const TaskName task,
    void* recvbuf, int recvcount, int recvdest )
{
  MPI_Irecv(recvbuf, recvcount, MPI_BYTE, recvdest, 
      generateTag(task, recvdest, getMyId()), this -> comm_, insertRequest(task)
      );
}

// WARNING: make sure message between each process pair is unique
int Communicator::groupSend(const TaskName task,
    const void* sendbuf, int sendnum, const int* sendcounts, 
    const int* senddests)
{
  size_t displ = 0;
  for(int i = 0; i < sendnum; i++)
  {
    MPI_Isend(&((const char*) sendbuf)[displ], sendcounts[i], MPI_BYTE, 
        senddests[i], generateTag(task, getMyId(), senddests[i]), 
        this -> comm_, insertRequest(task) );
    displ += sendcounts[i];
  }
}

// WARNING: make sure message between each process pair is unique
int Communicator::groupRecv(const TaskName task,
    void* recvbuf, int recvnum, const int* recvcounts, const int* recvdests
    )
{
  int displ = 0;
  for(int i = 0; i < recvnum; i++)
  {
    MPI_Irecv(&((char*) recvbuf) [displ], recvcounts[i], MPI_BYTE, 
    	recvdests[i], generateTag(task, recvdests[i], this->getMyId()), 
		this -> comm_, insertRequest(task) );
    displ += recvcounts[i];
  }
}

// WARNING: make sure message between each process pair is unique, and message
// size to exchange is equal at both side of each process pair.
int Communicator::exchange(const TaskName task,
    const void* sendbuf, void* recvbuf, int num, const int* counts, 
    const int* dests )
{
  int displ = 0;
  for(int i = 0; i < num; i++)
  {
    MPI_Isend(&((const char*) sendbuf)[displ], counts[i], MPI_BYTE, dests[i], 
        generateTag(task, getMyId(), dests[i]), this -> comm_, 
        insertRequest(task) );
    MPI_Irecv(&((char*) recvbuf)[displ], counts[i], MPI_BYTE, dests[i], 
        generateTag(task, dests[i], getMyId() ), this -> comm_, 
        insertRequest(task) );
    displ += counts[i];
  }
}

int Communicator::reduce(const TaskName task,
    const void* sendbuf, void* recvbuf, int count, CommData datatype, 
    CommOp op, int root )
{
#ifdef MPIv3
  MPI_Ireduce(sendbuf, recvbuf, count, datatype, op, root, this -> comm_, 
      insertRequest(task) );
#else
  MPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, this -> comm_ );
#endif
}

int Communicator::allReduce(const TaskName task,
     const void* sendbuf, void* recvbuf, int count, CommData datatype,
     CommOp op)
{
#ifdef MPIv3
  MPI_Iallreduce(sendbuf, recvbuf, count, datatype, op, this -> comm_, 
      insertRequest(task) );
#else
  MPI_Allreduce(sendbuf, recvbuf, count, datatype, op, this -> comm_ );
#endif
}

int Communicator::finishTask(const TaskName task)
{
  // check the message overflow
  if( messageNum_ > COMM_MAX_MSG_NUM || messageNum_ < 0 )
  {
    cerr<<__FILE__<<" +"<<__LINE__<<":"<<endl
      <<__FUNCTION__<<":"<<endl
      <<"Error: buffered message number exceed the limit "<<COMM_MAX_MSG_NUM
      <<endl;
    cerr<<"Check and make sure there's no misuse of sending like interfaces"
      <<endl;
    exit(-1);
  }

  TaskRequestTable::iterator iter = taskRequests_.find(task);
  if( iter != taskRequests_.end() )
  {
    int reqNum = iter->second.size();
    if( reqNum != 0 )
    {
      MPI_Request* reqHandle = &(iter->second[0]);
      MPI_Status stat[reqNum];
      MPI_Waitall(reqNum, reqHandle, stat);
    }
    unregisterTask( task );

    Communicator::messageNum_--;
  }
  return 0;

}

} // namespace HSF


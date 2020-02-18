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
 * @file communicationManager.cpp
 * @brief 
 * implemention of the basic CommunicationManager 
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-15
 */


#include "communicationManager.hpp"

namespace HSF
{

//--------------------------------------------------------------
// registeration for runtime selection
//--------------------------------------------------------------
typedef CommunicationManager* (*CommunicationManagerCreatePtr)(int* argc, 
    char*** argv); 
/*
 * @brief 
 * the selecting table with the keys being the hash value of the class 
 * name (or alias) and the value the create function pointer of each derived
 * classes.
 */
Table<Word, CommunicationManagerCreatePtr> 
	CommunicationManager::selectingTable;

namespace
{
/*
 * @brief createCommunicationManager 
 * creat a instatiation of this class
 *
 * @param[in] argc, parameter from main function
 * @param[out] argv, parameter from main function
 * @return 
 */
CommunicationManager* createCommunicationManager(int* argc, 
    char*** argv)
{
  return new CommunicationManager(argc, argv);
}

typedef pair<Word, CommunicationManagerCreatePtr> SelectEntry;
/*
 * @brief AddCommunicationManagerToSelectingTable
 * the internal class help register this class by create a global object
 */
class AddCommunicationManagerToSelectingTable
{
  const Word name_;
public:
  AddCommunicationManagerToSelectingTable(const Word& name, 
      CommunicationManagerCreatePtr ptr)
    :
      name_(name)
  {
    if( CommunicationManager::selectingTable.find(name_) 
        == CommunicationManager::selectingTable.end() ) 
      CommunicationManager::selectingTable.insert(SelectEntry(name_, ptr) );
  }

  ~AddCommunicationManagerToSelectingTable()
  {
    if( CommunicationManager::selectingTable.find(name_) 
        != CommunicationManager::selectingTable.end() )
      CommunicationManager::selectingTable.erase(name_);
  }
} AddCommunicationManagerToSelectingTable_register("MPI", 
	createCommunicationManager);

} // anonymous namespace

} // namespace HSF

namespace HSF
{
//--------------------------------------------------------------
// construction and deconstruction
//--------------------------------------------------------------
/*
 * @brief CommunicationManager 
 * main constructor
 *
 * @param argc, pointer to the parameters from main function
 * @param argv, pointer to the parameters from main function
 */
CommunicationManager::CommunicationManager(int* argc, char*** argv)
{
  int tag; MPI_Initialized(&tag);
  if( tag == 0 ) MPI_Init(argc, argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &globalRank_);
  MPI_Comm_size(MPI_COMM_WORLD, &globalSize_);

  // construct global communicator
  globalComm_ = new Communicator("globalComm");
  // initialize communicator as a friend
  {
    MPI_Comm_dup(MPI_COMM_WORLD, &(globalComm_->comm_) );
    globalComm_->rank_ = globalRank_;
    globalComm_->size_ = globalSize_;

    // assign the global log file file
    if( globalComm_-> getMyId() == 0 )
      globalComm_->log_ = new OStream(cout.rdbuf() );
    else globalComm_->log_ = new DummyOStream();
    // prepare the directory for sub-communicators' log files
    if( globalComm_-> getMyId() == 0 ) remakeDirectory("comm.log");
    globalComm_->barrier();
    // assign the process log file
    if( globalComm_-> getMyId() == 0 ) remakeDirectory("process.log");
    globalComm_->barrier();
    Word procLogFileName = "process.log/" + to_string(globalComm_-> getMyId());
    pOut_ = new OStream(procLogFileName );
    // assign the ERROR out put file
    StrBuf* coutStrBuf= cout.rdbuf();
    pErr_ = new MultiOStream(&coutStrBuf );
    ( (MultiOStream*) pErr_) -> addBuffer(pOut_->getStrBuf() );

    // initialize the global message number 
    globalComm_->messageNum_ = 0;
  }

}

/*
 * @brief ~CommunicationManager 
 * defult deconstructor
 *
 */
CommunicationManager::~CommunicationManager()
{
  delete globalComm_;
  int tag; MPI_Finalized(&tag);
  if( tag == 0 ) MPI_Finalize();
}

/*
 * @brief duplicate 
 * Copy constructor.
 *
 * @param[in] refComm, reference communicator
 * @param[out] newComm, a copy of the reference communicator
 * @return 
 */
int CommunicationManager::duplicate(const Communicator& refComm, 
    Communicator& newComm )
{ 
  if( newComm.comm_)
  {
    cerr<<__FILE__<<" + "<<__LINE__<<": "<<endl
      <<__FUNCTION__<<": "<<endl
      <<"Error: reconstruct a non-empty communicator!"<<endl;
    exit( -1 );
  }
  
  MPI_Comm_dup(refComm.comm_, &(newComm.comm_) );
  
  //newComm.log_  = refComm.log_->clone();
  newComm.size_ = refComm.size_;
  newComm.rank_ = refComm.rank_;
}

/*
 * @brief split 
 * split the referenced communicator to non-overlaping sub-communicators.
 *
 * @param[in] refComm, the reference communicator on which to do splitting
 * @param[out] newComm, the new communicator generated by splitting
 * @param[in] color, tag to identify the grouping rule.
 * @param[in] nPart, the number of new communicators, used only for result 
 * checking
 * @param[in] priority, priority to determine the process id, defult is 1.
 * @return 
 */
int CommunicationManager::split(const Communicator& refComm, 
    Communicator& newComm, const int color, const int nPart, 
    const int priority)
{
  if( newComm.comm_)
  {
    cerr<<__FILE__<<" + "<<__LINE__<<": "<<endl
      <<__FUNCTION__<<": "<<endl
      <<"Error: reconstruct a non-empty communicator!"<<endl;
    exit( -1 );
  }

  // check the part number consistency of colors
  vector<int>* colors = new vector<int> ( refComm.getMySize() );
  (const_cast<Communicator&>(refComm) ).allGather("allGatherColorsForSplit", &color, sizeof(int), 
      &((*colors)[0]), colors->size()*sizeof(int) );
  (const_cast<Communicator&>(refComm) ).finishTask("allGatherColorsForSplit");
  set<int>* colorSet = new set<int>;
  for(int i = 0 ; i < colors->size(); i++)
    colorSet->insert((*colors)[i]);
  if( colorSet->size() != nPart )
  {
    cerr<<__FILE__<<" + "<<__LINE__<<": "<<endl
      <<__FUNCTION__<<": "<<endl
      <<"Error: the colors for split is not consist with given part number!"
      <<endl;
    exit( -1 );
  }
  delete colors;
  delete colorSet;

  MPI_Comm_split(refComm.comm_, color, priority, &(newComm.comm_) );
  setup(newComm);
}

/*
 * @brief split 
 * extract a communicator to from the reference communicator.
 *
 * @param[in] refComm, the reference communicator on which to do extraction
 * @param[out] newComm, the new communicator generated by extraction
 * @param[in] color, tag to identify whether local process will participate in
 * the new communicator or not.
 * checking
 * @param[in] priority, priority to determine the process id, defult is 1.
 * @return 
 */
int CommunicationManager::extract(const Communicator& refComm, 
    Communicator& newComm, const bool color, int priority)
{
  if( newComm.comm_)
  {
    cerr<<__FILE__<<" + "<<__LINE__<<": "<<endl
      <<__FUNCTION__<<": "<<endl
      <<"Error: reconstruct a non-empty communicator!"<<endl;
    exit( -1 );
  }
  
  int color_int;
  if( color ) color_int = 1;
  else color_int = 0;
  MPI_Comm_split(refComm.comm_, color_int, priority, &(newComm.comm_) );

  if( !color )
    MPI_Comm_free( &(newComm.comm_) );
  else
    setup(newComm);
}

/*
 * @brief reBalance 
 * calculate rebalance solution of the input communicators ( from different
 * processes)
 *
 * @param[in] oldComm, The local communicator to be rebalanced with other 
 * remote communicators. All the input communicators must have NO 
 * intersections with each other.
 * @newComm[out], The new local communicator generated with better load
 * balance.
 * @param[in] weight, non-negetive integer representing the weight of local
 * process
 * @param[in] thresholdRatio, ratio to ideal load balance (100%), non-negative
 * @return 
 */
int CommunicationManager::reBalance(const Communicator& oldComm, 
    Communicator& newComm, const int weight, const float thresholdRatio )
{
    cerr<<__FILE__<<" +"<<__LINE__<<":"<<endl
      <<__FUNCTION__<<":"<<endl
      <<"Error: reBalance is not implemented! "<<endl;
    abort();
}

/*
 * @brief calculateBalance 
 * calculate the balance ratio of the input communicators ( from different
 * processes)
 *
 * @param[in] refComm, The local communicator to be rebalanced with other 
 * remote communicators. All the input communicators must have NO 
 * intersections with each other.
 * @param[in] weight, non-negetive integer representing the weight of local
 * process
 * @return current ratio to ideal load balance (100%), non-negative
 */
float CommunicationManager::calculateBalance(const Communicator& refComm, 
    const int weight)
{
    cerr<<__FILE__<<" +"<<__LINE__<<":"<<endl
      <<__FUNCTION__<<":"<<endl
      <<"Error: calculateBalance is not implemented! "<<endl;
    abort();
}

} // namespace HSF

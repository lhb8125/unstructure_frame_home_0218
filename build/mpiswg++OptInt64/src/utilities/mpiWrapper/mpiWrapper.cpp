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
 * @file mpiWrapper.cpp
 * @brief
 * global interfaces for communication 
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-15
 */

#include "mpiWrapper.hpp"

namespace
{

using namespace HSF;

/**
 * @brief 
 * global tag that indicate whether communication environment is initialized or
 * not.
 */
bool initialized_ = false;

/**
 * @brief InitCommRegister 
 * class to construct an object that will call COMM::stop to release
 * communication environment in deconstructor. Primitively only be used in
 * COMM::init.
 */
class InitCommRegister
{
public:
  InitCommRegister()
  { initialized_ = true; }

  ~InitCommRegister()
  {
    if( initialized_ )
    {
      COMM::stop();
    }
  }
};

}

namespace HSF
{

// static global communication manager
CommunicationManager* COMM::globalManager = NULL;

void COMM::init(int* argc, char*** argv, char* type)
{
  // prevent reinitialized and enable auto calling to COMM::stop to release
  // communication environment.
  if( initialized_ == false )
    static InitCommRegister reg;
  else
  {
	COMM::pErr()<<"!Proc["<<COMM::getGlobalId()<<"]: "<<'\n'<<__FILE__\
  	<<" +"<<__LINE__<<__FUNCTION__<<": "<<'\n'<<"Error: ";
    //PERR<<"Multiple initialization of communication environment!"<<ENDL;
    exit(-1);
  }

  COMM::globalManager = CommunicationManager::New(argc, argv, type);
}

void COMM::stop()
{
  // Warning: will not called delete yet, since there will be error occured
  // after the program finished!
  //if(COMM::globalManager != NULL )
  //  delete COMM::globalManager;
}

} // namespace HSF

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
 * @file OStream.h
 * @brief Standard OUTPUT Stream
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-05
 */

#include <stdint.h>
#include "usingCpp.hpp"

#ifndef HSF_OSTREAM_HPP
#define HSF_OSTREAM_HPP


namespace HSF
{

/**
 * @brief OsOp
 * The type of function pointer that matchs "std::endl" and "std::flush".
 */
typedef std::ostream& (*OsOp) ( std::ostream& );
typedef std::streambuf StrBuf;

#define ENDL std::endl
#define FLUSH std::flush

class OStream
{

  // core file stream
  ostream* file_;
  // if it is redirected
  bool redirected_;
  // original stream buffer holder in case redirected
  StrBuf* buffer_;

public:
//--------------------------------------------------------------
// construct & deconstruct
//--------------------------------------------------------------

  // construct empty
  OStream()
    :
      redirected_(false), buffer_(NULL)
  {
    file_ = new ofstream();
  }
  // construct from file name
  OStream(const string filename)
    :
      redirected_(false), buffer_(NULL)
  {
    file_ = new ofstream(filename.c_str());
  }

  // construct from file buffer
  OStream(StrBuf* rbuf )
    :
      redirected_(true)
  {
    file_ = new ofstream();
    buffer_ = file_->rdbuf();
    file_ -> rdbuf(rbuf);
  }

// No clone or operator= (before C++11) for ostream 
/* 
  // copy constructor
  OStream( const OStream& ref )
    : 
      redirected_(false), buffer_(NULL)
  {
    file_ = new ofstream();
    *(this -> file_) = (ofstream) *(ref.getRawStream() );
    this -> redirected_ = ref.redirected();
    this -> buffer_ = ref.getStrBuf()->clone();
  }
  
  // clone
  virtual OStream* clone() { return new OStream(*this) }; 

  // assign operator
  void operator = ( const OStream& ref )
  { 
    // deconstruct first
    if( redirected_ ) file_->rdbuf(buffer_);
    if( file_->rdbuf() != cout.rdbuf() 
        && file_->rdbuf() != NULL )
      ((ofstream*) file_)->close();
    // copy data
    file_ = new ofstream();
    *(this -> file_) = (ofstream) *(ref.getRawStream() );
    this -> redirected_ = ref.redirected();
    this -> buffer_ = ref.getStrBuf()->clone();
  }
*/

  // deconstruct
  virtual ~OStream()
  {
    if( redirected_ ) file_->rdbuf(buffer_);
    if( file_->rdbuf() != cout.rdbuf() 
        && file_->rdbuf() != NULL )
      ((ofstream*) file_)->close();
    if( file_ != NULL ) delete file_;
  }


//--------------------------------------------------------------
// redirecting & accessing
//--------------------------------------------------------------
  virtual int redirect(StrBuf* rbuf )
  {
    if(redirected_) file_->rdbuf(rbuf);
    else 
    {
      redirected_ = true;
      buffer_ = file_->rdbuf();
      file_->rdbuf(rbuf);
    }
  }

  virtual int reset()
  {
    if(redirected_) 
    {
      file_->rdbuf(buffer_);
      redirected_ = false;
    }
  }
  
  virtual const ostream* getRawStream() { return file_; }
  
  virtual StrBuf* getStrBuf() { return file_->rdbuf(); }

  virtual bool redirected() { return this->redirected_; }


//--------------------------------------------------------------
// streaming operator
//--------------------------------------------------------------
  virtual OStream & operator<<(char chrt)
  {
    *(this->file_)<<chrt;
    return (OStream &) *this;
  }

  virtual OStream & operator<<(string str)
  {
    *(this->file_)<<str;
    return (OStream &) *this;
  }

  virtual OStream & operator<<(int64_t val)
  {
    *(this->file_)<<val;
    return (OStream &) *this;
  }

  virtual OStream & operator<<(int32_t val)
  {
    *(this->file_)<<val;
    return (OStream &) *this;
  }

  virtual OStream & operator<<(unsigned long val)
  {
    *(this->file_)<<val;
    return (OStream &) *this;
  }

  virtual OStream & operator<<(unsigned int val)
  {
    *(this->file_)<<val;
    return (OStream &) *this;
  }

  virtual OStream & operator<<(double val)
  {
    *(this->file_)<<val;
    return (OStream &) *this;
  }
  
  virtual OStream & operator<<(float val)
  {
    *(this->file_)<<val;
    return (OStream &) *this;
  }

  /**
   * @brief operator<<, interface accept OsOp type parameters 
   * @param[in] opt, represent parameter like "ENDL" and "FLUSH".
   * @return 
   */
  virtual OStream & operator<<(OsOp opt)
  {
    *(this->file_)<<opt;
    return (OStream &) *this;
  }

};

}



#endif // HSF_OSTREAM_HPP

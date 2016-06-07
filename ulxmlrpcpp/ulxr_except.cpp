/***************************************************************************
                    ulxr_except.h  -  exception handling
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_except.cpp 10942 2011-09-13 14:35:52Z korosteleva $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/


#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {

    Exception::Exception(int fc, const std::string &s) : std::exception(),
        reason(s), faultcode(fc)
    {
        ULXR_DOUT("=== Exception: " << s);
    }


    Exception::~Exception() throw()
    {
    }


    std::string Exception::why() const
    {
        return reason;
    }


    const char* Exception::what() const throw()
    {
        what_helper = why();
        return what_helper.c_str();
    }


    int Exception::getFaultCode() const
    {
        return faultcode;
    }


/////////////////////////////////////////////////////////////////////////


    ConnectionException::ConnectionException(int fc, const std::string &phrase, int stat)
        : Exception(fc, phrase)
        , status(stat)
    {
    }


    ConnectionException::~ConnectionException() throw()
    {
    }


    int ConnectionException::getStatusCode() const
    {
        return status;
    }


/////////////////////////////////////////////////////////////////////////


    RuntimeException::RuntimeException(int fc, const std::string &s)
        : Exception(fc, s)
    {
    }


    RuntimeException::~RuntimeException() throw()
    {
    }


/////////////////////////////////////////////////////////////////////////


    XmlException::XmlException(int fc, const std::string &s, int l,
                               const std::string &err)
        : Exception(fc, s), line(l), xmlerror(err)
    {
    }


    XmlException::~XmlException() throw()
    {
    }


    int XmlException::getErrorLine() const
    {
        return line;
    }


    std::string XmlException::getErrorString() const
    {
        return xmlerror;
    }


/////////////////////////////////////////////////////////////////////////


    ParameterException::ParameterException(int fc, const std::string &s)
        : Exception(fc, s)
    {
    }


    ParameterException::~ParameterException()  throw()
    {
    }


/////////////////////////////////////////////////////////////////////////


    MethodException::MethodException(int fc, const std::string &s)
        : Exception(fc, s)
    {
    }


    MethodException::~MethodException()  throw()
    {
    }


}  // namespace ulxr

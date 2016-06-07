/***************************************************************************
                    ulxr_except.h  -  exception handling
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_except.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_EXCEPT_H
#define ULXR_EXCEPT_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <stdexcept>


namespace ulxr {


    /** Base class for XML RPC exceptions.
      * @ingroup grp_ulxr_utilit
      */
    class  Exception : public std::exception
    {
    public:

        /** Constructs an exception.
          * @param fc number representing the error class
          * @param s  the reason for the exception
          */
        Exception(int fc, const std::string &s);

        /** Destroys the exception.
          */
        virtual ~Exception()  throw();

        /** Gets the reason for the exception.
          * @return   the reason
          */
        virtual std::string why() const;

        /** Gets a number representing the error.
          * @return   the fault code
          */
        virtual int getFaultCode() const;

        /** Returns the exception description.
          * Provided for compatibility with std::exception. If Unicode is
          * activated you might get corrupt information if you are beyond
          * latin1 characters.
          * @return   description as pointer to a latin1 string.
          */
        virtual const char *what() const throw();

    private:

        std::string reason;
        int    faultcode;
        mutable std::string  what_helper;
    };


    /** Exception due to connection or network problems.
      */
    class  ConnectionException : public Exception
    {
    public:

        /** Constructs a connection exception.
          * @param fc      number representing the error class
          * @param stat    the error number (default is according to http documentation if applicable)
          * @param phrase  a human readable phrase describing the status
          */
        ConnectionException(int fc, const std::string &phrase, int stat);

        /** Destroys the exception.
          */
        virtual ~ConnectionException()  throw();

        /** Gets the status number.
          * @return status number
          */
        int getStatusCode() const;

    private:

        int    status;
    };


    /** Exception due to runtime errors.
      */
    class  RuntimeException : public Exception
    {
    public:

        /** Constructs a runtime exception.
          * @param fc number representing the error class
          * @param s  the reason for the exception
          */
        RuntimeException(int fc, const std::string &s);

        /** Destroys the exception.
          */
        virtual ~RuntimeException()  throw();
    };


    /** XML RPC exception due to a faulty xml structure.
      */
    class  XmlException : public Exception
    {
    public:

        /** Constructs an xml exception.
          * @param fc   number representing the error class
          * @param s    the reason for the exception
          * @param l    the faulty line (relative to the body start!)
          * @param err  a human readable error string
          */
        XmlException(int fc, const std::string &s, int l, const std::string &err);

        /** Destroys the exception.
          */
        virtual ~XmlException()  throw();

        /** Gets the line number of the error.
          * @return the line number
          */
        int getErrorLine() const;

        /** Gets the error string.
          * @return the string
          */
        std::string getErrorString() const;

    private:

        int        line;
        std::string  xmlerror;
    };


    /** XML RPC exception due to faulty rpc parameters.
      * One possibility is the wrong type.
      */
    class  ParameterException : public Exception
    {
    public:

        /** Constructs a parameter exception.
          * @param fc number representing the error class
          * @param s  the reason for the exception
          */
        ParameterException(int fc, const std::string &s);

        /** Destroys the exception.
          */
        virtual ~ParameterException()  throw();
    };


    /** XML RPC exception due to problems while executing
      * the remote method. User fault codes should be positive.
      */
    class  MethodException : public Exception
    {
    public:

        /** Constructs a method exception.
          * @param fc   number representing the error class
          * @param s    the reason for the exception
          */
        MethodException(int fc, const std::string &s);

        /** Destroys the exception.
          */
        virtual ~MethodException()  throw();
    };


} // namespace


#endif // ULXR_EXCEPT_H


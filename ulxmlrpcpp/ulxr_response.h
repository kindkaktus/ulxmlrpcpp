/***************************************************************************
                 ulxr_response.cpp  -  create xml-rpc response
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_response.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_RESPONSE_H
#define ULXR_RESPONSE_H


#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_value.h>


namespace ulxr {

    class Void;


    /** Abstraction of a response from a remote server.
      * You should take care to interpret the data correctly as XML-RPC
      * distinguishes between "normal" return values from the remote method
      * and error messages that indicate problems while transporting and
      * invoking the call.
      * @ingroup grp_ulxr_rpc
      */
    class  MethodResponse
    {
    public:

        /** Default constructor for an empty response.
          */
        explicit MethodResponse();

        /** Destroys the method response.
          */
        virtual ~MethodResponse();

        /** Constructs a "fault reponse" to indicate RPC problems.
          * The number and string are system dependent.
          * @param  fval   error code
          * @param  fstr   human readable desciption of the error
          */
        MethodResponse(int fval, const std::string &fstr);

        /** Constructs a return value from the remote method.
          * the value can be of any type. To return more than one
          * Value at once use an array or a Struct.
          * @param  val   the return value.
          */
        MethodResponse (const Value &val);

        /** Constructs an empty return value from the remote method.
          * Only exists for completeness and yields the same as the default constructor.
          * @param  val   the return value.
          */
        MethodResponse (const Void &val);

        /** Returns the signature of this call.
          * The signature consists of all type names in this call delimited by
          * commas. Elements of arrays are surrounded by braces. Structure elements
          * and element pairs of structs are grouped by curly braces.
          *
          * Example:
          * <pre>
          *  Array:  [int,double,string]
          *  Struct: {{first,int},{second,double}}
          * </pre>
          *
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the call as xml string.
          * The method cal is converted to an xml text. It is prepended with
          * the necessary xml procession instruction with version and encoding
          * set to UTF-8. The structure of the text is indented to facilitate
          * easy reading.
          * @param  indent   current indentation level
          * @return  The xml conentent
          */
        virtual std::string getXml(int indent = 0) const;

        /** Constructs a "fault reponse" to indicate RPC problems.
          * The number and string are system dependent.
          * @param  fval   error code
          * @param  fstr   human readable desciption of the error
          */
        void setFault(int fval, const std::string &fstr);

        /** Sets the return value from the remote method.
          * The value can be of any type. To return more than one
          * Value at once use an Array or a Struct.
          * @param  val   the return value.
          */
        void setResult (const Value &val);

        /** Gets the return value from the remote method.
          * The value can be of any type, even an Array or a Struct.
          * If the response is faulty, is contains a Struct with two elements:
          *  @li  an int named "faultCode"
          *  @li  a string with name "faultString"
          * @return the return value.
          */
        const Value& getResult() const;

        /** Tests the method response state.
          * @return  true if RPC processing went OK.
          */
        bool isOK() const;

    private:

        bool   wasOk;
        Value  respval;
    };


}  // namespace ulxr

#endif // ULXR_RESPONSE_H

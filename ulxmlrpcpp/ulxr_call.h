/***************************************************************************
                  ulxr_call.h  -  create xml-rpc calling data
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_call.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_CALL_H
#define ULXR_CALL_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_value.h>

namespace ulxr {


    /** Abstraction of a MethodCall on a remote server.
      * This call provides access to the name and all parameters.
      * @ingroup grp_ulxr_rpc
      */
    class  MethodCall
    {
    public:

        /** Creates the method call.
          * @param  name  the name of the call
          */
        MethodCall(const char *name);

        /** Creates an empty method call.
          */
        MethodCall();

        /** Creates the method call.
          * @param  name  the name of the call
          */
        MethodCall(const std::string &name);

        /** Destroys a method call.
          */
        virtual ~MethodCall();

        /** Returns the signature of this call.
          * The signature consists of all type names in this call delimited by
          * commas. Elements of arrays are surrounded by braces. Structure elements
          * and element pairs of structs are grouped by curly braces.
          *
          * Example:
          * <pre>
          *  Array  [int,double,string]
          *  Struct {{first,int},{second,double}}
          * </pre>
          *
          * @param   braces  true: add methodname and surounding braces to signature string
          * @return  The signature
          */
        virtual std::string getSignature(bool braces = true) const;


        /** Returns the call as xml string.
          * The method call is converted to an xml text. It is prepended with
          * the necessary xml procession instruction with version and encoding
          * set to UTF-8. The structure of the text is indented to facilitate
          * easy reading.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

        /** Adds another parameter to this call.
          * @param  val   the "value" of this parameter
          */
        MethodCall& addParam (const Value &val);

        /** Sets the parameter to this call.
          * The previous parameraters are removed.
          * @param  val   the "value" of this parameter
          */
        MethodCall& setParam (const Value &val);

        /** Sets the parameter for the goven index to this call.
          * @param  val   the "value" of this parameter
          */
        MethodCall& setParam (unsigned ind, const Value &val);

        /** Returns one of the parameters of this call.
          * @param  ind   index of this value
          * @return   the value of this parameter
          */
        Value getParam(unsigned ind) const;

        /** Returns the number of parameters in the call.
          * @return   the number of parameters
          */
        unsigned numParams() const;

        /** Removes all parameters from this call.
          */
        void clear();

        /** Returns the method name.
          * @return   the name
          */
        std::string getMethodName() const;

        /** Sets the method name.
          * @param  nm   the method name
          */
        void setMethodName(const std::string &nm);

    private:

        std::string            methodname;
        std::vector<Value>   params;
    };


}  // namespace ulxr


#endif // ULXR_CALL_H

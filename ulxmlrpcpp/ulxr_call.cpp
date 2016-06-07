/***************************************************************************
                  ulxr_call.h  -  create xml-rpc calling data
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_call.cpp 10942 2011-09-13 14:35:52Z korosteleva $

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

//#define ULXR_SHOW_TRACE
//#define ULXR_DEBUG_OUTPUT
//#define ULXR_SHOW_READ
//#define ULXR_SHOW_WRITE


#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_call.h>
#include <ulxmlrpcpp/ulxr_except.h>

namespace ulxr {


    MethodCall::MethodCall(const char *name)
    {
        methodname = name;
    }


    MethodCall::MethodCall()
    {
    }


    MethodCall::~MethodCall()
    {
    }


    MethodCall::MethodCall(const std::string &name)
    {
        methodname = name;
    }


    MethodCall&  /**/ MethodCall::addParam (const Value &val)
    {
        params.push_back(val);
        return *this;
    }


    MethodCall&  /**/ MethodCall::setParam (const Value &val)
    {
        clear();
        return addParam(val);
    }


    MethodCall&  /**/ MethodCall::setParam (unsigned ind, const Value &val)
    {
        if (ind < params.size() )
        {
            params[ind] = val;
            return *this;
        }

        throw ParameterException(InvalidMethodParameterError,
                                 "MethodCall::setParam: Parameter index " + toString(ind) + " in " + getSignature() + " method is out-of-range");
    }



    std::string MethodCall::getSignature(bool name_braces) const
    {
        std::string s;
        if (name_braces)
            s += methodname + "(";

        bool comma = params.size() >= 1;
        for (unsigned i = 0; i < params.size(); ++i) {
            if (comma && i != 0)
                s += ',';
            s += params[i].getSignature();
        }

        if (name_braces)
            s += ")";
        return s;
    }


    std::string MethodCall::getXml(int indent) const
    {
        std::string ind = getXmlIndent(indent);
        std::string ind1 = getXmlIndent(indent+1);
        std::string ind2 = getXmlIndent(indent+2);
        std::string s = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + getXmlLinefeed();
        s += ind + "<methodCall>" + getXmlLinefeed();
        s += ind1 + "<methodName>"+methodname+"</methodName>" + getXmlLinefeed();

        s += ind1 + "<params>" + getXmlLinefeed();

        for (std::vector<Value>::const_iterator
                it = params.begin(); it != params.end(); ++it)
        {
            s += ind2 + "<param>" + getXmlLinefeed();
            s += (*it).getXml(indent+3) + getXmlLinefeed();
            s += ind2 + "</param>" + getXmlLinefeed();
        }

        s += ind1 + "</params>" + getXmlLinefeed();
        s += ind + "</methodCall>";
        return s;
    }



    Value MethodCall::getParam(unsigned ind) const
    {
        if (ind < params.size() )
            return params[ind];

        throw ParameterException(InvalidMethodParameterError,
                                 "MethodCall::getParam: Parameter index " + toString(ind) + " in " + getSignature() + " method is out-of-range.");
    }

    unsigned MethodCall::numParams() const
    {
        return params.size();
    }


    void MethodCall::clear()
    {
        params.clear();
    }


    std::string MethodCall::getMethodName() const
    {
        return methodname;
    }


    void MethodCall::setMethodName(const std::string &nm)
    {
        methodname = nm;
    }


}  // namespace ulxr


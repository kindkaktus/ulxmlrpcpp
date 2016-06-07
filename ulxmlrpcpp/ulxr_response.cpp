/***************************************************************************
                 ulxr_response.cpp  -  create xml-rpc response
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_response.cpp 10942 2011-09-13 14:35:52Z korosteleva $

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

#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_value.h>


namespace ulxr {


    MethodResponse::MethodResponse()
        : wasOk(true)
    {
    }


    MethodResponse::MethodResponse (const Void & /*val*/)
        : wasOk(true)
    {
    }


    MethodResponse::MethodResponse(int fval, const std::string &fstr)

    {
        setFault(fval, fstr);
    }


    MethodResponse::MethodResponse (const Value &val)
    {
        setResult (val);
    }


    MethodResponse::~MethodResponse()
    {
    }


    bool MethodResponse::isOK() const
    {
        return wasOk;
    }


    void MethodResponse::setFault(int fval, const std::string &fstr)
    {
        ULXR_TRACE("setFault");
        wasOk = false;
        Struct st;
        st.addMember("faultCode", Integer(fval));
        st.addMember("faultString", RpcString(fstr));
        respval = st;
    }


    void MethodResponse::setResult (const Value &val)
    {
        ULXR_TRACE("setResult");
        wasOk = true;
        respval = val;
    }


    const Value& MethodResponse::getResult() const
    {
        return respval;
    }


    std::string MethodResponse::getSignature(bool deep) const
    {
        return respval.getSignature(deep);
    }


    std::string MethodResponse::getXml(int indent) const
    {
        std::string ind = getXmlIndent(indent);
        std::string ind1 = getXmlIndent(indent+1);
        std::string ind2 = getXmlIndent(indent+2);
        std::string s = "<?xml version=\"1.0\" encoding=\"utf-8\"?>" + getXmlLinefeed();
        s += ind + "<methodResponse>" + getXmlLinefeed();
        if (wasOk)
        {
            s += ind1 + "<params>" + getXmlLinefeed();
            if (!respval.isVoid())
            {
                s += ind2 + "<param>" + getXmlLinefeed();
                s += respval.getXml(indent+3) + getXmlLinefeed();
                s += ind2 + "</param>" + getXmlLinefeed();
            }
            s += ind1 + "</params>" + getXmlLinefeed();
        }
        else
        {
            s += ind1 + "<fault>" + getXmlLinefeed();
            s += respval.getXml(indent+2) +  getXmlLinefeed();
            s += ind1 + "</fault>" + getXmlLinefeed();
        }
        s += ind + "</methodResponse>";
        return s;
    }




}  // namespace ulxr


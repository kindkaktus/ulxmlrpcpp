/**************************************************************************
               ulxr_callparse_base.cpp  -  parse rpc method call
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_callparse_base.cpp 10942 2011-09-13 14:35:52Z korosteleva $

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

#include <ulxmlrpcpp/ulxr_callparse_base.h>


namespace ulxr {


    MethodCallParserBase::~MethodCallParserBase()
    {
    }


    unsigned MethodCallParserBase::numParams() const
    {
        return methodcall.numParams();
    }


    Value MethodCallParserBase::getParam(unsigned ind) const
    {
        return methodcall.getParam(ind);
    }


    std::string MethodCallParserBase::getMethodName() const
    {
        return methodcall.getMethodName();
    }


    MethodCall MethodCallParserBase::getMethodCall() const
    {
        return methodcall;
    }


    /*
    string MethodCallParserBase::ValueState::getStateName() const
    {
      switch (state)
      {
        case eMethodCall      : return "eMethodCall";
        case eMethodName      : return "eMethodName";
        case eParams          : return "eParams";
        case eParam           : return "eParam";
        case eCallParserLast  : return "eCallParserLast";
      }

      return ValueParser::getStateName();
    }
    */

}  // namespace ulxr


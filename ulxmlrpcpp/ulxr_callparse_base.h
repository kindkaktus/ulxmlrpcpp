/***************************************************************************
               ulxr_callparse_base.h  -  parse rpc method call
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_callparse_base.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_CALLPARSE_BASE_H
#define ULXR_CALLPARSE_BASE_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_call.h>
#include <ulxmlrpcpp/ulxr_valueparse_base.h>


namespace ulxr {


    /** An xml parser for a MethodCall.
      * @ingroup grp_ulxr_parser
      */
    class  MethodCallParserBase
    {
    public:

        /** Destroy parser.
          */
        virtual ~MethodCallParserBase();

        /** Gets the number of parameters in the method call.
          * @return amount of parameters.
          */
        unsigned numParams() const;

        /** Gets the amount of parameters in the method call.
          * @param  ind    index of the parameter
          * @return Value of the parameter
          */
        Value getParam(unsigned ind) const;

        /** Gets the name of the method.
          * @return the method name.
          */
        std::string getMethodName() const;

        /** Gets the complete MethodCall with all its data.
          * @return the method.
          */
        MethodCall getMethodCall() const;

        enum CallState
        {
            eMethodCall = ValueParserBase::eValueParserLast,
            eMethodName,
            eParams,
            eParam,
            eCallParserLast
        };

    protected:

        MethodCall   methodcall;
    };


}  // namespace ulxr


#endif // ULXR_CALLPARSE_BASE_H

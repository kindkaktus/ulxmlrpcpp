/***************************************************************************
         ulxr_signature.cpp  -  handle method signatures
                             -------------------
    begin                : Sat Mar 23 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_signature.cpp 10942 2011-09-13 14:35:52Z korosteleva $

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

#include <ulxmlrpcpp/ulxr_signature.h>
#include <ulxmlrpcpp/ulxr_value.h>

namespace ulxr {


    Signature::Signature()
    {
    }


    Signature::Signature(const Void & /* v */)
    {
    }


    Signature::Signature(const std::string &s)
    {
        sig = s;
    }


    Signature::Signature(const ValueBase &v)
    {
        sig = v.getSignature();
    }


    Signature & Signature::addParam(const Value &v)
    {
        if (sig.length() != 0)
            sig += ",";
        sig += v.getSignature();
        return *this;
    }


    Signature & Signature::addParam(const std::string &s)
    {
        if (sig.length() != 0)
            sig += ",";
        sig += s;
        return *this;
    }


    std::string Signature::getString() const
    {
        return sig;
    }


    Signature & Signature::operator<<(const Value &v)
    {
        addParam(v);
        return *this;
    }


    Signature & Signature::operator<<(const std::string &s)
    {
        addParam(s);
        return *this;
    }

}  // namespace ulxr

/**************************************************************************
               ulxr_valueparse_base.cpp  -  parse rpc primitive values
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_valueparse_base.cpp 10933 2011-09-09 09:44:29Z korosteleva $

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

//#define ULXR_DEBUG_OUTPUT


#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_valueparse_base.h>


namespace ulxr {


    ValueParserBase::~ValueParserBase()
    {
    }


    Value ValueParserBase::getValue() const
    {
        ULXR_TRACE("ValueParserBase::getValue()");
        Value *v = getTopValueState()->getValue();
        if (v != 0)
            return *v;
        else
            return Value(); // return ulxr::Void()
    }


//////////////////////////////////////////////////////////////////////////////
//


    ValueParserBase::ValueState::ValueState (unsigned st)
        : XmlParserBase::ParserState(st)
        , value (0)
        , candel(true)
    {
    }


    void ValueParserBase::ValueState::takeValue(Value *val, bool del)
    {
        ULXR_TRACE("ValueParserBase::ValueState::takeValue(Value *)");
        value = val;
        candel = del;
    }


    bool ValueParserBase::ValueState::canDelete() const
    {
        return candel;
    }


    std::string ValueParserBase::ValueState::getStateName() const
    {
        switch (getParserState())
        {
        case XmlParserBase::eNone     : return "eNone";
        case ValueParserBase::eValue  : return "eValue";
        case ValueParserBase::eArray  : return "eArray";
        case ValueParserBase::eData   : return "eData";
        case ValueParserBase::eStruct : return "eStruct";
        case ValueParserBase::eMember : return "eMember";
        case ValueParserBase::eName   : return "eName";
        case ValueParserBase::eBoolean: return "eBoolean";
        case ValueParserBase::eInt    : return "eInt";
        case ValueParserBase::eI4     : return "eI4";
        case ValueParserBase::eDouble : return "eDouble";
        case ValueParserBase::eString : return "eString";
        case ValueParserBase::eBase64 : return "eBase64";
        case ValueParserBase::eDate   : return "eDate";

        case ValueParserBase::eValueParserLast: return "eValueParserLast";
        }

        return "eUnknown";
    }


    void ValueParserBase::ValueState::takeName(const std::string & /* name */ )
    {
    }


    std::string ValueParserBase::ValueState::getName() const
    {
        return name;
    }


    void ValueParserBase::ValueState::setName(const std::string &nm)
    {
        name = nm;
    }


    Value* ValueParserBase::ValueState::getValue() const
    {
        return value;
    }


//////////////////////////////////////////////////////////////////////////////
//


    ValueParserBase::MemberState::MemberState(unsigned st, Value *val)
        : ValueState(st),
          mem_val(0)
    {
        value = val;
    }


    void ValueParserBase::MemberState::takeValue(Value *v, bool del)
    {
        candel = del;
        ULXR_TRACE("ValueParserBase::MemberState::takeValue(Value *)");
        if (getName().length() != 0) // wait for name, maybe second tag
        {
            value->getStruct()->addMember(getName(), *v);
            delete v;
        }
        else
            mem_val = v;
    }


    void ValueParserBase::MemberState::takeName(const std::string &n)
    {
        if (mem_val != 0)
        {
            value->getStruct()->addMember(n, *mem_val);
            delete mem_val;
            mem_val = 0;
        }
        else
            setName(n);
    }


//////////////////////////////////////////////////////////////////////////////
//

    ValueParserBase::ArrayState::ArrayState(unsigned st)
        : ValueState(st)
    {
    }


    void ValueParserBase::ArrayState::takeValue(Value *v, bool del)
    {
        candel = del;
        ULXR_TRACE("ValueParserBase::ArrayState::takeValue(Value *)");
        if (value == 0)
            value = new Value(Array());

        value->getArray()->addItem(*v);
        delete v;
    }


}  // namespace ulxr


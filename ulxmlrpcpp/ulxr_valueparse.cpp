/**************************************************************************
               ulxr_valueparse.cpp  -  parse xml-rpc primitive values
                             -------------------
    begin                : Wed Mar 13 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_valueparse.cpp 10933 2011-09-09 09:44:29Z korosteleva $

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

#include <memory>
#include <cstring>

#include <ulxmlrpcpp/ulxr_valueparse.h>
#include <ulxmlrpcpp/ulxr_except.h>

#ifdef DEBUG
#include <iostream>
#endif


namespace ulxr {


    ValueParser::ValueParser()
        : ValueParserBase()
    {
        ULXR_TRACE("ValueParser::ValueParser()");
        states.push(new ValueState(eNone));
    }


    ValueParser::~ValueParser()
    {
        ULXR_TRACE("ValueParser::~ValueParser()");

        while (states.size() != 0)
        {
            if (getTopValueState()->canDelete())
                delete getTopValueState()->getValue();
            delete getTopValueState();
            states.pop();
        }
    }



    ValueParserBase::ValueState* ValueParser::getTopValueState() const
    {
        ULXR_TRACE("ValueParser::getTopState() size: " << states.size());
#ifdef DEBUG
        ValueState *vs = dynamic_cast<ValueState*> (states.top());      // be careful about type
        if (vs == 0)
        {
            ULXR_TRACE("ValueParser::getTopState(), top state: " << (void*) states.top());
            ULXR_TRACE("ValueParser::getTopState(): state <> ValueState");
            ULXR_TRACE("ValueParser::getTopState(): state == " << typeid(states.top()).name());
        }
        return vs;
#else
        return reinterpret_cast<ValueParserBase::ValueState*> (states.top());  // dont care anymore
#endif
    }


    void
    ValueParser::startElement(const XML_Char* name, const XML_Char** atts)
    {
        ULXR_TRACE("ValueParser::startElement(const XML_Char*, const char**)");
        if (!testStartElement(name, atts))
            XmlParser::testStartElement(name, atts);
    }


    bool
    ValueParser::testStartElement(const XML_Char* name, const XML_Char** /* atts */)
    {
        ULXR_TRACE("ValueParser::testStartElement(const XML_Char*, const char**)"
                   << "\n  name: "
                   << name
                  );
        switch(getTopValueState()->getParserState() )
        {
        case eNone:
            if (strcmp(name, "value") == 0)
                states.push(new ValueState(eValue));
            else
                return false;
            break;

        case eValue:
            if (strcmp(name, "array") == 0)
                states.push(new ValueState(eArray));

            else if (strcmp(name, "struct") == 0)
                states.push(new ValueState(eStruct));

            else if (strcmp(name, "boolean") == 0)
                states.push(new ValueState(eBoolean));

            else if (strcmp(name, "int") == 0)
                states.push(new ValueState(eInt));

            else if (strcmp(name, "i4") == 0)
                states.push(new ValueState(eI4));

            else if (strcmp(name, "double") == 0)
                states.push(new ValueState(eDouble));

            else if (strcmp(name, "string") == 0)
                states.push(new ValueState(eString));

            else if (strcmp(name, "base64") == 0)
                states.push(new ValueState(eBase64));

            else if (strcmp(name, "dateTime.iso8601") == 0)
                states.push(new ValueState(eDate));

            else
                return false;
            break;

        case eStruct:
            if (strcmp(name, "member") == 0)
            {
                if (getTopValueState()->getValue() == 0)  // first closing member adds struct-item
                    getTopValueState()->takeValue(new Value(Struct()), false);
                states.push(new MemberState(eMember, getTopValueState()->getValue()));
            }
            else
                return false;
            break;

        case eMember:
            if (strcmp(name, "name") == 0)
                states.push(new ValueState(eName));

            else if (strcmp(name, "value") == 0)
                states.push(new ValueState(eValue));

            else
                return false;

            break;

        case eArray:
            if (strcmp(name, "data") == 0)
                states.push(new ArrayState(eData));
            else
                return false;
            break;

        case eData:
            if (strcmp(name, "value") == 0)  // closing value adds data-items
                states.push(new ValueState(eValue));
            else
                return false;
            break;

        default:
            return false;
        }

        return true;
    }


    void ValueParser::endElement(const XML_Char *name)
    {
        ULXR_TRACE("ValueParser::endElement(const XML_Char*)");
        if (!testEndElement(name))
            XmlParser::testEndElement(name);
    }


    bool ValueParser::testEndElement(const XML_Char *name)
    {
        ULXR_TRACE("ValueParser::testEndElement(const XML_Char*)");

        if (states.size() <= 1)
            throw RuntimeException(ApplicationError, "abnormal program behaviour: ValueParser::testEndElement() had no states left");

        std::auto_ptr<ValueState> curr(getTopValueState());
        states.pop();
        states.top()->setPrevParserState(curr->getParserState());
        switch(curr->getParserState() )
        {
        case eBoolean:
            assertEndElement(name, "boolean");
            getTopValueState()->takeValue(new Value(Boolean(curr->getCharData())) );
            break;

        case eInt:
            assertEndElement(name, "int");
            getTopValueState()->takeValue(new Value(Integer(curr->getCharData())) );
            break;

        case eI4:
            assertEndElement(name, "i4");
            getTopValueState()->takeValue(new Value(Integer(curr->getCharData())) );
            break;

        case eDouble:
            assertEndElement(name, "double");
            getTopValueState()->takeValue(new Value(Double(curr->getCharData())) );
            break;

        case eString:
            assertEndElement(name, "string");
            getTopValueState()->takeValue(new Value(RpcString(curr->getCharData())) );
            break;

        case eBase64:
        {
            assertEndElement(name, "base64");
            Base64 b64;
            b64.setBase64(curr->getCharData()); // move raw data!
            getTopValueState()->takeValue(new Value(b64));
        }
        break;

        case eDate:
            assertEndElement(name , "dateTime.iso8601");
            getTopValueState()->takeValue(new Value(DateTime(curr->getCharData())) );
            break;

        case eMember:
            assertEndElement(name, "member");
            getTopValueState()->takeValue (curr->getValue());
            break;

        case eName:
            assertEndElement(name, "name");
            getTopValueState()->takeName((curr->getCharData()) );
            break;

        case eValue:
            assertEndElement(name, "value");
            if (curr->getValue() == 0)     // special case
            {
                if(curr->getPrevParserState() == eArray)            // not empty Array
                    getTopValueState()->takeValue (new Value(Array()));

                else if (curr->getPrevParserState() == eStruct)     // not empty Struct
                    getTopValueState()->takeValue (new Value(Struct()));

                else                                                // no type tag defaults to string
                    getTopValueState()->takeValue (new Value(RpcString(curr->getCharData())));
            }
            else
                getTopValueState()->takeValue (curr->getValue());
            break;

        case eStruct:
            assertEndElement(name, "struct");
            getTopValueState()->takeValue (curr->getValue());
            break;

        case eArray:
            assertEndElement(name, "array");
            getTopValueState()->takeValue (curr->getValue());
            break;

        case eData:
            assertEndElement(name, "data");
            getTopValueState()->takeValue (curr->getValue());
            break;

        default:
            states.push(curr.release());
            return false;
        }

        return true;
    }


}  // namespace ulxr



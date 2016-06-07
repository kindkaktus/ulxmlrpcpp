/***************************************************************************
                  ulxr_value.h  -  values passed by xml-rpc
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_value.cpp 10942 2011-09-13 14:35:52Z korosteleva $

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

#include <cstdio>
#include <ctime>
#include <vector>
#include <iostream>
#include <cstdlib>

#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>


#define ULXR_ASSERT_RPCTYPE(x) \
  if (x != getType() )    \
  { \
    throw ParameterException(ApplicationError, (std::string) \
                           "Value type mismatch.\n" \
                           "Expected: " + #x + ".\n" \
                           "Actually have: " +getTypeName() + "."); \
  }


namespace ulxr {



//////////////////////////////////////////////////////


    bool Value::isVoid() const
    {
        return baseVal->isVoid();
    }


    bool Value::isBoolean() const
    {
        return baseVal->isBoolean();
    }


    bool Value::isInteger() const
    {
        return baseVal->isInteger();
    }


    bool Value::isDouble()   const
    {
        return baseVal->isDouble();
    }


    bool Value::isArray() const
    {
        return baseVal->isArray();
    }


    bool Value::isStruct() const
    {
        return baseVal->isStruct();
    }


    bool Value::isString() const
    {
        return baseVal->isString();
    }


    bool Value::isBase64() const
    {
        return baseVal->isBase64();
    }


    bool Value::isDateTime() const
    {
        return baseVal->isDateTime();
    }


    ValueType Value::getType() const
    {
        return baseVal->getType();
    }


    std::string Value::getTypeName() const
    {
        return baseVal->getTypeName();
    }


    std::string Value::getSignature(bool deep) const
    {
        std::string s;
        if (baseVal != 0)
            s = baseVal->getSignature(deep);
        return s;
    }


    std::string Value::getXml(int indent) const
    {
        std::string ret;
        if (baseVal != 0)
            ret = baseVal->getXml(indent);
        return ret;
    }


    Struct* Value::getStruct()
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        return structVal;
    };


    const Struct* Value::getStruct() const
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        return structVal;
    };


    Array* Value::getArray()
    {
        ULXR_ASSERT_RPCTYPE(RpcArray);
        return arrayVal;
    };


    const Array* Value::getArray() const
    {
        ULXR_ASSERT_RPCTYPE(RpcArray);
        return arrayVal;
    };


    Value::operator Boolean& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcBoolean);
        return *boolVal;
    }


    Value::operator const Boolean& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcBoolean);
        return *boolVal;
    }


    Value::operator Integer& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcInteger);
        return *intVal;
    }


    Value::operator const Integer& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcInteger);
        return *intVal;
    }


    Value::operator Double& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcDouble);
        return *doubleVal;
    }


    Value::operator const Double& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcDouble);
        return *doubleVal;
    }


    Value::operator Array& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcArray);
        return *arrayVal;
    }


    Value::operator const Array& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcArray);
        return *arrayVal;
    }


    Value::operator Struct& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        return *structVal;
    }


    Value::operator const Struct& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        return *structVal;
    }


    Value::operator RpcString& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcStrType);
        return *stringVal;
    }


    Value::operator const RpcString& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcStrType);
        return *stringVal;
    }


    Value::operator Base64& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcBase64);
        return *base64Val;
    }


    Value::operator const Base64& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcBase64);
        return *base64Val;
    }


    Value::operator DateTime& ()
    {
        ULXR_ASSERT_RPCTYPE(RpcDateTime);
        return *dateVal;
    }


    Value::operator const DateTime& () const
    {
        ULXR_ASSERT_RPCTYPE(RpcDateTime);
        return *dateVal;
    }


    Value::Value ()
    {
        voidVal = new Void();
    }


    Value::Value (const Value &val)
    {
        baseVal = val.baseVal->cloneValue();
    }


    Value::Value (const Boolean &val)
    {
        boolVal = new Boolean(val);
    }


    Value::Value (const Integer &val)
    {
        intVal = new Integer(val);
    }


    Value::Value (const Double &val)
    {
        doubleVal = new Double(val);
    }


    Value::Value (const Array &val)
    {
        arrayVal = new Array(val);
    }


    Value::Value (const Struct &val)
    {
        structVal = new Struct(val);
    }


    Value::Value (const RpcString &val)
    {
        stringVal = new RpcString(val);
    }


    Value::Value (const Base64 &val)
    {
        base64Val = new Base64(val);
    }



    Value::Value (const DateTime &val)
    {
        dateVal = new DateTime(val);
    }



    Value::Value (const char *val)
    {
        stringVal = new RpcString(val);
    }

    Value::Value (const double &val)
    {
        doubleVal = new Double(val);
    }


    Value::Value (const int &val)
    {
        intVal = new Integer(val);
    }


    Value::Value (const bool &val)
    {
        boolVal = new Boolean(val);
    }


    Value& Value::operator= (const Value &val)
    {
        ValueBase *temp = val.baseVal->cloneValue();
        delete baseVal;
        baseVal = temp;
        return *this;
    }


    Value::~Value ()

    {
        delete baseVal;
        baseVal = 0;
    }


//////////////////////////////////////////////////////


    ValueBase::ValueBase (ValueType t)
        : type(t)
    {
    }


    ValueType ValueBase::getType() const
    {
        return type;
    }


    ValueBase::~ValueBase ()
    {
    }


    bool ValueBase::isVoid() const
    {
        return type == RpcVoid;
    }


    bool ValueBase::isBoolean() const
    {
        return type == RpcBoolean;
    }


    bool ValueBase::isInteger() const
    {
        return type == RpcInteger;
    }


    bool ValueBase::isDouble() const
    {
        return type == RpcDouble;
    }


    bool ValueBase::isArray() const
    {
        return type == RpcArray;
    }


    bool ValueBase::isStruct() const
    {
        return type == RpcStruct;
    }


    bool ValueBase::isString() const
    {
        return type == RpcStrType;
    }


    bool ValueBase::isBase64() const
    {
        return type == RpcBase64;
    }


    bool ValueBase::isDateTime() const
    {
        return type == RpcDateTime;
    }


    std::string ValueBase::getTypeName() const
    {
        std::string ret = "RpcValue";
        switch(getType() )
        {
        case RpcVoid:
            ret = "RpcVoid";
            break;

        case RpcInteger:
            ret = "RpcInteger";
            break;

        case RpcDouble:
            ret = "RpcDouble";
            break;

        case RpcBoolean:
            ret = "RpcBoolean";
            break;

        case RpcStrType:
            ret = "RpcString";
            break;

        case RpcDateTime:
            ret = "RpcDateTime";
            break;

        case RpcBase64:
            ret = "RpcBase64";
            break;

        case RpcArray:
            ret = "RpcArray";
            break;

        case RpcStruct:
            ret = "RpcStruct";
            break;
        }
        return ret;
    }


//////////////////////////////////////////////////////


    Void::Void ()
        : ValueBase(RpcVoid)
    {
    }


    Void::~Void ()
    {
    }


    ValueBase * Void::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcVoid);
        return new Void(*this);
    }


    std::string Void::getSignature(bool /*deep*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcVoid);
        return getValueName();
    }


    std::string Void::getValueName()
    {
        return "void";
    }

    std::string Void::getXml(int /*indent*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcVoid);
        return "";
    }


//////////////////////////////////////////////////////


    Boolean::Boolean ()
        : ValueBase(RpcBoolean)
    {
    }


    Boolean::Boolean (bool b)
        : ValueBase(RpcBoolean), val(b)
    {
    }


    Boolean::~Boolean ()
    {
    }


    Boolean::Boolean (const std::string &s)
        : ValueBase(RpcBoolean)
    {
        if (   s == "true" || s == "TRUE"
                || s == "1" || s == "on" || s == "ON")
            val = true;
        else
            val = false;
    }


    ValueBase * Boolean::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcBoolean);
        return new Boolean(*this);
    }


    std::string Boolean::getSignature(bool /*deep*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcBoolean);
        return getValueName();
    }


    std::string Boolean::getValueName()
    {
        return "bool";
    }


    std::string Boolean::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcBoolean);
        std::string s = getXmlIndent(indent);
        s += "<value><boolean>";

        if (val)
            s += "1"; // "true"
        else
            s += "0"; // "false"

        s += "</boolean></value>";
        return s;
    }


    bool Boolean::getBoolean () const
    {
        ULXR_ASSERT_RPCTYPE(RpcBoolean);
        return val;
    }


    void Boolean::setBoolean(const bool newval)
    {
        ULXR_ASSERT_RPCTYPE(RpcBoolean);
        val = newval;
    }


//////////////////////////////////////////////////////


    Integer::Integer ()
        : ValueBase(RpcInteger)
    {
    }


    Integer::Integer (int i)
        : ValueBase(RpcInteger), val(i)
    {
    }


    Integer::~Integer ()
    {
    }


    Integer::Integer (const std::string &s)
        : ValueBase(RpcInteger)
    {
//  val = atoi(s.c_str() );
        char *endp;
        val = strtol(s.c_str(), &endp, 10 );
//  if (*endp != 0)
// FIXME
    }


    ValueBase * Integer::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcInteger);
        return new Integer(*this);
    }



    std::string Integer::getSignature(bool /*deep*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcInteger);
        return getValueName();
    }


    std::string Integer::getValueName()
    {
        return "int";
    }


    std::string Integer::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcInteger);
        std::string s = getXmlIndent(indent);
        s += "<value><i4>";

        char buff[100];
        if (snprintf(buff, sizeof(buff), "%d", val) >= (int) sizeof(buff))
            throw RuntimeException(ApplicationError, "Buffer for conversion too small in Integer::getXml() ");

        s += buff;
        s += "</i4></value>";
        return s;
    }


    int Integer::getInteger () const
    {
        ULXR_ASSERT_RPCTYPE(RpcInteger);
        return val;
    }


    void Integer::setInteger(const int newval)
    {
        ULXR_ASSERT_RPCTYPE(RpcInteger);
        val = newval;
    }


//////////////////////////////////////////////////////


    bool Double::scientific = false;


    Double::Double ()
        : ValueBase(RpcDouble)
    {
    }


    Double::Double (double d)
        : ValueBase(RpcDouble)
        , val(d)
    {
    }


    Double::~Double ()
    {
    }


    Double::Double (const std::string &s)
        : ValueBase(RpcDouble)
    {
        val = atof(s.c_str() );
    }


    ValueBase * Double::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcDouble);
        return new Double(*this);
    }


    std::string Double::getSignature(bool /*deep*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcDouble);
        return getValueName();
    }


    std::string Double::getValueName()
    {
        return "double";
    }


    bool Double::setScientificMode(bool in_scientific)
    {
        bool prev = scientific;
        scientific = in_scientific;
        return prev;
    }


    std::string Double::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcDouble);
        std::string s = getXmlIndent(indent);
        s += "<value><double>";
        char buff[1000];

        unsigned used;
        if (scientific)
            used = snprintf(buff, sizeof(buff), "%g", val);
        else
            used = snprintf(buff, sizeof(buff), "%f", val);

        if (used >= sizeof(buff))
            throw RuntimeException(ApplicationError,  "Buffer for conversion too small in Double::getXml ");

        s += buff;
        s += "</double></value>";
        return s;
    }


    double Double::getDouble () const
    {
        ULXR_ASSERT_RPCTYPE(RpcDouble);
        return val;
    }


    void Double::setDouble(const double newval)
    {
        ULXR_ASSERT_RPCTYPE(RpcDouble);
        val = newval;
    }


//////////////////////////////////////////////////////


    RpcString::RpcString ()
        : ValueBase(RpcStrType)
    {
    }


    RpcString::RpcString (const std::string &s)
        : ValueBase(RpcStrType)
    {
        setString(s);
    }


    RpcString::~RpcString ()
    {
    }

    ValueBase * RpcString::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcStrType);
        return new RpcString(*this);
    }


    std::string RpcString::getSignature(bool /*deep*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcStrType);
        return getValueName();
    }


    std::string RpcString::getValueName()
    {
        return "string";
    }


    std::string RpcString::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcStrType);
        std::string s = getXmlIndent(indent);
        s += "<value><string>";
        s += xmlEscape(val);
        s += "</string></value>";
        return s;
    }


    std::string RpcString::getString () const
    {
        ULXR_ASSERT_RPCTYPE(RpcStrType);
        return val;
    }


    void RpcString::setString(const std::string &newval)
    {
        ULXR_ASSERT_RPCTYPE(RpcStrType);
        val = newval;
    }

//////////////////////////////////////////////////////


    Base64::Base64 ()
        : ValueBase(RpcBase64)
    {
    }


    Base64::Base64 (const std::string &s)
        : ValueBase(RpcBase64)
    {
        setString(s);
    }


    Base64::~Base64 ()
    {
    }


    std::string Base64::getBase64() const
    {
        return val;
    }


    void Base64::setBase64(const std::string s)
    {
        val = s;
    }


    ValueBase * Base64::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcBase64);
        return new Base64(*this);
    }


    std::string Base64::getSignature(bool /*deep*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcBase64);
        return getValueName();
    }


    std::string Base64::getValueName()
    {
        return "base64";
    }


    std::string Base64::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcBase64);
        std::string s = getXmlIndent(indent);
        s += "<value><base64>";
        s += val;
        s += "</base64></value>";
        return s;
    }


    std::string Base64::getString () const
    {
        ULXR_ASSERT_RPCTYPE(RpcBase64);
        return vec2Str(fromBase64(val));
    }


    void Base64::setString(const std::string &newval)
    {
        ULXR_ASSERT_RPCTYPE(RpcBase64);
        val = toBase64(str2Vec<unsigned char>(newval));
    }


//////////////////////////////////////////////////////


    DateTime::DateTime ()
        : ValueBase(RpcDateTime)
    {
    }


    DateTime::DateTime (const std::string &s)
        : ValueBase(RpcDateTime)
        , val(s)
    {
    }


    DateTime::DateTime (const time_t &tm)
        : ValueBase(RpcDateTime)
    {
        setDateTime(tm);
    }


    DateTime::~DateTime ()
    {
    }


    ValueBase * DateTime::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcDateTime);
        return new DateTime(*this);
    }


    std::string DateTime::getSignature(bool /*deep*/) const
    {
        ULXR_ASSERT_RPCTYPE(RpcDateTime);
        return getValueName();
    }


    std::string DateTime::getValueName()
    {
        return "dateTime.iso8601";
    }


    std::string DateTime::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcDateTime);
        std::string s = getXmlIndent(indent);
        s += "<value><dateTime.iso8601>";
        s += val;
        s += "</dateTime.iso8601></value>";
        return s;
    }


    std::string DateTime::getDateTime () const
    {
        ULXR_ASSERT_RPCTYPE(RpcDateTime);
        return val;
    }


    namespace {

        static std::string padded(unsigned num)
        {
            std::string s;
            if (num < 10)
                s += L'0';
            s += toString(num);
            return s;
        }

    }


    void DateTime::setDateTime (const time_t &tmt,
                                bool add_dash /* =false */,
                                bool add_colon /* =true */)
    {
        std::string s;
        tm ltm;
        localtime_r(&tmt, &ltm);
        s += padded(ltm.tm_year+1900);
        if (add_dash)
            s += '-';
        s += padded(ltm.tm_mon+1);
        if (add_dash)
            s += '-';
        s += padded(ltm.tm_mday);
        s += "T";
        s += padded(ltm.tm_hour);
        if (add_colon)
            s += ':';
        s += padded(ltm.tm_min);
        if (add_colon)
            s += ':';
        s += padded(ltm.tm_sec);
        setDateTime(s);
    }


    void DateTime::setDateTime(const std::string &newval)
    {
        ULXR_ASSERT_RPCTYPE(RpcDateTime);
        val = newval;
    }


//////////////////////////////////////////////////////


    Array::Array ()
        : ValueBase(RpcArray)
    {
    }


    Array::~Array ()
    {
    }


    unsigned Array::size() const
    {
        return values.size();
    }


    void Array::clear()
    {
        values.clear();
    }


    ValueBase * Array::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcArray);
        return new Array(*this);
    }


    std::string Array::getSignature(bool deep) const
    {
        if (!deep)
            return getValueName();


        ULXR_ASSERT_RPCTYPE(RpcArray);
        std::string s;
        if (values.size() == 0)
            return "[]";

        s += '[';
        bool comma = values.size() >= 1;
        for (unsigned i = 0; i < values.size(); ++i)
        {
            if (comma && i != 0)
                s += ',';
            s += values[i].getSignature();
        }
        s += ']';

        return s;
    }


    std::string Array::getValueName()
    {
        return "array";
    }


    std::string Array::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcArray);
        std::string ind = getXmlIndent(indent);
        std::string ind1 = getXmlIndent(indent+1);
        std::string ind2 = getXmlIndent(indent+2);
        std::string s = ind + "<value>" + getXmlLinefeed();
        s += ind1 + "<array>" + getXmlLinefeed();
        s += ind2 + "<data>" + getXmlLinefeed();

        for (std::vector<Value>::const_iterator
                it = values.begin(); it != values.end(); ++it)
        {
            s += (*it).getXml(indent+3) + getXmlLinefeed();
        }

        s += ind2 + "</data>" + getXmlLinefeed();
        s += ind1 + "</array>"+ getXmlLinefeed();
        s += ind + "</value>";
        return s;
    }


    void Array::addItem(const Value &item)
    {
        values.push_back(item);
    }


    void Array::setItem(unsigned idx, const Value &item)
    {
        if (idx < values.size() )
            values[idx] = item;
    }


    Value Array::getItem(unsigned idx) const
    {
        if (idx < values.size() )
            return values[idx];
        throw Exception(ApplicationError, "Array index is out of range");
    }


//////////////////////////////////////////////////////


    Struct::Member::Member(const std::string &str_, const Value &t_)
        : str(str_)
        , t(t_)
    {
    }


    const std::string & Struct::Member::getName() const
    {
        return str;
    }


    const Value & Struct::Member::getValue() const
    {
        return t;
    }


//////////////////////////////////////////////////////


    Struct::Struct ()
        : ValueBase(RpcStruct)
    {
    }


    Struct::~Struct ()
    {
    }


    ValueBase * Struct::cloneValue() const
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        return new Struct(*this);
    }


    void Struct::clear()
    {
        val.clear();
    }


    unsigned Struct::size() const
    {
        return val.size() ;
    }


    const Struct::Members& Struct::getAllMembers() const
    {
        return val;
    }


    std::string Struct::getSignature(bool deep) const
    {
        if (!deep)
            return getValueName();

        ULXR_ASSERT_RPCTYPE(RpcStruct);
        std::string s;
        if (val.size() == 0)
            return "{}";

        if (val.size() > 1)
            s += '{';

        for (Members::const_iterator it = val.begin(); it != val.end(); ++it)
        {
            s += '{';
            s += (*it).first;
            s += ',';
            s += (*it).second.getSignature();
            s += '}';
        }

        if (val.size() > 1)
            s += '}';

        return s;
    }


    std::string Struct::getValueName()
    {
        return "struct";
    }


    std::string Struct::getXml(int indent) const
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        std::string ind = getXmlIndent(indent);
        std::string ind1 = getXmlIndent(indent+1);
        std::string ind2 = getXmlIndent(indent+2);
        std::string ind3 = getXmlIndent(indent+3);
        std::string s = ind + "<value>"+ getXmlLinefeed();
        s += ind1 + "<struct>" + getXmlLinefeed();

        for (Members::const_iterator it = val.begin(); it != val.end(); ++it)
        {
            s += ind2 + "<member>" + getXmlLinefeed();
            s += ind3 + "<name>" + (*it).first + "</name>" + getXmlLinefeed();
            s += (*it).second.getXml(indent+3) + getXmlLinefeed();
            s += ind2 + "</member>" + getXmlLinefeed();
        }

        s += ind1 + "</struct>" + getXmlLinefeed();
        s += ind + "</value>";
        return s;
    }


    void Struct::addMember(const std::string &name, const Value &item)
    {
        ULXR_TRACE("Struct::addMember(string, Value)");
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        val.insert(Member_pair(name, item));
    }


    bool Struct::hasMember(const std::string &name) const
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        return val.find(name) != val.end();
    }


    Value Struct::getMember(const std::string &name) const
    {
        ULXR_ASSERT_RPCTYPE(RpcStruct);
        Members::const_iterator it = val.find(name);
        if (it  == val.end())
            throw RuntimeException(ApplicationError, "Attempt to get unknown Struct member: " +name);

        return (*it).second;
    }


    std::vector<std::string> Struct::getMemberNames() const
    {
        std::vector<std::string> names;
        for (Members::const_iterator it = val.begin(); it != val.end(); ++it)
            names.push_back((*it).first);
        return names;
    }


    Struct::Member operator<< (const std::string &str, const Value &t)
    {
        ULXR_TRACE("Struct::Member operator<< (string, T)");
        return Struct::Member(str, t);
    }

    Struct::Member make_member (const std::string &str, const Value &t)
    {
        ULXR_TRACE("Member operator<< (string, T)");
        return Struct::Member(str, t);
    }


    Struct & operator<< (Struct &st, const Struct::Member &k)
    {
        ULXR_TRACE("operator<<(Struct, Struct::Member)");
        st.addMember(k.getName(), k.getValue());
        return st;
    }


};  // namespace ulxr


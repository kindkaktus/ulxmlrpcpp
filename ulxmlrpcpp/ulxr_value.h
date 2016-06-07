/***************************************************************************
                  ulxr_value.h  -  values passed by xml-rpc
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_value.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_VALUE_H
#define ULXR_VALUE_H


#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <map>
#include <vector>
#include <ctime>


namespace ulxr {


    /** @brief A value type which is used in an XML-RPC call or response.
      * @defgroup grp_ulxr_value_type Value types in an XML-RPC call
      */
    typedef enum
    {
        RpcInteger, RpcDouble,   RpcBoolean,
        RpcStrType, RpcDateTime, RpcBase64,
        RpcArray,   RpcStruct,   RpcVoid
    }
    ValueType;


    class Void;
    class Boolean;
    class Integer;
    class Double;
    class Array;
    class Struct;
    class RpcString;
    class Base64;
    class DateTime;
    class ValueBase;


    /** Abstraction of an XML RPC parameter.
      * Parameters are passed to and returned by method calls.
      */
    class  Value
    {
    public:

        /** Creates an empty Value
          */
        Value ();

        /** Creates a Value as duplicate of another Value.
          * @param  val  the source value
          */
        Value (const Value &val);

        /** Creates a value from a Boolean
          * @param  val  the source value
          */
        Value (const Boolean &val);

        /** Creates a value from an Integer
          * @param  val  the source value
          */
        Value (const Integer &val);

        /** Creates a value from a Double
          * @param  val  the source value
          */
        Value (const Double &val);

        /** Creates a value from an Array
          * @param  val  the source value
          */
        Value (const Array &val);

        /** Creates a value from a Struct
          * @param  val  the source value
          */
        Value (const Struct &val);

        /** Creates a value from a RpcString
          * @param  val  the source value
          */
        Value (const RpcString &val);

        /** Creates a value from a Base64
          * @param  val  the source value
          */
        Value (const Base64 &val);

        /** Creates a value from a DateTime
          * @param  val  the source value
          */
        Value (const DateTime &val);

        /** Creates a value from a const char*
          * @param  val  the source value
          */
        Value (const char *val);

        /** Creates a value from a const wchar_t*
          * @param  val  the source value
          */
        Value (const wchar_t *val);

        /** Creates a value from a double
          * @param  val  the source value
          */
        Value (const double &val);

        /** Creates a value from an int
          * @param  val  the source value
          */
        Value (const int &val);

        /** Creates a value from a bool
          * @param  val  the source value
          */
        Value (const bool &val);


        /** Destroys the Value
          */
        virtual ~Value();

        /** Creates a Value as copy of another Value.
          * @param  val  the source value
          */
        Value &operator=(const Value &val);

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        std::string getSignature(bool deep = false) const;

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        std::string getXml(int indent = 0) const;

        /** Converts the Value into a Void.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator Void& ();

        /** Converts the Value into a Void.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const Void& () const;

        /** Converts the Value into a Boolean.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator Boolean& ();
        /** Converts the Value into a Boolean.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const Boolean& () const;

        /** Converts the Value into an Integer.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator Integer& ();

        /** Converts the Value into an Integer.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const Integer& () const;

        /** Converts the Value into a Double.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator Double& ();

        /** Converts the Value into a Double.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const Double& () const;

        /** Converts the Value into an Array.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator Array& ();

        /** Converts the Value into an Array.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const Array& () const;

        /** Converts the Value into a Struct.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator Struct& ();

        /** Converts the Value into a Struct.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const Struct& () const;

        /** Converts the Value into a RpcString.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator RpcString& ();

        /** Converts the Value into a RpcString.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const RpcString& () const;

        /** Converts the Value into a Base64.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator Base64& ();

        /** Converts the Value into a Base64.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const Base64& () const;

        /** Converts the Value into a DateTime.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator DateTime& ();

        /** Converts the Value into a DateTime.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        operator const DateTime& () const;

        /** Converts the Value into a Struct.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return a pointer to the converted Value.
          */
        Struct* getStruct();

        /** Converts the Value into a Struct.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return a pointer to the converted Value.
          */
        const Struct* getStruct() const;

        /** Converts the Value into an Array.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        Array* getArray();

        /** Converts the Value into an Array.
          * If the type does not match exactly a RuntimeException is thrown.
          * @return the converted Value.
          */
        const Array* getArray() const;

        /** Determines if the Value is of type Boolean.
          * @return true if the type is correct.
          */
        bool isBoolean() const;

        /** Determines if the Value is of type Void.
          * @return true if the type is correct.
          */
        bool isVoid() const;

        /** Determines if the Value is of type Integer.
          * @return true if the type is correct.
          */
        bool isInteger() const;

        /** Determines if the Value is of type Double.
          * @return true if the type is correct.
          */
        bool isDouble() const;

        /** Determines if the Value is of type Array.
          * @return true if the type is correct.
          */
        bool isArray() const;

        /** Determines if the Value is of type Struct.
          * @return true if the type is correct.
          */
        bool isStruct() const;

        /** Determines if the Value is of type RpcString.
          * @return true if the type is correct.
          */
        bool isString() const;

        /** Determines if the Value is of type Base64.
          * @return true if the type is correct.
          */
        bool isBase64() const;

        /** Determines if the Value is of type DateTime.
          * @return true if the type is correct.
          */
        bool isDateTime() const;

        /** Returns the type of the Value.
          * @return value type
          */
        ValueType getType() const;

        /** Returns the C++-name of the ValueType.
          * @return type name
          */
        std::string getTypeName() const;

    private:

        union
        {
            ValueBase *baseVal;
            Void      *voidVal;

            Boolean   *boolVal;
            Integer   *intVal;
            Double    *doubleVal;
            Array     *arrayVal;
            Struct    *structVal;
            RpcString *stringVal;
            Base64    *base64Val;
            DateTime  *dateVal;
        };
    };


//////////////////////////////////////////////////////////////


    /** Base class of XML RPC parameters.
      * There exists several specialized parameter classes for number
      * and string types and collections of parameters.
      */
    class  ValueBase
    {
    public:

        /** Creates an empty ValueBase.
          * @param t  type of the Value.
          */
        ValueBase (ValueType t);

        /** Destroys the ValueBase
          */
        virtual ~ValueBase ();

        /** Determines if the Value is of type Void.
          * @return true if the type is correct.
          */
        bool isVoid() const;

        /** Determines if the Value is of type Boolean.
          * @return true if the type is correct.
          */
        bool isBoolean() const;

        /** Determines if the Value is of type Integer.
          * @return true if the type is correct.
          */
        bool isInteger() const;

        /** Determines if the Value is of type Double.
          * @return true if the type is correct.
          */
        bool isDouble() const;

        /** Determines if the Value is of type Array.
          * @return true if the type is correct.
          */
        bool isArray() const;

        /** Determines if the Value is of type Struct.
          * @return true if the type is correct.
          */
        bool isStruct() const;

        /** Determines if the Value is of type RpcString.
          * @return true if the type is correct.
          */
        bool isString() const;

        /** Determines if the Value is of type Base64.
          * @return true if the type is correct.
          */
        bool isBase64() const;

        /** Determines if the Value is of type DateTime.
          * @return true if the type is correct.
          */
        bool isDateTime() const;

        /** Returns the type of the Value.
          * @return value type
          */
        ValueType getType() const;

        /** Creates a copy of the actual Value.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const = 0;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const = 0;

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const = 0;

        /** Returns the C++-name of the ValueType.
          * @return type name
          */
        std::string getTypeName() const;

    private:

        ValueType type;

        /** Constructor for an empty ValueBase.
          * Exists as private method to prevent its use.
          */
        ValueBase ();
    };


//////////////////////////////////////////////////////////////


    /** Implementation of a void value.
      * @attention The void type is only a helper class and not
      *            intended for direct use.
      */
    class  Void : public ValueBase
    {
    public:

        /** Default constructor for a Void.
          */

        Void ();

        /** Destroys the Value
          */
        virtual ~Void ();

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

    private:

        bool val;
    };


//////////////////////////////////////////////////////////////


    /** Implementation of a boolean value.
      * Boolean values can either be "true" or "false".
      * @ingroup grp_ulxr_value_type
      */
    class  Boolean : public ValueBase
    {
    public:

        /** Default constructor for a Boolean.
          */

        Boolean ();

        /** Constructs a Boolean.
          * @param b  value for the construction
          */
        Boolean (bool b);

        /** Constructs a Boolean.
          * @param s  value as string (true, on and 1 count as TRUE)
          */
        Boolean (const std::string &s);

        /** Destroys the Value
          */
        virtual ~Boolean ();

        /** Returns the current value.
          * @return current value
          */
        bool getBoolean () const;

        /** Sets the current value.
          * @param newval  new value
          */
        void setBoolean(const bool newval);

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
         * The content and its type name is converted to an xml text.
         * The structure of the text is indented to facilitate easy reading
         * of larger blocks.
         * @param  indent   current indentation level
         * @return  The xml content
         */
        virtual std::string getXml(int indent = 0) const;

    private:

        bool val;
    };


//////////////////////////////////////////////////////////////


    /** Implementation of a integer value.
      * Integer values have 32 or more bits and are signed.
      * @ingroup grp_ulxr_value_type
      */
    class  Integer : public ValueBase
    {
    public:

        /** Default constructor for an Integer.
          */
        Integer ();

        /** Constructs an Integer.
          * @param  i  value for the construction
          */
        Integer (int i);

        /** Constructs an Integer.
          * @param  s  value for the construction as number string
          */
        Integer (const std::string &s);

        /** Destroys the Value
          */
        virtual ~Integer ();

        /** Returns the current value.
          * @return current value
          */
        int getInteger () const;

        /** Sets a new content.
          * @param  newval  the new content
          */
        void setInteger(const int newval);

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

    private:

        int val;
    };


//////////////////////////////////////////////////////////////


    /** Implementation of a floating point value.
      * The range of double values is system dependent
      * as far as I know but should at least cover
      * a "double" in C.
      * @ingroup grp_ulxr_value_type
      */
    class  Double : public ValueBase
    {
    public:

        /** Default constructor for a Double.
          */
        Double ();

        /** Constructs a Double.
          * @param  d  value for the construction
          */
        Double (double d);

        /** Constructs a Double.
          * @param  s  value for the construction as number string
          */
        Double (const std::string &s);

        /** Destroys the Value
          */
        virtual ~Double ();

        /** Returns the current value.
          * @return current value
          */
        double getDouble () const;

        /** Sets a new content.
          * @param  newval  the new content
          */
        void setDouble(const double newval);

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

        /** Switches the conversion mode for double values.
          * Large numbers create large strings for the number without necessarily increasing
          * the accuracy. For that reason it is possible to switch to scientific mode
          * which uses only the significant 15 or 16 digits.
          * @li A floating point value using only digits and a decimal point like
          *     1234567.890123. This is the default.
          * @li Scientific mode with mantissa and exponent like 123.4567E12.
          * @param   scientific   true: use scientifiy mode like 123.456E-78
          * @return the previous setting
          */
        static bool setScientificMode(bool scientific);

    private:

        double val;

        static bool scientific;
    };


//////////////////////////////////////////////////////////////


    /** Implementation of a parameter collection.
      * An Array can contain an arbitrary amount of values
      * of any type here mentioned, even nested arrays.
      * You access the elements by their index.
      * @ingroup grp_ulxr_value_type
      */
    class  Array : public ValueBase
    {
    public:

        /** Default constructor for an empty Array.
          */
        Array ();

        /** Destroys the Value
          */
        virtual ~Array ();

        /** Appends a new array member.
          * @param  item    the new Value
          */
        void addItem(const Value &item);

        /** Sets a new content of an array member.
          * Counting starts with "0".
          * @param  idx     index of the desired Value
          * @param  item    the new Value
          */
        void setItem(unsigned idx, const Value &item);

        /** Returns a the content of an array member.
          * Counting starts with "0".
          * @param  idx     index of the desired Value
          * @throw Exception if array index is out of range
          */
        Value getItem(unsigned idx) const;

        /** Returns the number of array members.
          * @return the array size
          */
        unsigned size() const;

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature may consist of the concatenated type names of all
          * elements sourrounded by brackets.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

        /** Removes all elements of the Array.
          */
        void clear();

    private:

        std::vector<Value> values;
    };


    /** Helper function template to "stream" an item into an array.
      */
    template<class T>
    Array& operator<< (Array& array, const T& t)
    {
        array.addItem(t);
        return array;
    }

//////////////////////////////////////////////////////////////


    /** Implementation of a "named" collection.
      * A Struct can contain an arbitrary amount of values
      * of any type here mentioned, even nested Structs.
      * Unlike arrays you access their values by their member names
      * and not by their index.
      * @ingroup grp_ulxr_value_type
      */
    class  Struct : public ValueBase
    {
    public:

        class Member;

        /** Default constructor for an empty Struct.
          */
        Struct ();

        /** Destroys the Value
          */
        virtual ~Struct ();

        /** Adds a name/Value pair.
          * @param  name the name of the member
          * @param  item its content
          */
        void addMember(const std::string &name, const Value &item);

        /** Gets a list of all member names.
          * @return vector with all member names
          */
        std::vector<std::string> getMemberNames() const;

        /** Returns the Value of a member.
          * @param  name the name of the member
          * @return the members Value
          */
        Value getMember(const std::string &name) const;

        /** Tests if a member exists.
          * @param  name the name of the member
          * @return true, if the member exists
          */
        bool hasMember(const std::string &name) const;

        /** Returns the number of array members.
          * @return the array size
          */
        unsigned size() const;

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature may consist of the concatenated name/element pairs of all
          * elements sourrounded by braces. The element pairs are grouped
          * by braces as well.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

        /** Returns the value as C++ structure declaration.
         * @param  name the declaration name
         * @return  The C++ source
         */
        std::string  dump_C_decl(const char *name) const;

        /** Removes all elements of the Struct.
          */
        void clear();

    protected:

        typedef std::map<std::string, Value>  Members;

        /** Returns all elements of the Struct.
          */
        const Members & getAllMembers() const;

    private:

        typedef std::pair<std::string, Value> Member_pair;

        Members val;
    };


    /** Implementation of a key-value pair.
      * This is used as temporary type when adding members to
      * Structs
      */
    class  Struct::Member
    {
    public:

        /** Constructs the member.
          * @param  str    the name of the member
          * @param  val    the members Value
          */
        Member(const std::string &str, const Value &val);

        /** Returns the member name.
          * @return the name of the member
          */
        const std::string &getName() const;

        /** Returns the member value.
          * @return t    the members Value
          */
        const Value &getValue() const;

    private:

        std::string  str;
        Value      t;

    };

//////////////////////////////////////////////////////////////


    Struct::Member  operator<< (const std::string &str, const Value &t);
    Struct::Member  make_member (const std::string &str, const Value &t);
    Struct  & operator<< (Struct &st, const Struct::Member &k);


//////////////////////////////////////////////////////////////


    /** Implementation of a string value.
      * The content is usually what you might call "text" and always
      * encoded in Unicode to avoid misinterpretation
      * due to unknown and variing charsets. The internal storage format is either
      * directly unicode (std::wstring) or UTF8 when compiled with 8bit characters in
      * non-unicode mode.
      *
      * @attention You must convert from your local encodings, for example with libiconv.
      * @ingroup grp_ulxr_value_type
      */
    class  RpcString : public ValueBase
    {
    public:

        /** Default constructor for an RpcString.
          */
        RpcString ();

        /** Constructs an RpcString.
          * @param  s  value for the construction as UTF8 string
          */
        RpcString (const std::string &s);

        /** Destroys the Value
          */
        virtual ~RpcString ();


        /** Sets a new content.
          * @param  newval  the new content in UTF8
          */
        void setString(const std::string &newval);

        /** Returns the current value encoded in UTF8.
          * @return current value
          */
        std::string getString () const;


        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

    private:

        std::string val;
    };


//////////////////////////////////////////////////////////////


    /** Implementation of a date and time value.
      * The format is according to ISO8601 and must currently be done
      * on the users side as this class only provides transportation.
      *
      * Though XmlRpc uses the ISO8601 specification, not all detail are intended
      * to be supported. Time zones, milliseconds, abbreviations and mybe more are
      * omited. Only the 17 bytes from a value like the following should be considered
      * implemented:
      *  @li \c 19980717T14:08:55.
      * @ingroup grp_ulxr_value_type
      */
    class  DateTime : public ValueBase
    {
    public:

        /** Default constructor for a DateTime.
          */
        DateTime ();

        /** Constructs a DateTime.
          * @param s value for the construction as string
          */
        DateTime (const std::string &s);

        /** Constructs a DateTime. Uses defaults from \c setDateTime as
          * as decribed in the example in the XMLRPC spec.
          * @param tm value for the construction as time value
          */
        DateTime (const time_t &tm);

        /** Destroys the Value
          */
        virtual ~DateTime ();

        /** Returns the current value.
          * @return current value
          */
        std::string getDateTime () const;

        /** Sets a new content.
          * @param  newval  the new content
          */
        void setDateTime(const std::string &newval);

        /** Sets a new content from a std::time_t. Since the XMLRPC spec says nothing
          * special about the format there is an option to add "-" and ":" to the date
          * string as defined by ISO8601. The default is to omit dashes and add colons
          * as is decribed in the example in the XMLRPC spec: 19980717T14:08:55.
          * @param tm        value for the construction as time value
          * @param add_dash  add the dashes in the date part
          * @param add_colon add the colons in the time part
          * @param tm value for the construction as time value
          */
        void setDateTime (const time_t &tm,
                          bool add_dash = false,
                          bool add_colon = true);

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

    private:

        std::string val;
    };


//////////////////////////////////////////////////////////////


    /** Implementation of a base64 value.
      * This class is intended to be used for content in raw binary data.
      * To transport such data you must convert to something conforming to
      * XML rules. This is achived with base64 that uses only letters, digits
      * and some punctuation characters.
      * @ingroup grp_ulxr_value_type
      */
    class  Base64 : public ValueBase
    {
    public:

        /** Default constructor for a Base64.
          */
        Base64 ();

        /** Constructs a Base64.
          * @param s value for the construction as string
          */
        Base64 (const std::string &s);

        /** Destroys the Value
          */
        virtual ~Base64 ();

        /** Returns the current value.
          * @return current value
          */
        std::string getString () const;

        /** Returns the current value encoded in base64.
          * @return internal value
          */
        std::string getBase64() const;

        /** Sets the current value encoded in base64.
          * @param s already encode value.
          */
        void setBase64(const std::string s);

        /** Sets a new content.
          * @param  newval  the new content (UTF8 in non-unicode mode)
          */
        void setString(const std::string &newval);

        /** Creates a copy of the actual object.
          * @return pointer to the copy
          */
        virtual ValueBase* cloneValue() const;

        /** Returns the signature of value.
          * The signature consists of its type name.
          * @param deep  if nested types exist, include them
          * @return  The signature
          */
        virtual std::string getSignature(bool deep = false) const;

        /** Returns the xml name of the Value.
          * @return value name
          */
        static std::string getValueName();

        /** Returns the value as xml string.
          * The content and its type name is converted to an xml text.
          * The structure of the text is indented to facilitate easy reading
          * of larger blocks.
          * @param  indent   current indentation level
          * @return  The xml content
          */
        virtual std::string getXml(int indent = 0) const;

    private:

        std::string val;
    };


}  // namespace ulxr



#endif // ULXR_VALUE_H


/***************************************************************************
                 ulxr_expatwrap.cpp  -  c++ wrapper for expat
                             -------------------
    begin                : Thu Apr 30 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_expatwrap.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_EXPATWRAP_H
#define ULXR_EXPATWRAP_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_xmlparse_base.h>


namespace ulxr {

    /** A wrapper for expat
      * @ingroup grp_ulxr_parser
      */
    class  ExpatWrapper : public XmlParserBase
    {
    public:

        /** Constructs an expat  parser.
          * @param createParser  create a new parser instance
          */
        ExpatWrapper(bool createParser=true);

        /** Destroys the parser.
          */
        virtual ~ExpatWrapper();

        /** Parse a pice of xml data.
          * @param buffer   pointer start of next data chunk
          * @param len      len of this chunk
          * @param isFinal  true: last call to parser
          * @return error condition, 0 = ok
          */
        virtual int parse(const char* buffer, int len, int isFinal);

        /** Gets the code for the current error.
          * @return error code
          */
        virtual unsigned getErrorCode() const;

        /** Gets the description for an error code
          * @param code  error code
          * @return  pointer to description
          */
        virtual std::string getErrorString(unsigned code) const;

        /** Gets the line number in the xml data
          * @return  line number
          */
        virtual int getCurrentLineNumber() const;

        /** Maps expat error codes to xml-rpc error codes.
          * @param  xpatcode   error code from expat
          * @return  the according xml-rpc error
          */
        virtual int mapToFaultCode(int xpatcode) const;

    protected:

        /** Gets the parser instance.
          */
        operator XML_Parser() const;

        /** C++ callback for an opening XML tag.
          * Used ONLY internally as callback from expat.
          * @param  name  the name of the current tag
          * @param  atts  pointer to the current attributs (unused in XML-RPC)
          */
        virtual void startElement(const XML_Char* name, const XML_Char** atts);

        /** C++ callback for a closing XML tag.
          * Used ONLY internally as callback from expat.
          * @param  name  the name of the current tag
          */
        virtual void endElement(const XML_Char *name);

        /** C++ callback for regular text from expat.
          * Used ONLY internally.
          * @param  s        starting buffer with more data
          * @param  len      lenth of buffer
          */
        virtual void charData(const XML_Char *s, int len);

        /** C-style callback for a closing XML tag from expat.
          * Used ONLY internally.
          * @param  userData pointer to the actual C++-object
          * @param  name     the name of the current tag
          * @param  atts     to the current attributs (unused in XML-RPC)
          */
        static void startElementCallback(void *userData, const XML_Char* name, const XML_Char** atts);

        /** C-style callback for an ending XML tag from expat.
          * Used ONLY internally.
          * @param  userData pointer to the actual C++-object
          * @param  name     the name of the current tag
          */
        static void endElementCallback(void *userData, const XML_Char* name);

        /** C-style callback for regular text from expat.
          * Used ONLY internally.
          * @param  userData pointer to the actual C++-object
          * @param  s        starting buffer with more data
          * @param  len      lenth of buffer
          */
        static void charDataCallback(void *userData, const XML_Char* s, int len);

    protected:

        /** Resets the parser.
          */
        void resetParser();

    private:

        /** Sets the callback handlers.
          */
        void setHandler();

    private:

        XML_Parser expatParser;
    };


}  // namespace ulxr


#endif // ULXR_EXPATWRAP_H


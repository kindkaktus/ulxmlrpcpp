/***************************************************************************
               ulxr_valueparse.h  -  parse xml-rpc primitive values
                             -------------------
    begin                : Wed Mar 13 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_valueparse.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_VALUEPARSE_H
#define ULXR_VALUEPARSE_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_xmlparse.h>
#include <ulxmlrpcpp/ulxr_valueparse_base.h>

#include <stack>


namespace ulxr {


    /** Base class for XML RPC parsing.
      *
      * IMPORTANT:
      * The current "Value" is moved around via pointers and is not
      * automatically destroyed. The object taking over the "Value" resp. the object
      * storing the value somehow else must "delete" the "Value" it gets.
      *
      * @see ArrayState::takeValue
      * @see MethodCallParser::testEndElement
      * @ingroup grp_ulxr_parser
      */
    class  ValueParser : public ValueParserBase,
        public XmlParser
    {
    public:

        /** Constructs a parser.
          */
        ValueParser();

        /** Destroys the parser.
          */
        virtual ~ValueParser();

    protected:

        /** Parses the current opening XML tag.
          * Used ONLY internally as callback from expat.
          * @param  name  the name of the current tag
          * @param  atts  to the current attributs (unused in XML-RPC)
          */
        virtual void startElement(const XML_Char *name,
                                  const XML_Char **atts);

        /** Parses the current closing XML tag.
          * Used ONLY internally as callback from expat.
          * @param  name  the name of the current tag
          */
        virtual void endElement(const XML_Char* name);

        /** Tests if the current opening tag is to be parsed by this
          * inheritance level or by the parent.
          * Used ONLY internally.
          * @param  name  the name of the current tag
          * @param  atts  pointer to the current attributs (unused in XML-RPC)
          */
        bool testStartElement(const XML_Char *name, const XML_Char **atts);

        /** Tests if the current closing tag is to be parsed by this
          * inheritance level or by the parent.
          * Used ONLY internally.
          * @param  name  the name of the current tag
          */
        bool testEndElement(const XML_Char *name);

        /** Gets a pointer to the topmost ValueState.
          * @return pointer to ValueState
          */
        ValueState *getTopValueState() const;
    };


}  // namespace ulxr


#endif // ULXR_VALUEPARSE_H

/***************************************************************************
            ulxr_responseparse.h  -  parse xml-rpc method response
                             -------------------
    begin                : Wed Mar 13 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_responseparse.h 10942 2011-09-13 14:35:52Z korosteleva $

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

#ifndef ULXR_RESPONSEPARSE_H
#define ULXR_RESPONSEPARSE_H

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <ulxmlrpcpp/ulxr_valueparse.h>
#include <ulxmlrpcpp/ulxr_responseparse_base.h>


namespace ulxr {


    /** An xml parser for a MethodResponse.
      * @ingroup grp_ulxr_parser
      */
    class  MethodResponseParser : public ValueParser,
        public MethodResponseParserBase
    {
    public:

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
        bool testStartElement(const XML_Char *name,
                              const XML_Char **atts);

        /** Tests if the current closing tag is to be parsed by this
          * inheritance level or by the parent.
          * Used ONLY internally.
          * @param  name  the name of the current tag
          */
        bool testEndElement(const XML_Char *name);
    };


}  // namespace ulxr


#endif // ULXR_RESPONSEPARSE_H

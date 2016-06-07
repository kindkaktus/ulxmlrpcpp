/**************************************************************************
               ulxr_xmlparse.cpp  -  parse xml files
                             -------------------
    begin                : Thu Aug 26 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_xmlparse.cpp 10933 2011-09-09 09:44:29Z korosteleva $

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

#include <cstring>

#include <ulxmlrpcpp/ulxr_xmlparse.h>
#include <ulxmlrpcpp/ulxr_except.h>


namespace ulxr {


    XmlParser::XmlParser()
        : ExpatWrapper()
    {
    }


    void XmlParser::charData(const XML_Char *s, int len)
    {
        ULXR_TRACE("XmlParser::charData(const XML_Char*, int)"
                   /*             << "\n  len: " << len
                                << "\n  s: >>"
                                << std::string(s, len)
                                << "<<"
                   */
                  );
        states.top()->appendCharData(s, len);
        ULXR_TRACE("XmlParser::charData(const XML_Char*, int) finished");
    }


    bool XmlParser::testStartElement(const XML_Char *name, const XML_Char ** /*atts*/)
    {
        throw XmlException(NotWellformedError,
                           "Problem while parsing xml structure",
                           getCurrentLineNumber(),
                           "unexpected opening tag: "+std::string(name) );
    }


    bool XmlParser::testEndElement(const XML_Char *name)
    {
        throw XmlException(NotWellformedError,
                           "Problem while parsing xml structure",
                           getCurrentLineNumber(),
                           "unexpected closing tag: "+std::string(name) );
    }


    void XmlParser::assertEndElement(const char *current, const char *expected)
    {
        ULXR_TRACE("XmlParser::assertEndElement(const char*, const char*): "
                   << "\n curr: "
                   << current
                   << " exp: "
                   << expected
                  );
        if (0 != strcmp(current, expected))
            throw XmlException(NotWellformedError,
                               (std::string)"Unexpected xml tag: " + current
                               + ", wanted: " + expected,
                               getCurrentLineNumber(),
                               "Document not wellformed");
    }


}  // namespace ulxr


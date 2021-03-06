/***************************************************************************
                   ulxr_expatwrap.h  -  c++ wrapper for expat
                             -------------------
    begin                : Thu Apr 30 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_expatwrap.cpp 10939 2011-09-12 13:22:25Z korosteleva $

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
#include <ulxmlrpcpp/ulxr_expatwrap.h>


namespace ulxr {


    ExpatWrapper::ExpatWrapper(bool createParser)
        : XmlParserBase()
    {
        if (createParser)
        {
            expatParser = ::XML_ParserCreate(0);
            setHandler();
        }
    }


    ExpatWrapper::~ExpatWrapper()
    {
        if (expatParser)  // allows subclasses to avoid finishing parsing
            ::XML_ParserFree(expatParser);
    }


    void ExpatWrapper::setHandler()
    {
        ::XML_SetUserData(expatParser, this);
        ::XML_SetElementHandler(expatParser, startElementCallback, endElementCallback);
        ::XML_SetCharacterDataHandler(expatParser, charDataCallback);
    }


    void ExpatWrapper::resetParser()
    {
        ::XML_ParserReset(expatParser, 0);
        setHandler();
    }


    void ExpatWrapper::startElement(const XML_Char*, const XML_Char**)
    {
    }


    void ExpatWrapper::endElement(const XML_Char*)
    {
    }


    void ExpatWrapper::charData(const XML_Char*, int )
    {
    }


    int ExpatWrapper::mapToFaultCode(int xpatcode) const
    {
        int fc = NotWellformedError;
        // try to map error codes
        switch (xpatcode)
        {
        case XML_ERROR_UNKNOWN_ENCODING:
            fc = UnsupportedEncodingError;
            break;

        case XML_ERROR_BAD_CHAR_REF:
        case XML_ERROR_PARTIAL_CHAR:
        case XML_ERROR_INCORRECT_ENCODING: // fallthrough
            fc = InvalidCharacterError;
            break;

        default:
            fc = NotWellformedError;
        }
        return fc;
    }


    void
    ExpatWrapper::startElementCallback(void *userData,
                                       const XML_Char* name,
                                       const XML_Char** atts)
    {
        ((ExpatWrapper*)userData)->startElement(name, atts);
    }


    void
    ExpatWrapper::endElementCallback(void *userData, const XML_Char* name)
    {
        ((ExpatWrapper*)userData)->endElement(name);
    }


    void
    ExpatWrapper::charDataCallback(void *userData, const XML_Char* s, int len)
    {
        ((ExpatWrapper*)userData)->charData(s, len);
    }


    int  ExpatWrapper::parse(const char* buffer, int len, int isFinal)
    {
        return ::XML_Parse(expatParser, buffer, len, isFinal);
    }


    unsigned  ExpatWrapper::getErrorCode() const
    {
        return ::XML_GetErrorCode(expatParser);
    }


    std::string  ExpatWrapper::getErrorString(unsigned code) const
    {
        return XML_ErrorString((XML_Error) code);
    }


    int  ExpatWrapper::getCurrentLineNumber() const
    {
        return ::XML_GetCurrentLineNumber(expatParser);
    }


}  // namespace ulxr

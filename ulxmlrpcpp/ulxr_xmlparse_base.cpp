/**************************************************************************
                   ulxr_xmlparse_base.cpp  -  parse xml files
                             -------------------
    begin                : Fri Jan 09 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_xmlparse_base.cpp 10933 2011-09-09 09:44:29Z korosteleva $

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
#include <ulxmlrpcpp/ulxr_xmlparse_base.h>


namespace ulxr {


    XmlParserBase::XmlParserBase()
    {
        ULXR_TRACE("XmlParserBase::XmlParserBase()");
        complete = false;
    }


    XmlParserBase::~XmlParserBase()
    {
    }


    void XmlParserBase::clearStates()
    {
        while (!states.empty())
        {
            delete states.top();
            states.pop();
        }
    }

    bool XmlParserBase::isComplete() const
    {
        return complete;
    }


    void XmlParserBase::setComplete(bool comp)
    {
        complete = comp;
    }


//////////////////////////////////////////////////////////////////////////
//

    XmlParserBase::ParserState::ParserState (unsigned st)
        : state(st)
        , prevstate(eUnknown)
    {
    }


    XmlParserBase::ParserState::~ParserState()
    {
    }


    unsigned XmlParserBase::ParserState::getParserState() const
    {
        return state;
    }


    unsigned XmlParserBase::ParserState::getPrevParserState() const
    {
        return prevstate;
    }


    void XmlParserBase::ParserState::setPrevParserState(unsigned prev)
    {
        prevstate = prev;
    }


    std::string XmlParserBase::ParserState::getStateName() const
    {
        return "eUnknown";
    }


    void XmlParserBase::ParserState::appendCharData(const std::string &/*s*/)
    {
        ULXR_TRACE("XmlParserBase::ParserState::appendCharData(const std::string &)");
    }


    void XmlParserBase::ParserState::appendCharData(const XML_Char *s, int len)
    {
        ULXR_TRACE("XmlParserBase::ParserState::appendCharData(const XML_Char *, int)");
        cdata.append(s, len);
    }


    std::string XmlParserBase::ParserState::getCharData() const
    {
        ULXR_TRACE("XmlParserBase::ParserState::getCharData()");
        return cdata;
    }


}  // namespace ulxr


/***************************************************************************
              ulxmlrpcpp.cpp  -  common stuff for xml-rpc project
                             -------------------
    begin                : Sam Apr 20 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxmlrpcpp.cpp 10942 2011-09-13 14:35:52Z korosteleva $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms odebug  true: compiled with DEBUGf the GNU Lesser General Public License as
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

#include <cctype>
#include <cerrno>
#include <sstream>
#include <iostream>
#include <iosfwd>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <string.h>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <openssl/bn.h>
#include <openssl/x509.h>


/** @brief Various general helper classes
  * @defgroup grp_ulxr_utilit General helper classes
  */

/** @brief Unicode converter functions
  * @defgroup grp_ulxr_unicode_cvt Unicode converter functions
  */

/** @brief Various low-level parser classes
  * @defgroup grp_ulxr_parser Low-level parser classes
  */

/** @brief XML-RPC handling
  * @defgroup grp_ulxr_rpc XML-RPC handling
  */

/** @brief HTTP and HTML handling
  * @defgroup grp_ulxr_http HTTP and HTML handling
  */


/** @mainpage

   \section abstract Abstract

   ulxmlrpcpp is a library to call methods on a remote server by using
   <a href="http://www.xmlrpc.com">XML-RPC</a>. It's main goal
   is to be as easy to use as possible but be fully compliant. It is object
   oriented and written in C++.

   \section docs_struct  Getting Started With The Documentation

   The following basic function groups are available in this documentation:
     - \ref grp_ulxr_value_type
     - \ref grp_ulxr_connection
     - \ref grp_ulxr_protocol
     - \ref grp_ulxr_rpc
     - \ref grp_ulxr_http
     - \ref grp_ulxr_utilit
     - \ref grp_ulxr_parser
     - \ref grp_ulxr_unicode_cvt
*/

namespace ulxr {


    void getVersion (int &major, int &minor, int &patch, bool &debug, std::string &/*info*/)
    {
        std::string s (ULXR_VERSION);
        std::string num;

        std::size_t pos = s.find('.');
        bool good = true;
        if (pos != std::string::npos)
        {
            num = s.substr(0, pos);
            if (num.length() == 0)
                good = false;
            major = atoi(num.c_str());
            s.erase(0, pos+1);
            pos = s.find('.');

            if (pos != std::string::npos)
            {
                num = s.substr(0, pos);
                if (num.length() == 0)
                    good = false;
                minor = atoi(num.c_str());
                s.erase(0, pos+1);

                if (s.length() == 0)
                    good = false;
                patch = atoi(s.c_str());
            }
            else
                good = false;
        }
        else
            good = false;

        if (!good)
        {
            major = -1;
            minor = -1;
            patch = -1;
        }

#ifdef DEBUG
        debug = true;
#else
        debug = false;
#endif
    }

    std::string stripWS(const std::string &s)
    {
        unsigned start = 0;
        while (start < s.length() && isspace(s[start]))
            ++start;

        unsigned end = s.length();
        while (end > start && isspace(s[end-1]))
            --end;

        return s.substr(start, end-start);
    }

    std::string toBase64(const std::vector<unsigned char>& anSrc, bool aSingleLine)
    {
        if (anSrc.empty())
            return "";
        BIO* myMemBio	  = BIO_new(BIO_s_mem());
        BIO* myBase64Bio  = BIO_new(BIO_f_base64());
        if (aSingleLine)
            BIO_set_flags(myBase64Bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* myBio        = BIO_push(myBase64Bio, myMemBio);
        int myWritten = BIO_write(myBio, &anSrc[0], (int)anSrc.size());
        if (myWritten < 0 || static_cast<size_t>(myWritten) < anSrc.size())
        {
            BIO_free_all(myBio);
            throw ParameterException(ApplicationError, "toBase64(): BIO_write failed.");
        }
        BIO_flush(myBio);
        BUF_MEM* myEncBuf;
        if (BIO_get_mem_ptr(myBio, &myEncBuf) < 0 || myEncBuf->length <= 0)
        {
            BIO_free_all(myBio);
            throw ParameterException(ApplicationError, "toBase64(): BIO_get_mem_ptr failed.");
        }
        size_t myLen = (myEncBuf->data[myEncBuf->length-1] == '\n') ?
                       static_cast<size_t>(myEncBuf->length-1) :
                       static_cast<size_t>(myEncBuf->length);
        std::string myRetVal(myEncBuf->data, myLen);
        BIO_free_all(myBio);
        return myRetVal;
    }

    std::vector<unsigned char> fromBase64(const std::string& anSrc, bool aSingleLine)
    {
        std::vector<char> mySrcVec(anSrc.begin(), anSrc.end());
        if (mySrcVec.empty() || mySrcVec.back() != '\n')
            mySrcVec.push_back('\n');
        BIO* myBase64Bio  = BIO_new(BIO_f_base64());
        if (aSingleLine)
            BIO_set_flags(myBase64Bio, BIO_FLAGS_BASE64_NO_NL);
        BIO* myMemBio = BIO_new_mem_buf(&mySrcVec[0], (int)mySrcVec.size());
        myMemBio = BIO_push(myBase64Bio, myMemBio);
        std::vector<unsigned char> myRetVal;
        while (true)
        {
            unsigned char myBuf[128] = {};
            int myRead = BIO_read(myMemBio, myBuf, sizeof(myBuf));
            if (myRead < 0)
            {
                BIO_free_all(myMemBio);
                throw ParameterException(ApplicationError, "fromBase64(): BIO_read failed.");
            }
            myRetVal.insert(myRetVal.end(), myBuf, myBuf + myRead);
            if (static_cast<unsigned int>(myRead) < sizeof(myBuf))
                break;
        }
        BIO_free_all(myMemBio);
        return myRetVal;
    }



    std::string xmlEscape(const std::string &str)
    {
        std::string ret;
        unsigned prev = 0;
        unsigned len = str.length();
        unsigned curs = 0;
        const char *pc = str.data();

        while (curs != len)
        {
            char c = *pc++;

            if (c == '&')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&amp;";
                prev = curs+1;
            }
            else if (c == '<')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&lt;";
                prev = curs+1;
            }
            else if (c == '>')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&gt;";
                prev = curs+1;
            }
            else if (c == '\'')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&apos;";
                prev = curs+1;
            }
            else if (c == '"')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&quot;";
                prev = curs+1;
            }
            else if (c == '\n')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&#xA;";
                prev = curs+1;
            }
            else if (c == '\r')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&#xD;";
                prev = curs+1;
            }
            else if (c == '\t')
            {
                ret += str.substr(prev, curs-prev);
                ret += "&#x9;";
                prev = curs+1;
            }


            ++curs;
        }
        ret += str.substr(prev, curs-prev);
        return ret;
    }


    namespace
    {

        std::string charRefDezCont ("0123456789");
        std::string charRefHexCont ("0123456789aAbBcCdDeEfF");

    }

    void makeLower( std::string &str)
    {
        for (unsigned i= 0; i < str.length(); ++i)
            str[i] = tolower(str[i]);
    }


    void makeUpper( std::string &str)
    {
        for (unsigned i= 0; i < str.length(); ++i)
            str[i] = toupper(str[i]);
    }


    std::string getLastErrorString(unsigned errornum)
    {
        std::stringstream ss;
        ss << strerror(errornum) << ". Error " << errornum;
        return ss.str();
    }

    namespace
    {
        bool pretty_xml  = false;
    }


    void enableXmlPrettyPrint(bool pretty)
    {
        pretty_xml = pretty;
    }

    namespace
    {
        static const std::string empty_LF = "";
        static const std::string normal_LF = "\n";
        static const std::string empty_Indent = "";
    }

    std::string getXmlLinefeed()
    {
        if (pretty_xml)
            return normal_LF;

        else
            return empty_LF;
    }


    std::string getXmlIndent(unsigned indent)
    {
        if (pretty_xml)
            return std::string(indent, ' ');

        else
            return empty_Indent;
    }


}  // namespace ulxr


/***************************************************************************
              ulxmlrpcpp.h  -  common stuff for xml-rpc project
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxmlrpcpp.h 10942 2011-09-13 14:35:52Z korosteleva $

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

/**
  * @libdoc ultra lightweight XML RPC library for C++
  *
  * @p ulxmlrpcpp is a lightweight library to perform calls on remote systems
  * using the xml protocol.
  */

#ifndef ULXR_H
#define ULXR_H

#define ULXR_PACKAGE  "ulxmlrpcpp"
#define ULXR_VERSION   "1.7.5p1"
#define ULXMLRPCPP_VERSION  ULXR_VERSION

// #define ULXR_DEBUG_OUTPUT
// #define ULXR_SHOW_TRACE

/* debugging: show http header contents. */
// #define ULXR_SHOW_HTTP

/* debugging: show each read characters. */
// #define ULXR_SHOW_READ

/* debugging: show each written character. */
// #define ULXR_SHOW_WRITE

/* debugging: show xml data. */
// #define ULXR_SHOW_XML


#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#if defined(ULXR_DEBUG_OUTPUT)

#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>

#  define ULXR_DTRACE(x)     { std::stringstream ss; ss << x; syslog(LOG_DEBUG, "%s", ss.str().c_str()); }// std::cerr << __FILE__ << "," << __LINE__ << ": " <<  x << "\n"; }
#  define ULXR_DOUT(x)      { std::stringstream ss; ss << x; syslog(LOG_DEBUG, "%s", ss.str().c_str()); } // std::cerr << x  << "\n"; }
#  define ULXR_DWRITE(buf, n) { ::write(2, buf, n); std::cerr << "\n";}

#else

# define ULXR_DWRITE(buf, n)
# define ULXR_DTRACE(x)
# define ULXR_DOUT(x)

#endif

/////////////////////////////////////////////////////////////
// quickly enable debug output without source changes

// trace output
#ifdef  ULXR_SHOW_TRACE
#define ULXR_TRACE(x)        ULXR_DTRACE(x)
#else
#define ULXR_TRACE(x)
#endif

// xml body
#ifdef  ULXR_SHOW_XML
#define ULXR_DOUT_XML(x)        ULXR_DOUT("xml:\n" << x)
#else
#define ULXR_DOUT_XML(x)
#endif

// http header
#ifdef  ULXR_SHOW_HTTP
#define ULXR_DOUT_HTTP(x)       ULXR_DOUT(x)
#else
#define ULXR_DOUT_HTTP(x)
#endif

// conn:write
#ifdef  ULXR_SHOW_WRITE
#define ULXR_DOUT_WRITE(x)      ULXR_DOUT(x)
#define ULXR_DWRITE_WRITE(p,l)  ULXR_DWRITE(p,l)
#else
#define ULXR_DOUT_WRITE(x)
#define ULXR_DWRITE_WRITE(p,l)
#endif

// conn:read
#ifdef  ULXR_SHOW_READ
#define ULXR_DOUT_READ(x)       ULXR_DOUT(x)
#define ULXR_DWRITE_READ(p,l)   ULXR_DWRITE(p,l)
#else
#define ULXR_DOUT_READ(x)
#define ULXR_DWRITE_READ(p,l)
#endif


/////////////////////////////////////////////////////////////
//

#define ULXR_RECV_BUFFER_SIZE  50    // keep rather small, otherwise two messages
// might be read as a single block
#define ULXR_SEND_BUFFER_SIZE  2000

namespace ulxr
{

    // officially reserved -32768 .. -32000

    // parse errors
    const int NotWellformedError           = -32700;
    const int UnsupportedEncodingError     = -32701;
    const int InvalidCharacterError        = -32702;

    // server errors
    const int NotConformingError           = -32600;
    const int MethodNotFoundError          = -32601;
    const int InvalidMethodParameterError  = -32602;
    const int InternalXmlRpcError          = -32603;

    // misc errors
    const int ApplicationError             = -32500;
    const int SystemError                  = -32400;
    const int TransportError               = -32300;

    /** Gets the various parts of the version number.
    * @param  major  major part
    * @param  minor  minor part
    * @param  patch  patch counter
    * @param  debug  true: compiled with DEBUG
    * @param  info   some human readable information
    */
    void getVersion (int &major, int &minor, int &patch, bool &debug, std::string &info);

    /** Removes trailing and leading white space.
    * @param  s  input string
    * @return  output string
    */
    std::string stripWS(const std::string &s);

    /**
      Encode to Base64 format

      @param[in] anSrc String which needs to be encoded
      @param[in] aSingleLine If true, mimics the behavior of 'openssl base64 -e -A' otherwise 'openssl base64 -e'
      @return Encoded string
      @throw std::exception
     */
    std::string toBase64(const std::vector<unsigned char>& anSrc, bool aSingleLine = false);

    /**
      Decode from Base64 format

      @param[in] anSrc String which needs to be decoded
      @param[in] aSingleLine If true, mimics the behavior of 'openssl base64 -d -A' otherwise 'openssl base64 -d'
      @return Decoded string
      @throw std::exception
     */
    std::vector<unsigned char> fromBase64(const std::string& anSrc, bool aSingleLine = false);

    /** Replaces dangerous sequences in xml strings.
    * Several characters have a special meaning in xml and must be
    * converted to not disturb parsing.
    * @param  str   the string to escape
    * @return the converted string
    */
    std::string xmlEscape(const std::string &str);

    /** Converts string into lowercase.
    * Since the C-library is used this is only valid for ascii characters
    * @param  str   pointer to the string to convert
    */
    void makeLower(std::string &str);

    /** Converts string into uppercase.
    * Since the C-library is used this is only valid for ascii characters
    * @param  str   pointer to the string to convert
    */
    void makeUpper(std::string &str);

    template <class T>
    std::string toString (T aNumber)
    {
        std::ostringstream myOs;
        myOs << aNumber;
        return myOs.str();
    }

    template <class T>
    std::string vec2Str(const std::vector<T>& aVec)
    {
        return std::string(aVec.begin(), aVec.end());
    }

    template <class T>
    std::vector<T> str2Vec(const std::string& aStr)
    {
        return std::vector<T>(aStr.begin(), aStr.end());
    }


    /** Returns a readable string about the cause of the last system error.
    * Simply calls nonreentrant \c std::strerror() in a thread safe mode.
    * @param  errornum   error number taken from \c errno
    * @return error message
    */
    std::string getLastErrorString(unsigned errornum);

    /** Enables pretty-print of xml repsponses for better readability
    * @param  pretty   true: add spaces and linefeeds for better readability
    */
    void enableXmlPrettyPrint(bool pretty);

    /** Gets a string with the line feed for an xml response
    * @return empty if pretty-print disabled
    * @return string for a regular line feed otherwise
    */
    std::string getXmlLinefeed();

    /** Gets a string with spaces for an indentation in an xml response
    * @param  indent   number of spaces to indent
    * @return empty if pretty-print disabled
    * @return string with \c indent spaces
    */
    std::string getXmlIndent(unsigned indent);

} // namespace ulxr


#endif // ULXR_H

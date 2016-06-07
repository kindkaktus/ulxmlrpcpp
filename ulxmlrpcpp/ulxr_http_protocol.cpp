/***************************************************************************
               ulxr_http_protocol.cpp  -  http prootocol
                             -------------------
    begin                : Mon May 3 2004
    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: ulxr_http_protocol.cpp 11073 2011-10-25 12:44:58Z korosteleva $

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

// #define ULXR_SHOW_TRACE
//#define ULXR_DEBUG_OUTPUT
//#define ULXR_SHOW_HTTP
// #define ULXR_SHOW_READ
// #define ULXR_SHOW_WRITE
// #define ULXR_SHOW_XML

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <sys/stat.h>

#include <cstring>

#include <ulxmlrpcpp/ulxmlrpcpp.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>
#include <ulxmlrpcpp/ulxr_except.h>
#include <ulxmlrpcpp/ulxr_response.h>
#include <ulxmlrpcpp/ulxr_call.h>

namespace ulxr
{

    struct HttpProtocol::PImpl
    {
        std::string    proxy_user;
        std::string    proxy_pass;
        std::string    useragent;
        std::string    header_firstline;
        std::string    header_buffer;
        std::string    hostname;
        unsigned     hostport;

        bool                              bAcceptcookies;
        std::map<std::string, std::string>    cookies;
        std::string                         serverCookie;
        std::string                         clientCookie;
        std::vector<std::string>            userTempFields;
        header_property                   headerprops;
    };


    HttpProtocol::HttpProtocol(const HttpProtocol &prot)
        : Protocol(prot)
        , pimpl(new PImpl)
    {
        *pimpl = *prot.pimpl;
    }



    HttpProtocol::HttpProtocol(Connection *conn, const std::string &hn, unsigned hp)
        : Protocol (conn)
        , pimpl(new PImpl)
    {
        pimpl->hostname = hn;
        pimpl->hostport = hp;
        ULXR_TRACE("HttpProtocol(conn, name, port)");
        init();
    }



    HttpProtocol::HttpProtocol(TcpIpConnection *conn)
        : Protocol (conn)
        , pimpl(new PImpl)
    {
        pimpl->hostname = conn->getPeerName();
        pimpl->hostport = conn->getPort();
        ULXR_TRACE("HttpProtocol(conn)");
        init();
    }


    HttpProtocol::~HttpProtocol()
    {
        ULXR_TRACE("~HttpProtocol");
        delete pimpl;
        pimpl = NULL;
    }

    void HttpProtocol::init()
    {
        ULXR_TRACE("HttpProtocol::init()");
        pimpl->headerprops.clear();
        pimpl->useragent = ULXR_PACKAGE + std::string("/") + ULXR_VERSION;
        pimpl->userTempFields.clear();
        pimpl->bAcceptcookies = false;
    }


    void HttpProtocol::clearHttpInfo()
    {
        ULXR_TRACE("clearHttpInfo");
        pimpl->header_firstline = "";
        pimpl->header_buffer = "";
        pimpl->headerprops.clear();
        pimpl->cookies.clear();
    }


    void HttpProtocol::resetConnection()
    {
        ULXR_TRACE("resetConnection");
        Protocol::resetConnection();
        clearHttpInfo();
    }


    void HttpProtocol::closeConnection()
    {
        ULXR_TRACE("closeConnection");
        Protocol::closeConnection();
    }


    std::string HttpProtocol::getHttpProperty(const std::string &in_name) const
    {
        ULXR_TRACE("getHttpProperty " << in_name);
        std::string name = in_name;
        makeLower(name);
        header_property::const_iterator it;

        if ((it = pimpl->headerprops.find(name)) == pimpl->headerprops.end() )
            throw ConnectionException(NotConformingError,
                                      "Http property field not available: "+name, 400);

        return (*it).second;
    }


    bool HttpProtocol::hasHttpProperty(const std::string &in_name) const
    {
        std::string name = in_name;
        makeLower(name);
        bool b = pimpl->headerprops.find(name) != pimpl->headerprops.end();
        ULXR_TRACE("hasHttpProperty: " << in_name << " " << b);
        return b;
    }


    void HttpProtocol::parseHeaderLine()
    {
        ULXR_TRACE("parseHeaderLine");

        if (pimpl->header_firstline.empty())
        {
            pimpl->header_firstline = pimpl->header_buffer;
            ULXR_DOUT_HTTP("firstline: <" << pimpl->header_firstline << ">");
        }
        else
        {
            std::string prop_name, prop_val;
            std::size_t pos = pimpl->header_buffer.find(':');
            if (pos == std::string::npos)
            {
                prop_name = pimpl->header_buffer;
                prop_val = "";
            }
            else
            {
                prop_name = pimpl->header_buffer.substr(0, pos);
                prop_val = pimpl->header_buffer.substr(pos+1);
            }

            makeLower(prop_name);
            prop_val = stripWS(prop_val);
            prop_name = stripWS(prop_name);
            pimpl->headerprops.insert(std::make_pair(prop_name, prop_val));

            if (pimpl->bAcceptcookies && (prop_name == "set-cookie"))  // distinguish between cookie / set-cookie?
                setCookie(prop_val);

            else if (pimpl->bAcceptcookies && (prop_name == "cookie"))
                setCookie(prop_val);

            ULXR_DOUT_HTTP("headerprop: <" << prop_name
                           << "> + <" << prop_val << "> ");
        }
        pimpl->header_buffer = "";
    }

    bool HttpProtocol::checkContinue()
    {
        ULXR_TRACE("checkContinue");
        std::string head_version;
        unsigned head_status = 500;
        std::string head_phrase;
        splitResponseHeaderLine(head_version, head_status, head_phrase);
        if (head_status == 100)
        {
            ULXR_TRACE("Ignoring header 100-Continue");
            setConnectionState(ConnStart);
            return true;
        }
        else
            return false;
    }


    Protocol::State HttpProtocol::connectionMachine(char * &buffer, long &len)
    {
        /*
          Each invocation of this state machine tries to parse one single
          http header line of the buffer. If the content of the buffer is too small
          (no linefeed found) the content is cached in an internal string
          and used the next time.
          buffer points to the beginning of the next line at return if
          a linefeed has been found. In the message body nothing is done.
        */
        ULXR_TRACE("connectionMachine with " << len << " bytes");
        if (len == 0 || buffer == 0)
            return getConnectionState();

        while (len > 0)
        {
            const unsigned state = getConnectionState();
            switch (state)
            {
            case ConnStart:
                setConnectionState(ConnHeaderLine);
                clearHttpInfo();
                break;

            case ConnPendingCR:
                if (*buffer == '\n') // CR+LF
                {
                    --len;
                    ++buffer;
                }

                if (pimpl->header_buffer.empty())
                    setConnectionState(ConnSwitchToBody);
                else
                    setConnectionState(ConnPendingHeaderLine);
                break;

            case ConnPendingHeaderLine:
                if (pimpl->header_buffer.empty())
                    setConnectionState(ConnSwitchToBody);

                else if (*buffer != ' ')  // continuation line of current header field?
                {
                    parseHeaderLine();
                    setConnectionState(ConnHeaderLine);
                }
                else
                    setConnectionState(ConnHeaderLine);
                break;

            case ConnHeaderLine:
//            ULXR_TRACE("ConnHeaderLine:");

                if (*buffer == '\r')
                    setConnectionState(ConnPendingCR);

                else if (*buffer == '\n')
                {
                    if (pimpl->header_buffer.empty())
                        setConnectionState(ConnSwitchToBody);
                    else
                        setConnectionState(ConnPendingHeaderLine);
                }

                else
                    pimpl->header_buffer += *buffer;

                ++buffer;
                --len;
                break;

            case ConnSwitchToBody:
                machine_switchToBody(buffer, len);
                break;

            case ConnBody:
                ULXR_TRACE("ConnBody:");
                return ConnBody;

            case ConnError:
                ULXR_TRACE("ConnError:");
                return ConnError;

            default:
                setConnectionState(ConnError);
                throw ConnectionException(SystemError, "connectionMachine(): unknown state", 500);
            }
        }

        if (getConnectionState() == ConnSwitchToBody)
            machine_switchToBody(buffer, len);

        ULXR_TRACE("/connectionMachine");

        return getConnectionState();
    }


    void HttpProtocol::machine_switchToBody(char * &buffer, long   &len)
    {
        ULXR_TRACE("ConnSwitchToBody:");
        if (!checkContinue())
        {
            if (hasHttpProperty("content-length"))
            {
                determineContentLength();

                ULXR_TRACE("content_length: " << getContentLength());
                ULXR_TRACE("len: " << len);

                if (getContentLength() >= 0)
                    setRemainingContentLength(getContentLength() - len);
            }
            setConnectionState(ConnBody);
        }
    }


    bool HttpProtocol::hasBytesToRead() const
    {
        return getRemainingContentLength() != 0;
    }


    void HttpProtocol::determineContentLength()
    {
        ULXR_TRACE("determineContentLength");

        header_property::iterator it;
        if ((it = pimpl->headerprops.find("content-length")) != pimpl->headerprops.end() )
        {
            ULXR_TRACE(" content-length: " << it->second);
            setContentLength(atoi(it->second.c_str()));
            ULXR_TRACE(" length: " << getContentLength());
        }
        else
        {
            throw ConnectionException(NotConformingError,  "Content-Length of message not available", 411);
        }

        setRemainingContentLength(getContentLength());
        ULXR_TRACE(" content_length: " << getContentLength());
    }


    void
    HttpProtocol::sendResponseHeader(int code,
                                     const std::string &phrase,
                                     const std::string &type,
                                     unsigned long len)
    {
        ULXR_TRACE("sendResponseHeader");
        char stat[40];
        snprintf(stat, sizeof(stat), "%d", code );

        char contlen[40];
        snprintf(contlen, sizeof(contlen), "%ld", len );

        std::string ps = phrase;

        std::size_t pos = 0;
        while ((pos = ps.find('\n', pos)) != std::string::npos)
        {
            ps.replace(pos, 1, " ");
            pos += 1;
        }

        pos = 0;
        while ((pos = ps.find('\r', pos)) != std::string::npos)
        {
            ps.replace(pos, 1, " ");
            pos += 1;
        }

        std::string http_str = std::string("HTTP/1.0 ") + stat + " " + ps + "\r\n";
        http_str += "Connection: Close\r\n";

        if (len != 0 && type.length() != 0)
            http_str  += "Content-Type: " + type + "\r\n";

        for (unsigned i = 0; i < pimpl->userTempFields.size(); ++i)
            http_str += pimpl->userTempFields[i] + "\r\n";
        pimpl->userTempFields.clear();

        if (hasServerCookie())
            http_str += "Set-Cookie: " + getServerCookie() + "\r\n";

        http_str += std::string("Content-Length: ") + contlen + "\r\n";
        http_str += "X-Powered-By: " + getUserAgent() + "\r\n"
                    + "Server: " + pimpl->hostname + "\r\n"
                    + "Date: " + getDateStr() + "\r\n";

        http_str += "\r\n";    // empty line at end of header

        ULXR_DOUT_HTTP("resp: \n" << http_str.c_str());
        writeRaw(http_str.data(), http_str.length());
    }


    void HttpProtocol::sendRequestHeader(const std::string &method,
                                         const std::string &in_resource,
                                         const std::string &type,
                                         unsigned long len)
    {
        ULXR_TRACE("sendRequestHeader");
        char contlen[40];
        snprintf(contlen, sizeof(contlen), "%ld", len );

        char ports[40];
        snprintf(ports, sizeof(ports), "%d", pimpl->hostport);
        std::string resource = "http://" + pimpl->hostname + ":" + ports + in_resource;
        std::string http_str = method + " " + resource + " HTTP/1.0\r\n";
        http_str += "Host: " + pimpl->hostname + "\r\n";

        http_str += "User-Agent: " + getUserAgent() + "\r\n";

        if (pimpl->proxy_user.length() + pimpl->proxy_pass.length() != 0)
            http_str += "Proxy-Authorization: Basic "
                        + toBase64(str2Vec<unsigned char>(pimpl->proxy_user + ":" + pimpl->proxy_pass));

        http_str += "Connection: Close\r\n";
        if (len != 0 && type.length() != 0)
            http_str += "Content-Type: " + type + "\r\n";

        for (unsigned i = 0; i < pimpl->userTempFields.size(); ++i)
            http_str += pimpl->userTempFields[i] + "\r\n";
        pimpl->userTempFields.clear();

        http_str += "Date: " + getDateStr() + "\r\n";
        http_str += std::string("Content-Length: ") + contlen + "\r\n";

        if (hasClientCookie())
            http_str += "Cookie: " + getClientCookie() + "\r\n";

        http_str += "\r\n";    // empty line at end of header

        ULXR_DOUT_HTTP("req: \n" << http_str.c_str());

        writeRaw(http_str.data(), http_str.length());
    };


    std::string HttpProtocol::getDateStr()
    {
        ULXR_TRACE("getDateStr");
        time_t tm = time(NULL);
        char buff[40];
        char * ct = ::ctime_r(&tm, buff);
        std::string s = ct;  // "\n" already included!

        s.erase(s.length()-1);              // remove it
        return s;
    }


    void
    HttpProtocol::sendNegativeResponse(int status,
                                       const std::string &phrase,
                                       const std::string &info)
    {
        ULXR_TRACE("sendNegativeResponse");

        char stat[40];
        snprintf(stat, sizeof(stat), "%d", status );

        std::string msg = std::string("<html>"
                                      "<head><title>Error occured</title></head>"
                                      "<body>"
                                      "<b>Sorry, error occured: ") + stat
                          + ", " + phrase;

        if (info.length() != 0)
            msg += "<br />" + info;

        msg += "</b>"
               "<hr /><p>"
               "This cute little server is powered by"
               " <a href=\"http://ulxmlrpcpp.sourceforge.net\">";

        msg += std::string(ULXR_PACKAGE)
               + "/v" + ULXR_VERSION
               + "</a>"
               + "</p>"
               "</body>"
               "</html>";

        sendResponseHeader(status, phrase, "text/html", msg.length());
        writeRaw(msg.data(), msg.length());
    }


    void HttpProtocol::sendRpcResponse(const MethodResponse &resp)
    {
        ULXR_TRACE("sendRpcResponse");

        std::string xml = resp.getXml(0)+"\n";
        ULXR_DOUT_XML(xml);

        sendResponseHeader(200, "OK", "text/xml", xml.length());
        writeBody(xml.data(), xml.length());
    }

    void HttpProtocol::writeBody(const char *data, unsigned long len)
    {
        writeRaw(data, len);
    }


    void HttpProtocol::sendRpcCall(const MethodCall &call,
                                   const std::string &resource)
    {
        ULXR_TRACE("sendRpcCall");

        std::string xml = call.getXml(0)+"\n";
        ULXR_DOUT_XML(xml);

        sendRequestHeader("POST", resource, "text/xml", xml.length());
        writeBody(xml.data(), xml.length());
        ULXR_TRACE("/sendRpcCall");
    };


    bool HttpProtocol::isResponseStatus200(std::string &phrase) const
    {
        ULXR_TRACE("isResponseStatus200");

        std::string s = stripWS(getFirstHeaderLine());
        if (s.empty())
        {
            s = "No connection status available";
            return false;
        }

        std::size_t pos = s.find(' ');
        if (pos != std::string::npos)  // skip version
            s.erase(0, pos+1);
        else
            s = "";

        std::string stat;
        s = stripWS(s);
        pos = s.find(' ');
        if (pos != std::string::npos)
        {
            stat = s.substr(0, pos);
            s.erase(0, pos+1);
        }
        else
        {
            stat = s;
            s = "";
        }

        phrase = stripWS(s);

        return stat == "200";
    }

    bool HttpProtocol::getUserPass(std::string &user, std::string &pass) const
    {
        ULXR_TRACE("getUserPass");
        user = "";
        pass = "";

        if (hasHttpProperty("authorization") )
        {
            std::string auth = getHttpProperty("authorization");

            ULXR_TRACE("getUserPass: " + auth);
            ULXR_TRACE("getUserPass: basic?");

            // only know basic auth
            std::string auth_id = auth.substr(0, 6);
            makeLower(auth_id);
            if (auth_id != "basic ")
                return false;

            auth.erase(0, 6);
            auth = vec2Str(fromBase64(auth));
            ULXR_TRACE("getUserPass: ':'? "  + auth);
            std::size_t pos = auth.find(':');
            if (pos != std::string::npos)
            {
                user = stripWS(auth.substr(0, pos));
                pass = stripWS(auth.substr(pos+1));
                ULXR_TRACE("getUserPass: user=" +user + ", pass="+pass);
                return true;
            }
        }

        return false;
    }


    void HttpProtocol::rejectAuthentication(const std::string &realm)
    {
        ULXR_TRACE("rejectAuthentication: " + realm);
        addOneTimeHttpField("WWW-Authenticate",
                            "Basic realm=\"" + realm +"\"");
        sendNegativeResponse(401, "Authentication required for realm \""+ realm + "\"");
    }


    void HttpProtocol::addOneTimeHttpField(const std::string &name, const std::string &value)
    {
        ULXR_TRACE("addOneTimeHttpField: " + name + ": " + value);
        pimpl->userTempFields.push_back(stripWS(name) + ": " + stripWS(value));
    }


    void HttpProtocol::setMessageAuthentication(const std::string &user,
            const std::string &pass)
    {
        ULXR_TRACE("setMessageAuthentication");
        std::string s = "Basic ";
        s += toBase64(str2Vec<unsigned char>(user + ":" + pass));
        addOneTimeHttpField("Authorization", s);
    }


    void HttpProtocol::setProxyAuthentication(const std::string &user,
            const std::string &pass)
    {
        ULXR_TRACE("setProxyAuthentication");
        pimpl->proxy_user = user;
        pimpl->proxy_pass = pass;
    }


    std::string HttpProtocol::getProtocolName()
    {
        return "http";
    }


    bool HttpProtocol::hasCookie() const
    {
        bool b = !pimpl->cookies.empty();
        ULXR_TRACE("hasCookie: " << b);
        return b;
    }


    void HttpProtocol::setCookie(const std::string &in_cont)
    {
        ULXR_TRACE("setCookie: " << in_cont);
        std::string cont = in_cont;
        std::size_t uEnd = cont.find(';');
        while (uEnd != std::string::npos)
        {
            std::string sKV = cont.substr(0, uEnd);
            cont.erase(0, uEnd+1);
            std::size_t uEq = sKV.find('=');
            if (uEq != std::string::npos)
            {
                std::string sKey = stripWS(sKV.substr(0, uEq));
                std::string sVal = stripWS(sKV.substr(uEq+1));
                ULXR_TRACE("setCookie: " << sKey << " " << sVal);
                pimpl->cookies[sKey] = sVal;
            }
            uEnd = cont.find(';');
        }

        std::size_t uEq = cont.find('=');
        if (uEq != std::string::npos)
        {
            std::string sKey = stripWS(cont.substr(0, uEq));
            std::string sVal = stripWS(cont.substr(uEq+1));
            ULXR_TRACE("setCookie: " << sKey << " " << sVal);
            pimpl->cookies[sKey] = sVal;
        }
    }


    std::string HttpProtocol::getCookie() const
    {
        std::string ret;
        for (std::map<std::string, std::string>::const_iterator iCookie = pimpl->cookies.begin();
                iCookie != pimpl->cookies.end();
                ++iCookie)
        {
            if (iCookie != pimpl->cookies.begin())
                ret += "; ";
            ret += (*iCookie).first + "=" + (*iCookie).second;
        }
        ULXR_TRACE("getCookie: " << ret);
        return ret;
    }


    void HttpProtocol::setAcceptCookies(bool bAccept)
    {
        ULXR_TRACE("setAcceptCookies: " << bAccept);
        pimpl->bAcceptcookies = bAccept;
    }


    bool HttpProtocol::isAcceptCookies() const
    {
        ULXR_TRACE("isAcceptCookies: " << pimpl->bAcceptcookies);
        return pimpl->bAcceptcookies;
    }


    void HttpProtocol::setServerCookie(const std::string &cookie)
    {
        ULXR_TRACE("setServerCookie: " << cookie);
        pimpl->serverCookie = cookie;
    }


    std::string HttpProtocol::getServerCookie() const
    {
        ULXR_TRACE("getServerCookie: " << pimpl->serverCookie);
        return pimpl->serverCookie;
    }


    bool HttpProtocol::hasServerCookie() const
    {
        bool b = pimpl->serverCookie.length() != 0;
        ULXR_TRACE("hasServerCookie: " << b);
        return b;
    }


    void HttpProtocol::setClientCookie(const std::string &cookie)
    {
        ULXR_TRACE("setClientCookie: " << cookie);
        pimpl->clientCookie = cookie;
    }


    std::string HttpProtocol::getClientCookie() const
    {
        ULXR_TRACE("getClientCookie: " << pimpl->clientCookie);
        return pimpl->clientCookie;
    }


    bool HttpProtocol::hasClientCookie() const
    {
        bool b = pimpl->clientCookie.length() != 0;
        ULXR_TRACE("hasClientCookie: " << b);
        return b;
    }


    void HttpProtocol::setUserAgent(const std::string &ua)
    {
        pimpl->useragent = ua;
    }


    std::string HttpProtocol::getUserAgent() const
    {
        return pimpl->useragent;
    }


    std::string HttpProtocol::getFirstHeaderLine() const
    {
        return pimpl->header_firstline;
    }


    void HttpProtocol::splitResponseHeaderLine(std::string &head_version, unsigned  &head_status, std::string &head_phrase)
    {
        head_version = "";
        head_status = 500;
        head_phrase = "Internal error";

        std::string s = stripWS(getFirstHeaderLine());
        std::size_t pos = s.find(' ');
        if (pos != std::string::npos)
        {
            head_version = s.substr(0, pos);
            s.erase(0, pos+1);
        }
        else
        {
            head_version = s;
            s = "";
        }
        pos = head_version.find('/');
        if (pos != std::string::npos)
            head_version.erase(0, pos+1);

        std::string stat;
        s = stripWS(s);
        pos = s.find(' ');
        if (pos != std::string::npos)
        {
            stat = s.substr(0, pos);
            s.erase(0, pos+1);
        }
        else
        {
            stat = s;
            s = "";
        }
        head_status = atoi(stat.c_str());

        s = stripWS(s);
        head_phrase = s;
    }

}  // namespace ulxr


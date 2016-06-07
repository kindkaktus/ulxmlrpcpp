/***************************************************************************
                   httpfunc.cpp  -  test suite for basic http

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: httpfunc.cpp 11054 2011-10-18 13:00:59Z korosteleva $

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

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_tcpip_connection.h>

#define NoSuccess { success = 1; std::cout << __LINE__ << std::endl; }

int main(int argc, char ** argv)
{
    ulxr::intializeLogger(argv[0]);

    ulxr::TcpIpConnection conn (false, "localhost", 32001);
    ulxr::HttpProtocol http (&conn, "123", 123);

    const char *http_msg =
        "HTTP/1.0  100 Continue\r\n"
        "Connection: Close\r\n"
        "Content-Type: xyz/abc\r\n"
        "\r\n"
        "HTTP/1.0 200 OK\r\n"
        "Connection: Close\r\n"
        "Content-Length: 42\r\n"
        "Content-Type: text/xml; encoding=UTF-8;\r\n"
        "\r\n"
        "12345678901234567890\r\n"
        "abcasdf8901234567890\r\n";

    const char *http_msg2 =
        "HTTP/1.0 200 OK\r\n"
        "Connection     : Close\r\n"
        "Host           : me.at.home\r\n"
        "Content-Length : 44\r\n"
        "Content-Type   : text/xml;\r\n"
        " encoding=UTF-8;\r\n"
        "\r\n"
        "kasdklfjasldkjfaksjf\r\n"
        "abcasdf8901234567890\r\n"
        "-->ready";

    const char *http_msg3 =
        "HTTP/1.0 200      OK\r\n"
        "Connection     :     Close\r\n"
        "Host           :     me.at.home\r\n"
        "Transfer-Encoding :  chunked\r\n"
        "Content-Length :  16\r\n"
        "Content-Type   :  text/xml;\r\n"
        " encoding=UTF-8;\r\n"
        "\r\n"
        "5\r\n"
        "kas\r\n\r\n"
        "8\r\n"
        "abc123\r\n\r\n"
        "3 ; more info\r\n"
        "k\r\n\r\n"
        "0\r\n"
        "footer1: text1\r\n"
        "footer2: text2\r\n"
        "\r\n";

    const char *http_msg4 =
        "HTTP/1.0 200      OK\r\n"
        "Connection     :      Close\r\n"
        "Host           :    me.at.home\r\n"
        "Transfer-Encoding : chunked\r\n"
        "Content-Type   : text/xml;\r\n"
        " encoding=UTF-8;\r\n"
        "\r\n"
        "5\r\n"
        "kas\r\n\r\n"
        "8\r\n"
        "abc123\r\n\r\n"
        "3 ; more info\r\n"
        "k\r\n\r\n"
        "0\r\n"
        "footer1: text1\r\n"
        "footer2: text2\r\n"
        "\r\n";

    int success = 0;

    try
    {
        {
            char *buff = (char*)http_msg;
            long buff_len = strlen(buff);
            http.resetConnection();
            http.connectionMachine(buff, buff_len);
            std::cout << "len: " << buff_len << std::endl;
            std::cout << "body:\n" << buff << std::endl;

            if (http.getHttpProperty("Content-Type") != "text/xml; encoding=UTF-8;")
                NoSuccess;

            std::string head_version;
            unsigned head_status = 500;
            std::string head_phrase;
            http.splitHeaderLine(head_version, head_status, head_phrase);

            std::cout << "content-type: " << http.getHttpProperty("Content-Type") << std::endl;
            std::cout << "head_phrase: " << head_phrase << std::endl;
            std::cout << "head_status: " << head_status << std::endl;

            if (head_status != 200)
                NoSuccess;

            if (head_phrase != "OK")
                NoSuccess;

            if (buff_len != 44)
                NoSuccess;
        }

        {
            char *buff = (char*)http_msg2;
            long buff_len = strlen(buff);
            http.resetConnection();
            http.connectionMachine(buff, buff_len);
            std::cout << "len2: " << buff_len << std::endl;
            std::cout << "body2:\n" << buff << std::endl;
        }

        {
            char *buff_start = strdup(http_msg3);
            const int buff_len = strlen(buff_start);
            http.resetConnection();
            if (!http.hasBytesToRead())
                NoSuccess;
            long ptr = 0;
            std::string body;

            while (ptr < buff_len)
            {
                const int bl = 1;
                long len = bl;
                char *buff = buff_start + ptr;
                ulxr::HttpProtocol::State st = http.connectionMachine(buff, len);
                if (ulxr::HttpProtocol::ConnBody == st)
                    body.append(buff, len);

                else if (ulxr::HttpProtocol::ConnSwitchToBody == st)
                {
                    if (!http.hasBytesToRead())
                        NoSuccess;
                }
                ptr += bl;
            }

            free(buff_start);
            std::string b = "kas\r\n" "abc123\r\n" "k\r\n";
            if (b != body)
                NoSuccess;
            if (http.hasBytesToRead())
                NoSuccess;
            if (http.getHttpProperty("footer1") != "text1")
                NoSuccess;
            if (http.getHttpProperty("footer2") != "text2")
                NoSuccess;
        }

        {
            char *buff_start = strdup(http_msg3);
            const int buff_len = strlen(buff_start);  // a multiple of 2
            if (buff_len % 2 != 0)
                NoSuccess;

            http.resetConnection();
            long ptr = 0;
            std::string body;

            while (ptr < buff_len)
            {
                const int bl = 2;
                long len = bl;
                char *buff = buff_start + ptr;
                if (ulxr::HttpProtocol::ConnBody == http.connectionMachine(buff, len))
                    body.append(buff, len);
                ptr += bl;
            }

            free(buff_start);
            std::string b = "kas\r\n" "abc123\r\n" "k\r\n";
            if (b != body)
                NoSuccess;
            if (http.hasBytesToRead())
                NoSuccess;
            if (http.getHttpProperty("footer2") != "text2")
                NoSuccess;
            if (http.getHttpProperty("footer1") != "text1")
                NoSuccess;
        }

        {
            char *buff_start = strdup(http_msg3);
            const int buff_len = strlen(buff_start);
            if (buff_len % 3 != 0)
                NoSuccess;
            http.resetConnection();
            long ptr = 0;
            std::string body;

            while (ptr < buff_len)
            {
                const int bl = 3;
                long len = bl;
                char *buff = buff_start + ptr;
                if (ulxr::HttpProtocol::ConnBody == http.connectionMachine(buff, len))
                    body.append(buff, len);
                ptr += bl;
            }

            free(buff_start);
            std::string b = "kas\r\n" "abc123\r\n" "k\r\n";
            if (b != body)
                NoSuccess;
            if (http.hasBytesToRead())
                NoSuccess;
            if (http.getHttpProperty("footer1") != "text1")
                NoSuccess;
            if (http.getHttpProperty("footer2") != "text2")
                NoSuccess;
        }

        {
            char *buff_start = strdup(http_msg3);
            const int buff_len = strlen(buff_start);
            if (buff_len % 4 != 0)
                NoSuccess;
            http.resetConnection();
            long ptr = 0;
            std::string body;

            while (ptr < buff_len)
            {
                const int bl = 4;
                long len = bl;
                char *buff = buff_start + ptr;
                if (ulxr::HttpProtocol::ConnBody == http.connectionMachine(buff, len))
                    body.append(buff, len);
                ptr += bl;
            }

            free(buff_start);
            std::string b = "kas\r\n" "abc123\r\n" "k\r\n";
            if (b != body)
                NoSuccess;
            if (http.hasBytesToRead())
                NoSuccess;
            if (http.getHttpProperty("footer1") != "text1")
                NoSuccess;
            if (http.getHttpProperty("footer2") != "text2")
                NoSuccess;
        }

        {
            char *buff_start = strdup(http_msg4);
            const int buff_len = strlen(buff_start);  // a multiple of 3
            if (buff_len % 3 != 0)
                NoSuccess;

            http.resetConnection();
            long ptr = 0;
            std::string body;

            while (ptr < buff_len)
            {
                const int bl = 3;
                long len = bl;
                char *buff = buff_start + ptr;
                if (ulxr::HttpProtocol::ConnBody == http.connectionMachine(buff, len))
                    body.append(buff, len);
                ptr += bl;
            }

            free(buff_start);
            std::string b = "kas\r\n" "abc123\r\n" "k\r\n";
            if (b != body)
                NoSuccess;
            if (http.hasBytesToRead())
                NoSuccess;
            if (http.getHttpProperty("footer1") != "text1")
                NoSuccess;
            if (http.getHttpProperty("footer2") != "text2")
                NoSuccess;
        }

        {
            char *buff_start = strdup(http_msg4);
            const int buff_len = strlen(buff_start);

            http.resetConnection();
            std::string body;

            long len = buff_len;
            char *buff = buff_start;
            if (ulxr::HttpProtocol::ConnBody == http.connectionMachine(buff, len))
                body.append(buff, len);

            free(buff_start);
            std::string b = "kas\r\n" "abc123\r\n" "k\r\n";
            if (b != body)
                NoSuccess;
            if (http.hasBytesToRead())
                NoSuccess;
            if (http.getHttpProperty("footer1") != "text1")
                NoSuccess;
            if (http.getHttpProperty("footer2") != "text2")
                NoSuccess;
        }
    }

    catch(...)
    {
        NoSuccess;
    }

    if (success == 0)
        std::cout << "\nsuccess\n";
    else
        std::cout << "\nfailed\n";

    return success;
}

/***************************************************************************
                  xmlbench - simple test suite for speed

    copyright            : (C) 2002-2007 by Ewald Arnold
    email                : ulxmlrpcpp@ewald-arnold.de

    $Id: xmlbench.cpp 10942 2011-09-13 14:35:52Z korosteleva $

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

#include <iostream>
#include <ctime>
#include <cstring>

#include <ulxmlrpcpp/ulxr_value.h>


int main(int argc, char ** argv)
{
    int success = 0;
    try
    {
        ulxr::Array arr;
        arr.addItem(ulxr::Integer());
        arr.addItem(ulxr::Boolean());
        arr.addItem(ulxr::RpcString());
        arr.addItem(ulxr::Double());
        arr.addItem(ulxr::DateTime());
        arr.addItem(ulxr::Base64());
        arr.addItem(ulxr::Integer());
        arr.addItem(ulxr::Boolean());
        arr.addItem(ulxr::RpcString());
        arr.addItem(ulxr::Double());
        arr.addItem(ulxr::DateTime());
        arr.addItem(ulxr::Base64());
        arr.addItem(ulxr::Integer());
        arr.addItem(ulxr::Boolean());
        arr.addItem(ulxr::RpcString());
        arr.addItem(ulxr::Double());
        arr.addItem(ulxr::DateTime());
        arr.addItem(ulxr::Base64());
        arr.addItem(ulxr::Integer());
        arr.addItem(ulxr::Boolean());
        arr.addItem(ulxr::RpcString());
        arr.addItem(ulxr::Double());
        arr.addItem(ulxr::DateTime());
        arr.addItem(ulxr::Base64());

        const unsigned count = 5000;

////////////////////////////////////////////////////////////////

        std::cout << "Starting time measuring for generating xml data\n";

        std::string us;
        time_t starttime = time(0);
        for (unsigned i1 = 0; i1 < count; ++i1)
        {
            us = arr.getXml();
        }
        std::cout << "Size of xml string in bytes: " << us.length() << std::endl;

        time_t endtime = time(0);
        time_t mins = (endtime - starttime) / 60;
        time_t secs = (endtime - starttime) % 60;
        std::cout << "Time needed for xml (ref: 95us): " << mins << ":"
                  << secs << std::endl;


////////////////////////////////////////////////////////////////

        /* Ratio val1_server/client:
           4000 runs, debug output in file:
               xml:   130s
         */
    }
    catch(...)
    {
        success = 1;
    }
    return success;
}

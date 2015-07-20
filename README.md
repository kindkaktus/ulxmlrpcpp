ulxmlrpcpp
==========
[![Build Status](https://travis-ci.org/kindkaktus/ulxmlrpcpp.svg?branch=master)](https://travis-ci.org/kindkaktus/ulxmlrpcpp)

Ulxmlrpcpp is a lightweight C++ XML-RPC library

The library is initially cloned from http://ulxmlrpcpp.sourceforge.net/ for the purpose of adding IPv6 support and adding support for Ellipic-Curve keys for SSL connections. Besides that some code cleanup has been made.


Quick start
----------------

<code>OSNAME=linux make</code>

will build the library and place it under lib directory

<code>OSNAME=linux make test</code>

will build the library, then build the test suite and then run the test suite against the library

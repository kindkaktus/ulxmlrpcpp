INCLUDES      = -I.  -I$(top_srcdir) $(all_includes) $(EA_EXTRA_INC)

CLEANFILES    = *.~* *.~~* *~

AM_CXXFLAGS   = -DULXR_HAVE_CONFIG_H

EXTRA_DIST    = \
  ca-cert.pem.in \
	nosignedcert.pem.in \
	sessioncert.pem.in \
	signedcert2.pem.in \
	signedcerta.pem.in \
	signedcert.pem.in

LDADD         = ../../libulxmlrpcpp.la
AM_LDFLAGS    = $(all_libraries) $(LIBPTHREAD) $(LIBEXPAT) $(LIBICONV) $(LIBSOCKET)  $(EA_EXTRA_LIB)  $(PTHREAD_CFLAGS) \
                $(top_builddir)/ulxmlrpcpp/contrib/libulxmlrpcpp_contrib.la

check_PROGRAMS = \
  test_rpc_ssl_client test_rpc_ssl_server \
  mprocess_client mprocess_server

# TESTS        =

test_rpc_ssl_client_SOURCES    = test_rpc_ssl_client.cpp
test_rpc_ssl_server_SOURCES    = test_rpc_ssl_server.cpp

mprocess_client_SOURCES           = mprocess_client.cpp
mprocess_server_SOURCES           = mprocess_server.cpp

mprocess_server_LDADD = $(top_builddir)/ulxmlrpcpp/libulxmlrpcpp.la -lstdc++
mprocess_client_LDADD = $(top_builddir)/ulxmlrpcpp/libulxmlrpcpp.la -lstdc++

test_rpc_ssl_server_LDADD = $(top_builddir)/ulxmlrpcpp/libulxmlrpcpp.la -lstdc++
test_rpc_ssl_client_LDADD = $(top_builddir)/ulxmlrpcpp/libulxmlrpcpp.la -lstdc++

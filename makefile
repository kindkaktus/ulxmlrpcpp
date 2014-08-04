CXX = g++
CXXFLAGS = -c

SRCS = ulxmlrpcpp.cpp  \
	ulxr_call.cpp ulxr_callparse.cpp ulxr_callparse_base.cpp \
	ulxr_connection.cpp ulxr_dispatcher.cpp ulxr_except.cpp ulxr_expatwrap.cpp \
	ulxr_protocol.cpp ulxr_http_protocol.cpp ulxr_mprpc_server.cpp\
	ulxr_requester.cpp ulxr_response.cpp ulxr_responseparse.cpp ulxr_responseparse_base.cpp \
	ulxr_signature.cpp ulxr_ssl_connection.cpp ulxr_tcpip_connection.cpp \
	ulxr_value.cpp ulxr_valueparse.cpp ulxr_valueparse_base.cpp \
	ulxr_xmlparse.cpp ulxr_xmlparse_base.cpp

OBJS = $(SRCS:.cpp=.o)
STATIC_LIB = libulxmlrpcpp.a
OSSPEC=`uname | tr "[:upper:]" "[:lower:]"`-`uname -r`
OUT_DIR = lib/$(OSSPEC)
SRCS_DIR = ulxmlrpcpp
TEST_DIR = tests

.PHONY: lib clean test

lib: $(STATIC_LIB)
test: $(STATIC_LIB) build-test run-test

$(STATIC_LIB): make_out_dir $(OBJS)
	ar rcs $(OUT_DIR)/$(.TARGET) $(OBJS)

make_out_dir:
	mkdir -p $(OUT_DIR)
    
build-test:
	cd $(SRCS_DIR)/$(TEST_DIR) && $(MAKE)
    
run-test:
	cd $(SRCS_DIR)/$(TEST_DIR) && ./run_all_tests.sh

$(OBJS): $(SRCS_DIR)/$(.PREFIX).cpp
	$(CXX) -I. $(CXXFLAGS) $(SRCS_DIR)/$(.PREFIX).cpp -o $(.TARGET)


clean: clean-test
	-rm -f $(OBJS) $(OUT_DIR)/$(STATIC_LIB)

clean-test:
	cd $(SRCS_DIR)/$(TEST_DIR) && $(MAKE) clean
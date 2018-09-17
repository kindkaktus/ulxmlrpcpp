CXX=g++
CXXFLAGS=-c

SRCS=ulxmlrpcpp.cpp \
	ulxr_call.cpp ulxr_callparse.cpp ulxr_callparse_base.cpp \
	ulxr_connection.cpp ulxr_dispatcher.cpp ulxr_except.cpp ulxr_expatwrap.cpp \
	ulxr_protocol.cpp ulxr_http_protocol.cpp ulxr_mprpc_server.cpp\
	ulxr_requester.cpp ulxr_response.cpp ulxr_responseparse.cpp ulxr_responseparse_base.cpp \
	ulxr_signature.cpp ulxr_ssl_connection.cpp ulxr_tcpip_connection.cpp \
	ulxr_value.cpp ulxr_valueparse.cpp ulxr_valueparse_base.cpp \
	ulxr_xmlparse.cpp ulxr_xmlparse_base.cpp

SRCS_DIR=ulxmlrpcpp
DEPS:=$(SRCS:.cpp=.h)
OBJS=$(SRCS:.cpp=.o)
STATIC_LIB=libulxmlrpcpp.a
OUT_DIR=lib
TEST_DIR=tests

DEPS:=$(patsubst %,$(SRCS_DIR)/%,$(DEPS))

.PHONY: all release debug lib clean test

all release debug: lib

lib: $(STATIC_LIB)

test: $(STATIC_LIB) build-test run-test

$(STATIC_LIB): $(OBJS)
	mkdir -p $(OUT_DIR)
	ar rcs $(OUT_DIR)/$@ $(OBJS)

build-test:
	cd $(SRCS_DIR)/$(TEST_DIR) && $(MAKE)

run-test:
	cd $(SRCS_DIR)/$(TEST_DIR) && ./run_all_tests.sh

%.o: $(SRCS_DIR)/%.cpp $(DEPS)
	$(CXX) -I. -I../$(OPENSSL_INCLUDE_DIR) $(CXXFLAGS) $< -o $@

clean: clean-test
	-rm -f $(OBJS) $(OUT_DIR)/$(STATIC_LIB)

clean-test:
	cd $(SRCS_DIR)/$(TEST_DIR) && $(MAKE) clean

.PHONY: build-test clean

build-test: all_tests

all_tests: all_tests.cpp
	g++ -I../../ all_tests.cpp -o $(.TARGET) ../../lib/`uname | tr "[:upper:]" "[:lower:]"`-`uname -r`/libulxmlrpcpp.a /usr/lib/libexpat.a -lssl -lcrypto -lpthread

clean:
	-rm -f all_tests.o all_tests


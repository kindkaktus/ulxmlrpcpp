.PHONY: build-test clean

build-test: all_tests

all_tests: all_tests.cpp
	g++ -I../../ all_tests.cpp -o all_tests ../../lib/libulxmlrpcpp.a -lexpat -lssl -lcrypto -lpthread

clean:
	-rm -f all_tests.o all_tests


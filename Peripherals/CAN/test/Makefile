# This makefile is a derived, stripped down version of the 
# sample makefile provided by Google.

# Target will be called test.
TEST = test

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = ../../../../googletest-release-1.8.0/googletest-release-1.8.0/googletest/

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -I. -I..

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

MODULE = CAN
MODULE_UNDER_TEST = ../$(MODULE).c
MODULE_OBJECT = $(MODULE).o

all : $(TEST)

clean :
	rm -f $(TEST) gtest.a gtest_main.a *.o

# Instead of building the gtest objects, we will assume that gtest objects 
# has already been built in the make directory in the gtest directory. These
# objects will be copied over.
gtest_main.a : $(GTEST_DIR)/make/gtest_main.a
	cp $(GTEST_DIR)/make/gtest_main.a .

# Build the test
test.o : $(TEST).cpp
	g++ $(CPPFLAGS) $(CXXFLAGS) -c $(TEST).cpp

test : $(MODULE_OBJECT) test.o gtest_main.a
	g++ $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@

# Build module under test
$(MODULE_OBJECT): $(MODULE_UNDER_TEST)
	gcc -I. -I.. -DTEST -c $(MODULE_UNDER_TEST)

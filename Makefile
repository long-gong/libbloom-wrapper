#
# Copyright (c) 2012-2019, Jyri J. Virkki
# All rights reserved.
#
# This file is under BSD license. See LICENSE file.
#
# By default, builds optimized 64bit libbloom (under ./build)
# Requires GNU Make, so invoke appropriately (make or gmake)
#
# Other build options:
#
#   DEBUG=1 make        to build debug instead of optimized
#   BITS=32 make        to build 32bit library
#   BITS=default make   to build platform default bitness (32 or 64)
#
# Other build targets:
#
#   make test           to build and run test code
#   make release_test   to build and run larger tests
#   make gcov           to build with code coverage and run gcov
#   make clean          the usual
#

BLOOM_VERSION_MAJOR=1
BLOOM_VERSION_MINOR=6
BLOOM_VERSION=$(BLOOM_VERSION_MAJOR).$(BLOOM_VERSION_MINOR)

TOP := $(shell /bin/pwd)
BUILD_OS := $(shell uname)

RM = gio trash

BUILD=$(TOP)/build
INC=-I$(TOP) -I$(TOP)/murmur2 -I$(TOP)/wyhash
LIB=-lm 
OPT=-O3
COM=${CC} $(CFLAGS) $(CPPFLAGS) -Wall ${OPT} ${MM} -std=c99 -fPIC -DBLOOM_VERSION=$(BLOOM_VERSION) 
CPPCOM=${CXX} $(CPPFLAGS) -Wall ${OPT} ${MM} -std=c++11 -fPIC -DBLOOM_VERSION=$(BLOOM_VERSION) 
CPPCOMFORBENCH=${CXX} $(CPPFLAGS) -Wall ${OPT} ${MM} -std=c++17 -fPIC -DBLOOM_VERSION=$(BLOOM_VERSION) 
TESTDIR=$(TOP)/misc/test
WRAPPERTESTDIR=$(TOP)/tests
BENCHDIR=$(TOP)/benchmark

ifeq ($(BITS),)
MM=-m64
else ifeq ($(BITS),64)
MM=-m64
else ifeq ($(BITS),32)
MM=-m32
else ifeq ($(BITS),default)
MM=
else
MM=$(BITS)
endif

#
# Shared library names - these definitions work on most platforms but can
# be overridden in the platform-specific sections below.
#
BLOOM_SONAME=libbloom.so.$(BLOOM_VERSION_MAJOR)
SO_VERSIONED=libbloom.so.$(BLOOM_VERSION)
LD_SONAME=-Wl,-soname,$(BLOOM_SONAME)
SO=so


ifeq ($(BUILD_OS),$(filter $(BUILD_OS), GNU/kFreeBSD GNU Linux))
RPATH=-Wl,-rpath,$(BUILD)
endif

ifeq ($(BUILD_OS),SunOS)
RPATH=-R$(BUILD)
CC=gcc
endif

ifeq ($(BUILD_OS),OpenBSD)
RPATH=-R$(BUILD)
endif

ifeq ($(BUILD_OS),Darwin)
MAC=-install_name $(BUILD)/libbloom.dylib \
	-compatibility_version $(BLOOM_VERSION_MAJOR) \
	-current_version $(BLOOM_VERSION)
RPATH=-Xlinker -rpath -Xlinker $(BUILD)
SO=dylib
BLOOM_SONAME=libbloom.$(BLOOM_VERSION_MAJOR).$(SO)
SO_VERSIONED=libbloom.$(BLOOM_VERSION).$(SO)
LD_SONAME=
endif

ifeq ($(DEBUG),1)
OPT=-g $(DEBUGOPT)
else
OPT=-O3
endif


all: $(BUILD)/$(SO_VERSIONED) $(BUILD)/libbloom.a example wrapper_example

$(BUILD)/$(SO_VERSIONED): $(BUILD)/murmurhash2.o $(BUILD)/bloom.o
	(cd $(BUILD) && \
	    $(COM) $(LDFLAGS) bloom.o murmurhash2.o -shared $(LIB) $(MAC) \
		$(LD_SONAME) -o $(SO_VERSIONED) && \
		rm -f $(BLOOM_SONAME) && ln -s $(SO_VERSIONED) $(BLOOM_SONAME) && \
		rm -f libbloom.$(SO) && ln -s $(BLOOM_SONAME) libbloom.$(SO))

$(BUILD)/libbloom.a: $(BUILD)/murmurhash2.o $(BUILD)/bloom.o
	(cd $(BUILD) && ar rcs libbloom.a bloom.o murmurhash2.o)

$(BUILD)/test-libbloom: $(TESTDIR)/test.c $(BUILD)/$(SO_VERSIONED)
	$(COM) -I$(TOP) -c $(TESTDIR)/test.c -o $(BUILD)/test.o
	(cd $(BUILD) && \
	    $(COM) test.o -L$(BUILD) $(RPATH) -lbloom $(LIB) -o test-libbloom)

$(BUILD)/test-perf: $(TESTDIR)/perf.c $(BUILD)/$(SO_VERSIONED)
	$(COM) -I$(TOP) -c $(TESTDIR)/perf.c -o $(BUILD)/perf.o
	(cd $(BUILD) && \
	    $(COM) perf.o -L$(BUILD) $(RPATH) -lbloom $(LIB) -o test-perf)

$(BUILD)/bf-perf: $(BENCHDIR)/benchmarks.cpp $(TOP)/bloom.c $(TOP)/murmur2/MurmurHash2.c
	@echo "Downloading two other bloom filters"
	cd $(BUILD) && git clone https://github.com/ArashPartow/bloom.git
	(cd $(BUILD) && git clone https://github.com/mavam/libbf.git && cd libbf && mkdir install && \
		./configure --prefix=../install && make && make install)
	@echo "Downloading completed"
	$(CPPCOMFORBENCH) -I$(TOP) -I$(TOP)/murmur2 -I$(BENCHDIR) -I$(BUILD) -I$(BUILD)/bloom -I$(BUILD)/libbf/install/include -L$(BUILD)/libbf/install/lib $^ -o $@ -lbf 

$(BUILD)/bf_libbloom_org_perf: $(BENCHDIR)/benchmark_libbloom_org.cpp	
	cd $(BUILD) && git clone https://github.com/jvirkki/libbloom.git 
	$(CPPCOMFORBENCH) -I$(TOP) -I$(TOP)/murmur2 -I$(BENCHDIR) -I$(BUILD) -I$(BUILD)/bloom -I$(BUILD)/libbf/install/include -L$(BUILD)/libbf/install/lib $< $(BUILD)/libbloom/bloom.c  $(BUILD)/libbloom/murmur2/MurmurHash2.c -o $@ 



$(BUILD)/test-basic: $(TESTDIR)/basic.c $(BUILD)/libbloom.a
	$(COM) -I$(TOP) \
	    $(TESTDIR)/basic.c $(BUILD)/libbloom.a $(LIB) -o $(BUILD)/test-basic

$(BUILD)/test-cpp-wrapper: $(WRAPPERTESTDIR)/BloomFilterTest.cpp $(TOP)/bloom.c $(TOP)/murmur2/MurmurHash2.c 
	$(CPPCOM) -I$(TOP) -I$(TOP)/murmur2 $^ -o $@ -lgtest_main -lgtest -lpthread

$(BUILD)/%.o: %.c
	mkdir -p $(BUILD)
	$(COM) $(INC) -c $< -o $@

$(BUILD)/murmurhash2.o: murmur2/MurmurHash2.c murmur2/murmurhash2.h
	mkdir -p $(BUILD)
	$(COM) $(INC) -c murmur2/MurmurHash2.c -o $(BUILD)/murmurhash2.o


example: example/example.cc $(BUILD)/libbloom.a
	$(CXX) example/example.cc -o $(BUILD)/example -Wall -L$(BUILD) -lbloom


wrapper_example: example/wrapper_example.cc $(BUILD)/libbloom.a
	$(CXX) example/wrapper_example.cc -o $(BUILD)/wrapper_example -Wall -I$(TOP) -L$(BUILD) -lbloom


clean:
	$(RM) -f $(BUILD)

test: $(BUILD)/test-libbloom $(BUILD)/test-basic $(BUILD)/test-cpp-wrapper
	$(BUILD)/test-basic
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(BUILD)
	$(BUILD)/test-libbloom
	$(BUILD)/test-cpp-wrapper
	@echo "valgrind testing ..."
	valgrind --leak-check=yes --error-exitcode=1 $(BUILD)/test-basic
	valgrind --leak-check=yes --error-exitcode=1 $(BUILD)/test-libbloom
	valgrind --leak-check=yes --error-exitcode=1 $(BUILD)/test-cpp-wrapper
	@echo "tests completed"
	

perf: $(BUILD)/test-perf $(BUILD)/bf-perf $(BUILD)/bf_libbloom_org_perf
	$(BUILD)/bf-perf
	$(BUILD)/bf_libbloom_org_perf
	$(BUILD)/test-perf

vtest: $(BUILD)/test-libbloom
	valgrind --tool=memcheck --leak-check=full --show-reachable=yes \
	    $(BUILD)/test-libbloom

gcov:
	$(MAKE) clean
	DEBUG=1 DEBUGOPT="-fprofile-arcs -ftest-coverage" \
	    $(MAKE) $(BUILD)/test-libbloom
	(cd $(BUILD) && \
	    cp ../*.c . && \
	    ./test-libbloom && \
	    gcov -bf bloom.c)
	@echo Remember to make clean to remove instrumented objects

lcov: gcov
	lcov --capture --directory build --output-file lcov.info
	lcov --remove lcov.info xxhash.c --output-file lcov.info
	genhtml lcov.info --no-branch-coverage \
		--output-directory $(LCOV_OUTPUT_DIR)
	rm -f lcov.info
	$(MAKE) clean

#
# This target runs a test which creates a filter of capacity N and inserts
# N elements, for N in 100,000 to 1,000,000 with an expected error of 0.001.
# To preserve and graph the output, move it to ./misc/collisions and use
# the ./misc/collisions/dograph script to plot it.
#
# WARNING: This can take a very long time (on a slow machine, multiple days)
# to run.
#
collision_test: $(BUILD)/test-libbloom
	$(BUILD)/test-libbloom -G 100000 1000000 10 0.001 \
	    | tee collision_data_v$(BLOOM_VERSION)

#
# This target should be run when preparing a release, includes more tests
# than the 'test' target.
# For a final release, should run the collision_test target above as well,
# not included here as it takes so long.
#
release_test:
	$(MAKE) test
	$(MAKE) vtest
	$(BUILD)/test-libbloom -G 100000 1000000 50000 0.001 \
	    | tee short_coll_data
	gzip short_coll_data
	./misc/collisions/dograph short_coll_data.gz


DESTDIR     ?=
# directory variables: GNU conventions prefer lowercase
# see https://www.gnu.org/prep/standards/html_node/Makefile-Conventions.html
# support both lower and uppercase (BSD), use uppercase in script
prefix      ?= /usr/local
PREFIX      ?= $(prefix)
exec_prefix ?= $(PREFIX)
libdir      ?= $(exec_prefix)/lib
LIBDIR      ?= $(libdir)
includedir  ?= $(PREFIX)/include
INCLUDEDIR  ?= $(includedir)
bindir      ?= $(exec_prefix)/bin
BINDIR      ?= $(bindir)
datarootdir ?= $(PREFIX)/share
mandir      ?= $(datarootdir)/man
man1dir     ?= $(mandir)/man1

ifneq (,$(filter $(shell uname),SunOS))
INSTALL ?= ginstall
else
INSTALL ?= install
endif

INSTALL_PROGRAM ?= $(INSTALL)
INSTALL_DATA    ?= $(INSTALL) -m 644

.PHONY: install
install: $(BUILD)/$(SO_VERSIONED) $(BUILD)/libbloom.a
	@echo Installing libbloom
	@$(INSTALL) -d -m 755 $(DESTDIR)$(LIBDIR)
	@$(INSTALL_DATA) $(BUILD)/libbloom.a $(DESTDIR)$(LIBDIR)
	@$(INSTALL_PROGRAM) $(BUILD)/$(SO_VERSIONED) $(DESTDIR)$(LIBDIR)
	@$(INSTALL_PROGRAM) $(BUILD)/libbloom.so $(DESTDIR)$(LIBDIR)
	@$(INSTALL_PROGRAM) $(BUILD)/libbloom.so.1 $(DESTDIR)$(LIBDIR)
	@$(INSTALL) -d -m 755 $(DESTDIR)$(INCLUDEDIR)   # includes
	@$(INSTALL_DATA) bloom.h $(DESTDIR)$(INCLUDEDIR)
	@$(INSTALL_DATA) murmur2/murmurhash2.h $(DESTDIR)$(INCLUDEDIR)
	@echo libbloom installation completed
	@echo Installing Python wrapper 
	@python3 ./setup.py install --user 
	@echo Python wrapper installation completed
	@echo Installing C++ wrapper 
	@$(INSTALL_DATA) BitUtil.h $(DESTDIR)$(INCLUDEDIR)
	@$(INSTALL_DATA) BloomFilter.h $(DESTDIR)$(INCLUDEDIR)
	@echo C++ wrapper installation completed

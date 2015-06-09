#++++++++++++++++++++++++++ 发行版/调试版 +++++++++++++++++++++++++++

ifeq ($(findstring release,$(MAKECMDGOALS)),release)	
RELEASE=YES
else
DEBUG=YES
endif

#////////////////////////////////////////////////////////////////////

#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#编译路径
#+++++++++++++++++++++++++++顶层目录+++++++++++++++++++++++++++++++++
ROOT_DIR = .
SOURCE_DIR = $(ROOT_DIR)

#+++++++++++++++++++++++++++生成目标目录+++++++++++++++++++++++++++++
DEST = $(ROOT_DIR)/test

#+++++++++++++++++++++++++++所有源文件+++++++++++++++++++++++++++++++
SRCS := $(wildcard $(SOURCE_DIR)/*.cpp)

#+++++++++++++++++++++++++++生成目标文件+++++++++++++++++++++++++++++
OBJ = $(patsubst %.cpp, %.o, $(SRCS))

#++++++++++++++++++++++++++头文件包含目录++++++++++++++++++++++++++++
INC = -I$(SOURCE_DIR)

#++++++++++++++++++++++++++库连接目录++++++++++++++++++++++++++++++++
			
#/////////////////////////////////////////////////////////////////////
#+++++++++++++++++++++++++++++编译参数+++++++++++++++++++++++++++++++
CXX = g++
CC = gcc
CXXDFLAG = -DLINUX -Wall -Wno-format -g -ggdb -O3 -rdynamic -D_REENTRANT -lstdc++
CXXRFLAG = -DLINUX -Wall -Wno-format -O3 -rdynamic -D_REENTRANT -lstdc++
CXXFLAGS = ${if $(DEBUG),$(CXXDFLAG),$(CXXRFLAG)}
SHAREFLAG = -fPIC -shared

#++++++++++++++++++++++++++++动态库++++++++++++++++++++++++++++++++++
LIBS = -lpthread \
		-levent \
		-levent_core \
		-levent_extra \
		-levent_openssl 

#/////////////////////////////////////////////////////////////////////
#++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

.PHONY: all prebuild build install clean rebuild

all: $(DEST)
debug: $(DEST)
release: $(DEST)

.cpp.o:
	rm -f $@
	$(CXX) $(INC) $(CXXFLAGS) $(SHAREFLAG) -c -o $@ $<

$(DEST): $(OBJ)
	rm -f $@
	$(CXX) $(LDPATHS) $(LIBS) -o $(DEST) $(OBJ) 

clean:
	rm -f $(OBJ)
	rm -f $(DEST)

rebuild: clean all


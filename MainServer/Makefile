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
SOURCE_DIR = $(ROOT_DIR)/source

#+++++++++++++++++++++++++++生成目标目录+++++++++++++++++++++++++++++
DEST = $(ROOT_DIR)/SealServer

#+++++++++++++++++++++++++++所有源文件+++++++++++++++++++++++++++++++
SRCS := $(wildcard $(SOURCE_DIR)/*.cpp \
		$(SOURCE_DIR)/Mysql/*.cpp \
		$(SOURCE_DIR)/WLog/*.cpp	\
		$(SOURCE_DIR)/Redis/*.cpp \
		$(SOURCE_DIR)/Config/*.cpp \
		$(SOURCE_DIR)/Connection/*.cpp \
		$(SOURCE_DIR)/Common/*.cpp \
		$(SOURCE_DIR)/Request/*.cpp)

#+++++++++++++++++++++++++++生成目标文件+++++++++++++++++++++++++++++
OBJ = $(patsubst %.cpp, %.o, $(SRCS))

#++++++++++++++++++++++++++头文件包含目录++++++++++++++++++++++++++++
INC = -I$(SOURCE_DIR) \
	  -I$(SOURCE_DIR)/WLog  \
	  -I$(SOURCE_DIR)/Config \
	  -I$(SOURCE_DIR)/Mysql \
	  -I$(SOURCE_DIR)/Redis \
	  -I$(ROOT_DIR)/../MemPool \
	  -I$(SOURCE_DIR)/Connection \
	  -I$(SOURCE_DIR)/Request \
	  -I$(SOURCE_DIR)/Common \
	  -I/usr/local/include/hiredis \
	  -I/usr/include/mysql

#++++++++++++++++++++++++++库连接目录++++++++++++++++++++++++++++++++
LDPATHS =-L/usr/lib64/mysql
			
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
		-levent_pthreads \
		-levent_openssl \
		-luuid \
		-lhiredis \
		-lmysqlclient

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


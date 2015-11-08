PROJECT_NAME=smpp
VERSION=0.9.0

CPPC   =g++
CC     =gcc
LINKER =g++

ROOT_DIR = .
SRC_DIR = $(ROOT_DIR)/src
OUTPUT_DIR ?=build/lib

ifeq ($(origin OBJS_ROOT), undefined)
	OBJS_ROOT=build/objs
else
	OBJS_ROOT:=$(OBJS_ROOT)/smpplib
endif

Configuration ?= Release
#Configuration = Debug

OBJS_DIR =$(OBJS_ROOT)/$(Configuration)

CFLAGS :=$(CFLAGS) -Wall -Wextra -Wno-variadic-macros -fPIC -c
LDFLAGS :=$(LDFLAGS) -Wall -fPIC -shared
INCLUDES = -I$(ROOT_DIR) -I$(SRC_DIR)/libsmpp34
LIBS = -lboost_system -lboost_iostreams -lboost_thread

ifeq ($(EXTREME_DEBUG), yes)
	CFLAGS :=$(CFLAGS) -DEXTREME_DEBUG=1
endif

ifeq (Debug, $(findstring Debug,$(Configuration)))
	OUTPUT_LIB    :=lib$(PROJECT_NAME)-$(VERSION)-d.so
	CFLAGS        :=$(CFLAGS) -g -D_DEBUG
	LDFLAGS       :=$(LDFLAGS) -g
else
	OUTPUT_LIB    :=lib$(PROJECT_NAME)-$(VERSION).so
	CFLAGS        :=$(CFLAGS) -g -O2 -fno-strict-aliasing
	LDFLAGS       :=$(LDFLAGS) -g
endif

OUTPUT_FILE =$(OUTPUT_DIR)/$(OUTPUT_LIB)
LDFLAGS:=$(LDFLAGS) -Wl,-soname,lib$(PROJECT_NAME).so

OBJS = $(OBJS_DIR)/converter.o \
       $(OBJS_DIR)/libsmpp.o \
       $(OBJS_DIR)/logger.o \
       $(OBJS_DIR)/smppconnection.o \
       $(OBJS_DIR)/smppserver.o \
       $(OBJS_DIR)/smppclient.o \
       $(OBJS_DIR)/smppusersmanager.o \
       $(OBJS_DIR)/stdafx.o \
       $(OBJS_DIR)/gsm7.o \
       $(OBJS_DIR)/smpp34_dumpBuf.o \
       $(OBJS_DIR)/smpp34_dumpPdu.o \
       $(OBJS_DIR)/smpp34_pack.o \
       $(OBJS_DIR)/smpp34_params.o \
       $(OBJS_DIR)/smpp34_structs.o \
       $(OBJS_DIR)/smpp34_unpack.o

CPPCOMPILE = $(CPPC) $(CFLAGS) "$<" -o "$(OBJS_DIR)/$(*F).o" $(INCLUDES)
CCOMPILE = $(CC) $(CFLAGS) "$<" -o "$(OBJS_DIR)/$(*F).o" $(INCLUDES)
LINK = $(LINKER) $(LDFLAGS) -o "$(OUTPUT_FILE)" $(OBJS) $(LIBS)


build : $(OUTPUT_FILE)

$(OBJS_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CPPCOMPILE)

$(OBJS_DIR)/%.o : $(SRC_DIR)/libsmpp34/%.c
	$(CCOMPILE)

$(OBJS_DIR)/%.o : $(SRC_DIR)/iconv/%.c
	$(CCOMPILE)

$(SRC_DIR)/smppconnection.cpp: $(SRC_DIR)/smppconnection.h \
	$(SRC_DIR)/smppdefs.h $(SRC_DIR)/smppcommands.h

$(SRC_DIR)/smppserver.cpp: $(SRC_DIR)/smppserver.h \
	$(SRC_DIR)/smppdefs.h $(SRC_DIR)/smppcommands.h $(SRC_DIR)/smppconnection.h $(SRC_DIR)/smppusersmanager.h

$(SRC_DIR)/smppusersmanager.cpp: $(SRC_DIR)/smppusersmanager.h \
	$(SRC_DIR)/smppdefs.h $(SRC_DIR)/smppcommands.h $(SRC_DIR)/smppconnection.h

$(SRC_DIR)/smppclient.cpp: $(ROOT_DIR)/libsmpp.h $(ROOT_DIR)/libsmpp.hpp\
	$(SRC_DIR)/smppdefs.h $(SRC_DIR)/smppcommands.h $(SRC_DIR)/smppconnection.h $(SRC_DIR)/converter.h

$(SRC_DIR)/libsmpp.cpp: $(ROOT_DIR)/libsmpp.h $(ROOT_DIR)/libsmpp.hpp \
	$(SRC_DIR)/smppdefs.h $(SRC_DIR)/smppusersmanager.h $(SRC_DIR)/smppserver.h

$(SRC_DIR)/converter.cpp: $(SRC_DIR)/smppdefs.h

$(OUTPUT_FILE): $(OBJS_DIR) $(OUTPUT_DIR) $(OBJS)
	$(LINK)
	cd $(OUTPUT_DIR) && ln -svf $(OUTPUT_LIB) lib$(PROJECT_NAME).so

$(OBJS_DIR):
	mkdir -p $(OBJS_DIR)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

clean:
	rm -f -r $(OBJS_DIR)/*.o
	rm -f "$(OUTPUT_FILE)"

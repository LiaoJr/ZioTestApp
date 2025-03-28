TARGET=MasterOfZio
CROSS_COMPILE=aarch64-none-linux-gnu-

ROOT_DIR  = .
DEL_DIR		= $(ROOT_DIR)/build
OBJ_DIR 	= $(ROOT_DIR)/build
LIB_DIR		= $(ROOT_DIR)/lib
INCLUDE_DIR = $(ROOT_DIR)/include
APPLICATION_DIR = $(ROOT_DIR)/application
ECAT_MASTER_DIR = $(ROOT_DIR)/ecat_master
UTILS_DIR   = $(ROOT_DIR)/utils
# LIBMODBUS_DIR = $(ROOT_DIR)/libmodbus
INC_DIR = $(INCLUDE_DIR) $(APPLICATION_DIR) $(ECAT_MASTER_DIR) $(UTILS_DIR)

CFLAGS = $(patsubst %, -I %, $(INC_DIR))
CFLAGS+=-Wall -ggdb -O0 -pipe
CXXFLAGS=$(CFLAGS)  -std=c++11 
LDFLAGS=-L. -L$(LIB_DIR) -lpthread -lrt -ldl -lstdc++ -lm -lzecm -lmodbus


CSRCS= \
	application/main.c \
	application/cmd_process.c \
	application/modbus_tcp_server.c \
	application/app_zmcio.c \
	application/data_log.c \
	ecat_master/ecat_master.c \
	ecat_master/eeprom.c \
	ecat_master/slave.c \
	utils/log.c \
	utils/ticks.c \

CXXSRCS= \

ODIR=build
COBJS=$(addprefix $(ODIR)/, $(patsubst %.c,%.c.o,$(CSRCS)))
CXXOBJS=$(addprefix $(ODIR)/, $(patsubst %.cpp,%.cpp.o,$(CXXSRCS)))
ODIRS=$(dir $(COBJS) $(CXXOBJS))

$(ODIR)/%.c.o: %.c
	@echo "compiling:" $<
	@$(CROSS_COMPILE)gcc -c $(CFLAGS) -o $@ $<

$(ODIR)/%.cpp.o: %.cpp
	@echo "compiling:" $<
	@$(CROSS_COMPILE)g++ -c $(CXXFLAGS) -o $@ $<

all: dirs target

dirs:
	@mkdir -pv $(ODIRS)

target: $(COBJS) $(CXXOBJS)
	@echo "linking:" $(TARGET)
	@$(CROSS_COMPILE)g++ -o $(TARGET) $(LDFLAGS) $(COBJS) $(CXXOBJS)

clean:
	rm $(COBJS) $(CXXOBJS) $(TARGET)
	@rm -rvf $(ODIR)


CC = gcc
LIB_TARGET = libhthread.so

INC_DIR = ./inc

CFLAGS = -I$(INC_DIR) -fpermissive
#CFLAGS += -fno-inline
#CFLAGS += -fgnu-tm
CFLAGS += -DUSE_SEMAPHORE
#CFLAGS += -DUSE_FUTEX
#CFLAGS += -DUSE_MUTEX
#CFLAGS += -DUSE_HSEM
#CFLAGS += -DFOR_OPENMP
CFLAGS += -O3 
#CFLAGS += -g 
#CFLAGS += -DHDEBUG
#CFLAGS += -DLOCKFREE_COND
CFLAGS += -DSPIN_LOCK_STYLE

CFLAGS += -DBLOCKING_QUEUE
#CFLAGS += -DLOCKFREE_QUEUE
#CFLAGS += -DHLOCKFREEQUEUE

CFLAGS += -DBLOCKED_BARRIER
#CFLAGS += -DLOCKFREE_BARRIER

LDFLAGS = -L$(LIB_DIR) -lhthread 

LIB_CFLAGS = -shared $(CFLAGS)  -std=c++11  -fpic
LIB_LDFLAGS = -lm -lrt -ldl -lpthread -lstdc++ -ltcmalloc_minimal

BIN_DIR = ./bin
SRC_DIR = ./src
ASM_DIR = ./asm

OBJ_DIR = ./obj
LIB_DIR = ./lib
TEST_DIR = ./test

src = $(wildcard $(SRC_DIR)/*.cpp)
asm = $(wildcard $(ASM_DIR)/*.S)
obj = $(patsubst %.cpp,%.o,$(notdir $(src)))
obj_with_dir = $(addprefix $(OBJ_DIR)/,$(obj))
tests = $(wildcard $(TEST_DIR)/*.cpp)

.phony: $(lib)

lib: $(LIB_DIR)/$(LIB_TARGET)
	 

$(LIB_DIR)/$(LIB_TARGET): $(obj_with_dir)
	$(CC) $(LIB_CFLAGS) -o $(LIB_DIR)/$(LIB_TARGET) $(asm) $(obj_with_dir) $(LIB_LDFLAGS)
	g++ -std=c++11 clean_shm.cpp -Llib -lhthread -o clean_shm

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $(LIB_CFLAGS) $< -o $@
	
clean:
	rm $(LIB_DIR)/* $(OBJ_DIR)/* $(BIN_DIR)/* $(BIN_DIR)/tests clean_shm

SHELL := /bin/bash
BIN := toy-yacc

BUILD_DIR := build
SRC_DIR := src
TEST_DIR := test

INCLUDE := -I./include
CFLAGS := -g -O0 -Wall #-O3
CXXFLAGS := -std=c++11 -g -O0
CXX := g++ 
CC := gcc
ARFLAGS := r

##############################################

SRC1 := $(wildcard $(SRC_DIR)/*.cpp)
SRC1 += $(wildcard $(SRC_DIR)/*.c)
SRC := $(subst $(SRC_DIR)/,,$(SRC1))
OBJ1 := $(patsubst %.cpp,%.o,${SRC1})
OBJ2 := $(patsubst %.c,%.o,${OBJ1})
OBJ := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(OBJ2))
OBJ_S := $(subst $(SRC_DIR)/,,$(OBJ2))
DEP1 := $(patsubst %.cpp,%.d,${SRC1})
DEP2 := $(patsubst %.c,%.d,${DEP1})
DEP := $(subst $(SRC_DIR)/,$(BUILD_DIR)/,$(DEP2))
LIBOBJ := $(subst $(BUILD_DIR)/regex.o,,$(OBJ))

.PHONY: all clean test

vpath %.cpp $(SRC_DIR)
vpath %.c $(SRC_DIR)
vpath %.d $(BUILD_DIR)
vpath %.o $(BUILD_DIR)

all : $(BIN) test

$(BIN) : $(OBJ_S)
	@if \
	$(CXX) $(OBJ) $(LIBS) $(CXXFLAGS) -o $(BIN);\
	then echo -e "[\e[32;1mLINK\e[m] \e[33m$(OBJ) \e[m \e[36m->\e[m \e[32;1m$(BIN)\e[m"; \
	else echo -e "[\e[31mFAIL\e[m] \e[33m$(OBJ) \e[m \e[36m->\e[m \e[32;1m$(BIN)\e[m"; exit -1; fi;
	@if \
	$(AR) $(ARFLAGS) $(BIN).a $(LIBOBJ);\
	then echo -e "[\e[32;1mAR\e[m] \e[33m$(LIBOBJ) \e[m \e[36m->\e[m \e[32;1m$(BIN).a\e[m"; \
	else echo -e "[\e[31mFAIL\e[m] \e[33m$(LIBOBJ) \e[m \e[36m->\e[m \e[32;1m$(BIN).a\e[m"; exit -1; fi;


-include $(DEP) 

$(BUILD_DIR)/%.d: %.cpp
	@if [ ! -d $(BUILD_DIR) ]; then \
	mkdir $(BUILD_DIR); fi;
	@if \
	$(CXX) ${CXXFLAGS} ${INCLUDE} -MM $< > $@;\
	then echo -e "[\e[32mCXX \e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$@\e[m"; \
	else echo -e "[\e[31mFAIL\e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$@\e[m"; exit -1; fi;

$(BUILD_DIR)/%.d: %.c
	@if [ ! -d $(BUILD_DIR) ]; then \
	mkdir $(BUILD_DIR); fi;
	@if \
	$(CC) ${INCLUDE} -MM $< > $@;\
	then echo -e "[\e[32mCC  \e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$@\e[m"; \
	else echo -e "[\e[31mFAIL\e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$@\e[m"; exit -1; fi;

%.o: %.cpp
	@if \
	$(CXX) ${CXXFLAGS} ${INCLUDE} -c $< -o $(BUILD_DIR)/$@; \
	then echo -e "[\e[32mCXX \e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$(BUILD_DIR)/$@\e[m"; \
	else echo -e "[\e[31mFAIL\e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$(BUILD_DIR)/$@\e[m"; exit -1; fi;

%.o: %.c
	@if \
	$(CC) ${CFLAGS} ${INCLUDE} -c $< -o $(BUILD_DIR)/$@; \
	then echo -e "[\e[32mCC  \e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$(BUILD_DIR)/$@\e[m"; \
	else echo -e "[\e[31mFAIL\e[m] \e[33m$<\e[m \e[36m->\e[m \e[33;1m$(BUILD_DIR)/$@\e[m"; exit -1; fi;

test : $(BIN)
	$(MAKE) -C $(TEST_DIR)

clean:
	@echo -e "[\e[32mCLEAN\e[m] \e[33m$(BIN) $(BIN).a $(BUILD_DIR)\e[m"
	@rm -rf $(BIN) $(BIN).a build
	$(MAKE) -C $(TEST_DIR) clean

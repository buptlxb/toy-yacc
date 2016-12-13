TARGET_DIR := target
GTEST_DIR := googletest
USER_DIR := ..
BUILD_DIR := build
SRC_DIR := unittest

CXX := g++
CXXFLAGS += -g -Wall -Wextra -pthread -std=c++11
CPPFLAGS += -isystem $(GTEST_DIR)/include
INCLUDE := -I../include

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_SRCS = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

.PHONY : all clean

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
TARGET = $(patsubst %.cpp, %, ${SRC})

vpath %.cpp $(SRC_DIR)
vpath %.c $(SRC_DIR)
vpath %.d $(BUILD_DIR)
vpath %.o $(BUILD_DIR)

all : ${GTEST_DIR} ${TARGET} 
	@for testcase in ${TARGET}; do \
		${TARGET_DIR}/$$testcase; \
	done

-include $(DEP)

$(BUILD_DIR)/%.d: %.cpp
	@if [ ! -d $(BUILD_DIR) ]; then \
	mkdir $(BUILD_DIR); fi;
	@if \
	$(CXX) ${CXXFLAGS} ${CPPFLAGS} ${INCLUDE} -MM $< > $@;\
	then echo "[\033[32mCXX \033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$@\033[m"; \
	else echo "[\033[31mFAIL\033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$@\033[m"; exit -1; fi;

$(BUILD_DIR)/%.d: %.c
	@if [ ! -d $(BUILD_DIR) ]; then \
	mkdir $(BUILD_DIR); fi;
	@if \
	$(CC) ${CXXFLAGS} ${CPPFLAGS} ${INCLUDE} -MM $< > $@;\
	then echo "[\033[32mCC  \033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$@\033[m"; \
	else echo "[\033[31mFAIL\033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$@\033[m"; exit -1; fi;

% : %.cpp
	@if [ ! -f ${USER_DIR}/toy-yacc.a ]; then \
		echo "[\033[31mTEST\033[m] \033[33m${USER_DIR}/toy-yacc.a not exists\033[m"; exit -1; fi;
	@if [ ! -d $(TARGET_DIR) ]; then \
	mkdir $(TARGET_DIR); fi;
	@if \
	$(CXX) ${CXXFLAGS} ${CPPFLAGS} ${INCLUDE} $< ${USER_DIR}/toy-yacc.a ${GTEST_DIR}/gtest_main.a -o $(TARGET_DIR)/$@; \
	then echo "[\033[32mCXX \033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$(TARGET_DIR)/$@\033[m"; \
	else echo "[\033[31mFAIL\033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$(TARGET_DIR)/$@\033[m"; exit -1; fi;

% : %.c
	@if [ ! -f ${USER_DIR}/toy-yacc.a ]; then \
		echo "[\033[31mTEST\033[m] \033[33m${USER_DIR}/toy-yacc.a\033[m"; exit -1; fi;
	@if \
	$(CC) ${CFLAGS} ${INCLUDE} $< ${USER_DIR}/toy-yacc.a ${GTEST_DIR}/gtest_main.a -o $(BUILD_DIR)/$@; \
	then echo "[\033[32mCC  \033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$(TARGET_DIR)/$@\033[m"; \
	else echo "[\033[31mFAIL\033[m] \033[33m$<\033[m \033[36m->\033[m \033[33;1m$(TARGET_DIR)/$@\033[m"; exit -1; fi;

clean:
	@echo "[\033[32mCLEAN\033[m] \033[33m$(TARGET_DIR) $(BUILD_DIR)\033[m"
	@rm -rf $(TARGET_DIR) $(BUILD_DIR)

${GTEST_DIR} :
	$(MAKE) -C $@

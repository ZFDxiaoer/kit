############################################################
#
#
#
#          聊天室Makefile
#
#
#
############################################################
# 路径配置
CUR_DIR := .
SCRIPT_DIR := $(CUR_DIR)/script
EXT_DIR := $(CUR_DIR)/ext
PROTO_DIR := $(CUR_DIR)/pb
PROTO_SRC_DIR := $(CUR_DIR)/pb/src
# server
SRC_DIR := $(CUR_DIR)/src

# client
# SRC_DIR := $(CUR_DIR)/client


SRC_DIR += $(PROTO_SRC_DIR)
INC_DIR := $(CUR_DIR)/src
INC_DIR += $(shell find $(SRC_DIR) -type d)
INC_DIR += $(EXT_DIR)/boost_1_72_0/
INC_DIR += $(EXT_DIR)/spdlog-1.5.0/include/
LD_LIBS := protobuf boost_system boost_regex pthread
LIB_DIR := $(EXT_DIR)/boost_1_72_0/stage/lib
OBJ_DIR := $(CUR_DIR)/.build/

# 编译参数
CC := gcc
XX := g++
C_FLAGS := -std=c++17 -g -Wall -Werror -fPIC -O0
C_FLAGS += $(addprefix -I,$(INC_DIR))
LD_FLAGS := $(addprefix -L,$(LIB_DIR))
LD_FLAGS += $(addprefix -l,$(LD_LIBS))

#协议文件名
PROTO_NAME := cmd
#应用名
APP_TARGET := michatroom

# 客户端
# APP_TARGET := michatclient

#
# cpp 文件列表计算
#
SRC_FILES := $(shell find $(SRC_DIR) -name "*\.cpp" -type f)
PROTO_SRC_FILES := $(shell find $(SRC_DIR) -name "*\.cc" -type f)
OBJ_FILES := $(SRC_FILES:.cpp=.o)
OBJ_FILES += $(PROTO_SRC_FILES:.cc=.o)
DEP_FILES := $(OBJ_FILES:.o=.d)
ALL_BUILD_OBJ := $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))
ALL_BUILD_DEP := $(addprefix $(OBJ_DIR)/, $(DEP_FILES))


all: proto $(APP_TARGET) 
	@echo -e "\033[36m=========== build $(APP_TARGET) succ =============\033[0m"

$(APP_TARGET): $(ALL_BUILD_OBJ)
	@echo -e "\033[36m[$(APP_TARGET) LINK] $@\033[0m"
	@$(XX) -o $(APP_TARGET) $(C_FLAGS) $(ALL_BUILD_OBJ) $(LIBS_FLAG) $(LD_FLAGS)

$(OBJ_DIR)/%.d: %.cpp
	@echo -e "\033[36m[$(APP_TARGET) DEP] $@\033[0m"
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@set -e; rm -f $@;
	@$(XX) -MM $(C_FLAGS) $< > $@.tmp;
	@sed 's,$(notdir $(basename $@))\.o[ :]*,$(basename $@).o $(basename $@).d : ,g' < $@.tmp > $@;
	@rm -f $@.tmp

$(OBJ_DIR)/%.o: %.cpp
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@echo -e "\033[36m[$(APP_TARGET) CXX] $<\033[0m"
	@$(XX) $(C_FLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cc
	@[ -d $(dir $@) ] || mkdir -p $(dir $@)
	@echo -e "\033[36m[$(APP_TARGET) CXX] $<\033[0m"
	@$(XX) $(C_FLAGS) -c $< -o $@

proto: ${PROTO_DIR}/${PROTO_NAME}.proto
	@echo -e "\033[36m[$(APP_TARGET) gen proto] $@\033[0m"
	@[ -d $(PROTO_SRC_DIR) ] || mkdir -p $(PROTO_SRC_DIR)
	@[ -d $(SCRIPT_DIR) ]  || mkdir -p $(SCRIPT_DIR) 
	@protoc -I=${PROTO_DIR} --cpp_out=${PROTO_SRC_DIR} --python_out=${SCRIPT_DIR} ${PROTO_DIR}/${PROTO_NAME}.proto

clean:
	@rm -rf $(APP_TARGET) $(OBJ_DIR) $(PROTO_SRC_DIR)

help:
	@echo "LD_LIBS " ${LD_LIBS}
	@echo "INC_DIR: "${INC_DIR}
	@echo "C_FLAGS: "$(C_FLAGS)
	@echo "LD_FLAGS: "$(LD_FLAGS)

# 包含依赖文件
ifneq "$(MAKECMDGOALS)" "clean"
ifneq "$(MAKECMDGOALS)" "help"
ifneq "$(MAKECMDGOALS)" "proto"
   -include $(ALL_BUILD_DEP)
endif
endif
endif

.PHONY: all clean

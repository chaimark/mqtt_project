# 启用 .ONESHELL 模式
.ONESHELL:

# 设置默认编译器
CROSS_COMPILE ?=
AR = $(CROSS_COMPILE)ar
CC = $(CROSS_COMPILE)gcc
export CROSS_COMPILE

# 条件设置编译选项（仅在交叉编译时添加 C99 支持）
ifneq ($(CROSS_COMPILE),)
export CFLAGS = -std=gnu99
else
export CFLAGS = -m64
endif
	
# 准备默认编译选项
CFLAGS += -Wall -Wextra -Wno-return-local-addr
CFLAGS += -fplugin=$(shell pwd)/C_MyLib/check_strnew/check_stack_escape.so
LDFLAGS = 

# 调试标志，默认关闭
DEBUG ?= 0
# 检查是否启用 DEBUG 模式
ifeq ($(DEBUG), 1)
    CFLAGS += -g3 -O0
    LDFLAGS += -g3
endif

BUILD_DIR = build
BUILD_DIR_ABS := $(abspath $(BUILD_DIR))

# 连接库
LIB_NAME = -lpthread -lMyLib
LIB_OTHER = -lmqttclient 

# ========== MQTT 头文件路径 ==========
CFLAGS += \
	-I ./C_MyLib \
	-I ./OtherCode \
	-I ./Eclipse_Paho/MQTTClient-C/src \
	-I ./Eclipse_Paho/MQTTClient-C/src/linux \
	-I ./Eclipse_Paho/MQTTPacket/src 

# ========== MQTT 源文件列表 ==========
PROJECT_SRCS := \
	OtherCode/*.c \
	Eclipse_Paho/*.c \
	Eclipse_Paho/MQTTPacket/src/MQTTPacket.c \
	Eclipse_Paho/MQTTPacket/src/MQTTFormat.c \
	Eclipse_Paho/MQTTPacket/src/MQTTConnectClient.c \
	Eclipse_Paho/MQTTPacket/src/MQTTConnectServer.c \
	Eclipse_Paho/MQTTPacket/src/MQTTSerializePublish.c \
	Eclipse_Paho/MQTTPacket/src/MQTTDeserializePublish.c \
	Eclipse_Paho/MQTTPacket/src/MQTTSubscribeClient.c \
	Eclipse_Paho/MQTTPacket/src/MQTTSubscribeServer.c \
	Eclipse_Paho/MQTTPacket/src/MQTTUnsubscribeClient.c \
	Eclipse_Paho/MQTTPacket/src/MQTTUnsubscribeServer.c \
	Eclipse_Paho/MQTTClient-C/src/MQTTClient.c \
	Eclipse_Paho/MQTTClient-C/src/linux/MQTTLinux.c 

PROJECT_OBJS := $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(PROJECT_SRCS))))

# 伪目标
.PHONY: all subMake clear clean distclean distclear mkbuild mqtt_lib init

# ---------- 三个任务存储在变量中，合并成一条规则 ----------
other_exe := 
main_exe := main

# 默认目标：生成所有可执行文件
all: $(main_exe)

mkbuild:
	mkdir -p $(BUILD_DIR)

init:
	@cd C_MyLib/check_strnew/ && ./initcheck_strnew.sh


# 编译规则：将当前目录下的 .c 编译到 build/ 下的 .o
$(BUILD_DIR)/%.o: %.c | mkbuild
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------- 生成 libMyLib.a（子项目） ----------
subMake: mkbuild
	@echo Building MyLib.a
	$(MAKE) -C C_MyLib BUILD_DIR=$(BUILD_DIR_ABS) DEBUG=$(DEBUG)

# ---------- 生成 xxxxx.a ------
mqtt_lib: $(PROJECT_OBJS)
	@echo Building libmqttclient.a
	$(AR) rcs $(BUILD_DIR)/libmqttclient.a $(PROJECT_OBJS)

# 链接 main（新增 mqtt 库依赖）
$(main_exe): %: subMake mqtt_lib $(BUILD_DIR)/%.o
	@echo build $@
	$(CC) $(LDFLAGS) -o $@ $(BUILD_DIR)/$@.o -L$(BUILD_DIR) $(LIB_NAME) $(LIB_OTHER)

# ---------- 其他目标 -------
$(other_exe): %: subMake $(BUILD_DIR)/%.o
	@echo Building $@
	$(CC) $(LDFLAGS) -o $@ $(BUILD_DIR)/$@.o -L$(BUILD_DIR) $(LIB_NAME)

# ---------- 清理 ----------
clear clean distclean distclear:
	@echo Cleaning up
	-rm -rf $(other_exe) $(main_exe) $(BUILD_DIR)
	-$(MAKE) distclean -C C_MyLib BUILD_DIR=$(BUILD_DIR_ABS) DEBUG=$(DEBUG)

# 该目录是用于构建开发板与服务器通信的工具
	交叉编译使用 make CROSS_COMPILE=arm-linux 
	linux 服务器 make

## 这是基于 Paho-Mqtt C 库实现的
	库地址: https://github.com/eclipse-paho/paho.mqtt.embedded-c.git

## mqxsh 用于快速构建或启动 mqtt 交互程序
	用户可自由编辑

## 默认
	运行 install_mqtt
	会自动构建 mqtt 客户端，输入mqx_run 可调用

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <sys/select.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include "MQTTLinux.h"
#include "MQTTClient.h"
#include "./C_MyLib/cJson.h"
#include "./C_MyLib/NumberBaseLib.h"
#include "./C_MyLib/TimeLib.h"

// 定义会发生的事件
#define EVENT_BIT_0 (SIGRTMIN + 1) // 代表 MQTT 连接成功
#define EVENT_BIT_1 (SIGRTMIN + 2) // 代表需要重连
#define EVENT_BIT_2 (SIGRTMIN + 2) // 代表需要重连

// Mqtt 参数
#define DEFINE_QOS 0
#define TIMEOUT 10000
#define UserInputSizeMax 1024
struct _MqttConfigSpaces {
    char Url[256];
    int Port;
    char DevSendTopic[UserInputSizeMax];
    char WebSendTopic[UserInputSizeMax];
    char UserName[50];
    char Password[50];
    char UserId[50];
    bool IsWebClient;
    bool IsShowTime;
    char CmdNameArray[UserInputSizeMax];
    char CmdVarArray[UserInputSizeMax];
} MqttConfigSpaces = {0};
Network NetworkStu;
MQTTClient Client;

JsonObject MqttConfig = {0};
JsonArray CmdName = {0};
JsonArray CmdVar = {0};

// 运行标记
int RunningFlag = -1; // -1 表示未完全启动 mqtt

// 定义互斥锁
pthread_mutex_t MqttMutex = PTHREAD_MUTEX_INITIALIZER;

// 主线程句柄
pthread_t MainThreadId;
// 信号处理函数
void handle_sigint(int Sig) {
    (void)Sig;
    printf("\nCaught SIGINT (Ctrl+C), cleaning up...\n");
    if (RunningFlag == -1) {
        RunningFlag = false;
        exit(0);
    }
    RunningFlag = false; // 主线程稍后会检查这个标志并退出
}

void printConfigStu(void) {
    // 打印配置文件内容
    printf("{\n");
    printf("    devSend_topic: %s\n", MqttConfigSpaces.DevSendTopic);
    printf("    webSend_topic: %s\n", MqttConfigSpaces.WebSendTopic);
    printf("    username: %s\n", MqttConfigSpaces.UserName);
    printf("    password: %s\n", MqttConfigSpaces.Password);
    printf("    userid: %s\n", MqttConfigSpaces.UserId);
    printf("    url: %s\n", MqttConfigSpaces.Url);
    printf("    port: %d\n", MqttConfigSpaces.Port);
    printf("    isWebClient: %s\n", MqttConfigSpaces.IsWebClient ? "true" : "false");
    printf("    isShowTime: %s\n", MqttConfigSpaces.IsShowTime ? "true" : "false");
    printf("    cmd_Name_Array: %s\n", MqttConfigSpaces.CmdNameArray);
    printf("    cmd_Var_Array: %s\n", MqttConfigSpaces.CmdVarArray);
    printf("}\n");
}

// 底层只读文件，无交互
int _readFile(FILE *file, bool IsUpTopic) {
    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long FileSize = ftell(file);
    rewind(file);

    // Allocate memory for the file content
    strnew_malloc(Content, FileSize + 1);
    if (Content.Name._char == NULL) {
        perror("Failed to allocate memory");
        return -1;
    }

    // Read the file content into the buffer
    size_t ReadSize = fread(Content.Name._char, 1, FileSize, file);
    if (ReadSize != (size_t)FileSize) {
        perror("Failed to read the entire file");
        return -1;
    }

    // Null-terminate the string
    Content.Name._char[FileSize] = '\0';

    MqttConfig = newJsonObjectByString(Content);

    if (MqttConfig.isJsonNull(&MqttConfig, "devSend_topic") < 0) {
        return -1;
    }
    if (MqttConfig.isJsonNull(&MqttConfig, "webSend_topic") < 0) {
        return -1;
    }
    if (MqttConfig.isJsonNull(&MqttConfig, "username") < 0) {
        return -1;
    }
    if (MqttConfig.isJsonNull(&MqttConfig, "password") < 0) {
        return -1;
    }
    if (MqttConfig.isJsonNull(&MqttConfig, "userid") < 0) {
        return -1;
    }
    if (MqttConfig.isJsonNull(&MqttConfig, "isWebClient") < 0) {
        return -1;
    }
    if (MqttConfig.isJsonNull(&MqttConfig, "isShowTime") < 0) {
        return -1;
    }

    // 存储结果
    if (IsUpTopic) {
        MqttConfig.getArray(&MqttConfig, "devSend_topic", NEW_NAME(MqttConfigSpaces.DevSendTopic));
        MqttConfig.getArray(&MqttConfig, "webSend_topic", NEW_NAME(MqttConfigSpaces.WebSendTopic));
    }
    MqttConfig.getString(&MqttConfig, "username", NEW_NAME(MqttConfigSpaces.UserName));
    MqttConfig.getString(&MqttConfig, "password", NEW_NAME(MqttConfigSpaces.Password));
    MqttConfig.getString(&MqttConfig, "userid", NEW_NAME(MqttConfigSpaces.UserId));
    MqttConfigSpaces.IsWebClient = MqttConfig.getBool(&MqttConfig, "isWebClient");
    MqttConfigSpaces.IsShowTime = MqttConfig.getBool(&MqttConfig, "isShowTime");

    if ((MqttConfig.isJsonNull(&MqttConfig, "cmd_Name_Array") >= 0) &&
        (MqttConfig.isJsonNull(&MqttConfig, "cmd_Var_Array") >= 0)) {
        CmdVar = MqttConfig.getArray(&MqttConfig, "cmd_Var_Array", NEW_NAME(MqttConfigSpaces.CmdVarArray));
        CmdName = MqttConfig.getArray(&MqttConfig, "cmd_Name_Array", NEW_NAME(MqttConfigSpaces.CmdNameArray));
    }
    if (CmdName.sizeItemNum(&CmdName) != CmdVar.sizeItemNum(&CmdVar)) {
        JsonArray Zero = {0};
        CmdName = Zero;
        CmdVar = Zero;
        printf("CmdName_Array and CmdVar_Array size not equal\n");
        return -1;
    }
    return 0;
}

// return 负数表示返回失败
int readConfigFile(const char *FileName) {
    FILE *file = fopen(FileName, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }

    int ResFlag = 0;
    if (_readFile(file, true) == -1) {
        goto EndOver;
    }

    printConfigStu();
    printf("please key your url: 35.172.255.228:1883\n>>:");
    newString(IpAndProt, 100);

    char Url[100] = {0};
    unsigned int Port = 0;
    // 安全读取输入
    if (fgets(IpAndProt.Name._char, IpAndProt.MaxLen, stdin) == NULL) {
        fprintf(stderr, "Input error\n");
        goto EndOver;
    }
    // 移除换行符（如果有）
    IpAndProt.Name._char[strcspn(IpAndProt.Name._char, "\n")] = '\0';
    // 解析 IP 和端口
    if (sscanf(IpAndProt.Name._char, "%99[^:]:%d", Url, &Port) != 2) {
        fprintf(stderr, "Invalid format! Use 'ip:port'\n");
        goto EndOver;
    }
    // 验证端口
    if (Port > 65535) {
        fprintf(stderr, "Invalid format! Use 'ip:port'\n");
        goto EndOver;
    }
    strcpy(MqttConfigSpaces.Url, Url);
    MqttConfigSpaces.Port = Port;
    ResFlag = 1; // 所有读取全部通过
EndOver:
    system("clear");
    ResFlag--;
    fclose(file);
    printConfigStu();
    return ResFlag;
}

// 消息回调函数：收到订阅主题的消息时调用
void messageArrived(MessageData *Md) {
    MQTTMessage *msg = Md->message;
    if (MqttConfigSpaces.IsShowTime == true) {
        uint32_t current_time = time(NULL);
        TimeStuClass RTC_Data = timestampToRTCData(current_time + 8 * 60 * 60);
        printf("%04d-%02d-%02d %02d:%02d:%02d  ", RTC_Data.year, RTC_Data.month, RTC_Data.day, RTC_Data.hour, RTC_Data.minute, RTC_Data.second);
    }
    printf("Topic:%.*s\n", (int)Md->topicName->lenstring.len, (char *)Md->topicName->lenstring.data);
    printf("%.*s\n", (int)msg->payloadlen, (char *)msg->payload);
}

// 检查用户的指令是否匹配快捷指令表
int makeUpDataJson(strnew StrSpace) {
    // 测试 对象数
    newRootJsonObject(StrSpace);
    addJsonItemData(StrSpace, "model:\"%s\"", "aaaaaa");
    newSubArrayJsonItem(StrSpace, "messages", {
        newSubObjectJsonItem(StrSpace, "", {
            addJsonItemData(StrSpace, "role:\"%s\"", "aaaaaaaa");
            addJsonItemData(StrSpace, "content:\"%s\"", "AAAAAAA");
        });
        newSubObjectJsonItem(StrSpace, "", {
            addJsonItemData(StrSpace, "role:\"%s\"", "bbbbbb");
            addJsonItemData(StrSpace, "content:\"%s\"", "BBBBBBB");
        });
    });
    addJsonItemData(StrSpace, "temperature:%.1f", 3.14);
    addJsonItemData(StrSpace, "max_tokens:%d", 37648);
    addJsonItemData(StrSpace, "stream:%s", "false");
    return 1;
}

// 线程扫描接收 Buff
void *mqttYieldThread(void *arg) {
    MQTTClient *client = (MQTTClient *)arg;
    int Rc = SUCCESS;
    while (RunningFlag) {
        pthread_mutex_lock(&MqttMutex); // 加锁
        Rc = MQTTYield(client, 30);
        pthread_mutex_unlock(&MqttMutex); // 解锁
                                          //
        // 每100ms处理一次MQTT网络事件
        if (Rc != SUCCESS) {
            printf("res data error\n");
            pthread_kill(MainThreadId, EVENT_BIT_1);
            break;
        }
        usleep(30 * 1000); // 空闲等待
    }
    return NULL;
}

// 定时器回调函数,周期置位事件
void setSendHeatPack(union sigval Sv) {
    static uint8_t HeatConunt = 0;
    pthread_t *NowThreadId = (pthread_t *)Sv.sival_ptr;
    if (NowThreadId != NULL) {
        pthread_kill((*NowThreadId), EVENT_BIT_0);
    }
    if (HeatConunt == 3) {
        pthread_kill((*NowThreadId), EVENT_BIT_2);
    }
    HeatConunt = (HeatConunt < 3 ? HeatConunt + 1 : 0);
}

// 设置定时器
timer_t startTimer(void) {
    timer_t TimerId = (timer_t)NULL;
    struct sigevent Sev;
    // 配置通知机制为：SIGEV_THREAD（触发回调线程）
    Sev.sigev_notify = SIGEV_THREAD;
    // 绑定你的回调函数
    Sev.sigev_notify_function = setSendHeatPack;
    // 传递给回调的参数（不需要就传NULL）
    Sev.sigev_value.sival_ptr = (void *)MainThreadId;
    Sev.sigev_notify_attributes = NULL;

    // 创建定时器
    timer_create(CLOCK_MONOTONIC, &Sev, &TimerId);

    // 设置时间：自动重复触发
    struct itimerspec Its;
    Its.it_value.tv_sec = 3; // First expiration
    Its.it_value.tv_nsec = 0;
    Its.it_interval.tv_sec = 7; // Period
    Its.it_interval.tv_nsec = 0;

    // 启动定时器
    timer_settime(TimerId, 0, &Its, NULL);
    return TimerId;
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);

    // 注册 SIGINT 信号处理函数
    signal(SIGINT, handle_sigint);
    system("clear");
    if (readConfigFile("config.json") < 0) {
        return 0;
    }

    // 创建一个事件组
    sigset_t event_group;
    siginfo_t info;
    struct timespec timeout = {
        .tv_sec = 0,
        .tv_nsec = 0,
    };

    // 防止信号（事件位）直接杀死进程
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, EVENT_BIT_0);
    sigaddset(&mask, EVENT_BIT_1);
    sigaddset(&mask, EVENT_BIT_2);
    sigemptyset(&event_group); // 初始化事件组
    // 保存主线程句柄,供定时器周期通知主线程
    sigaddset(&event_group, EVENT_BIT_0);
    sigaddset(&event_group, EVENT_BIT_1);
    sigaddset(&event_group, EVENT_BIT_2);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
    MainThreadId = pthread_self();
    timer_t TimerId = startTimer(); // 开启定时器,周期设置事件位

ReConnect:
    printf("Connecting to %s:%d...\n", MqttConfigSpaces.Url, MqttConfigSpaces.Port);
    NetworkInit(&NetworkStu);
    int Rc = NetworkConnect(&NetworkStu, MqttConfigSpaces.Url, MqttConfigSpaces.Port);
    if (Rc != 0) {
        printf("TCP connect failed, rc=%d\n", Rc);
        return -1;
    }
    printf("TCP connected\n");

    newString(ReadBuf, UserInputSizeMax * 2);
    newString(SendBuf, UserInputSizeMax * 2);
    MQTTClientInit(&Client, &NetworkStu, TIMEOUT, SendBuf.Name._uchar, SendBuf.MaxLen, ReadBuf.Name._uchar, ReadBuf.MaxLen);

    MQTTPacket_connectData MqttData = MQTTPacket_connectData_initializer;
    MqttData.keepAliveInterval = 60;
    MqttData.cleansession = 1;
    MqttData.clientID.cstring = MqttConfigSpaces.UserId;
    MqttData.username.cstring = MqttConfigSpaces.UserName;
    MqttData.password.cstring = MqttConfigSpaces.Password;
    Rc = MQTTConnect(&Client, &MqttData);
    if (Rc != SUCCESS) {
        printf("MQTT connect failed, rc=%d\n", Rc);
        NetworkDisconnect(&NetworkStu);
        return -1;
    }
    printf("MQTT connected, ClientID=%s\n", MqttConfigSpaces.UserId);

    // 准备发布的 topic; 和订阅的 topic
    JsonArray SendTopic = {0};
    JsonArray SubTopic = {0};
    if (MqttConfigSpaces.IsWebClient == true) {
        SendTopic = newJsonArrayByString(NEW_NAME(MqttConfigSpaces.WebSendTopic));
        // 如果是 网络客户端, 即 pc 监控端
        // 需要监控所有 dev 的 send topic
        SubTopic = newJsonArrayByString(NEW_NAME(MqttConfigSpaces.DevSendTopic));
    } else {
        SendTopic = newJsonArrayByString(NEW_NAME(MqttConfigSpaces.DevSendTopic));
        // 否则是 设备客户端, 即 dev 模拟端
        // 需要监控 web 下发命令的 send topic
        SubTopic = newJsonArrayByString(NEW_NAME(MqttConfigSpaces.WebSendTopic));
    }

    newString(TempTopic, UserInputSizeMax);
    for (int i = 0; i < SubTopic.sizeItemNum(&SubTopic); i++) {
        SubTopic.get(&SubTopic, TempTopic, i);
        printf("Subscribing to [%s]\n", TempTopic.Name._cschar);
        Rc = MQTTSubscribe(&Client, TempTopic.Name._cschar, DEFINE_QOS, messageArrived);
        if (Rc != SUCCESS) {
            return -1;
        }
        int HeadAddrNum = strlen(TempTopic.Name._char);
        TempTopic.Name._char[HeadAddrNum++] = '\0';
        TempTopic.MaxLen -= HeadAddrNum;
        TempTopic.Name._char = &TempTopic.Name._char[HeadAddrNum];
    }
    usleep(500000); // 500ms 延迟,等待 SUBACK
    if (Rc != SUCCESS) {
        return -1;
    }

    // 定义一个用户键盘输入的缓冲区
    newString(UserString, UserInputSizeMax * 2);
    // 将发布 topic 从 json 数组中提取出来，暂时存到用户缓存区
    SendTopic.get(&SendTopic, UserString, 0);
    // 释放发布数组的空间 MqttConfigSpaces.xxxSendTopic
    memset(SendTopic.JsonString.Name._char, 0, SendTopic.JsonString.MaxLen);
    // 内存复制发布 topic 到刚释放的 MqttConfigSpaces.xxxSendTopic
    memcpy(SendTopic.JsonString.Name._char, UserString.Name._cschar, strlen(UserString.Name._cschar));
    // 复位用户缓存区
    memset(UserString.Name._char, 0, UserString.MaxLen);
    printf("\nPlease enter help\n");

    RunningFlag = true; // 准备启动线程和主循环
    pthread_t ListenTid;
    pthread_create(&ListenTid, NULL, mqttYieldThread, &Client);
    // 循环等待用户输入
    while (RunningFlag) {
        int sig = sigtimedwait(&event_group, &info, &timeout);
        if ((sig > 0) && (sig == EVENT_BIT_2)) {
            memset(UserString.Name._char, 0, UserString.MaxLen);
            if (makeUpDataJson(UserString) == -1) {
                continue;
            }
            // 准备发布
            MQTTMessage MqttMsg = {
                .qos = DEFINE_QOS,
                .retained = 0,
                .dup = 0,
                .payload = UserString.Name._void,
                .payloadlen = strlen(UserString.Name._char),
            };
            // 发布
            pthread_mutex_lock(&MqttMutex); // 加锁
            Rc = MQTTPublish(&Client, SendTopic.JsonString.Name._cschar, &MqttMsg);
            pthread_mutex_unlock(&MqttMutex); // 解锁
            printf("SendFlag=%s >> %s\n\n", (Rc == 0 ? "true" : "false"), SendTopic.JsonString.Name._cschar);
            memset(UserString.Name._char, 0, UserString.MaxLen);
        }
        // 判断事件是否置位
        if ((sig > 0) && (sig == EVENT_BIT_0)) {
            // 构造你的心跳 JSON 或字符串
            char heatPack[] = "{\"cmd\":\"heartbeat\"}";
            MQTTMessage HeartBeatMsg = {
                .qos = DEFINE_QOS,
                .retained = 0,
                .dup = 0,
                .payload = (void *)heatPack,
                .payloadlen = strlen(heatPack),
            };
            // 安全地发送 MQTT 数据（主循环和 yield 线程共用 client，必须加锁）
            pthread_mutex_lock(&MqttMutex);
            Rc = MQTTPublish(&Client, SendTopic.JsonString.Name._cschar, &HeartBeatMsg);
            pthread_mutex_unlock(&MqttMutex);
        }
        // 判断是否重连
        if ((sig > 0) && (sig == EVENT_BIT_1)) {
            pthread_join(ListenTid, NULL);
            MQTTDisconnect(&Client);
            NetworkDisconnect(&NetworkStu);
            printf("Reboot Connect\n");
            goto ReConnect;
        }
    }
    printf("Disconnected\n");
    pthread_join(ListenTid, NULL);
    MQTTDisconnect(&Client);
    NetworkDisconnect(&NetworkStu);
    timer_delete(TimerId);
    return 0;
}

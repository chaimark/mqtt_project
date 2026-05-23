#ifndef __EVENTGROUPLIB__
#define __EVENTGROUPLIB__

#include "./StrLib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <string.h>

struct _ItemEvent{


}ItemEvent;
typedef struct _EventGroup {
	// 事件总数
	int EventsNumber;
	// 添加事件	
    int (*addEvent)(struct _EventGroup This,strnew Name);
	// 等待某个事件（阻塞）
	int (*waitEventForName)(struct _EventGroup This, strnew Name);
	// 检查是否有事件产生
	int (*checkEvents)(struct _EventGroup This);
} eventgroup_t;

// 定义一个事件组
extern eventgroup_t newEventGroup(void);
// 删除所有事件
extern void cleanEventGroup(eventgroup_t This);

// 安全宏 - 防止忘记释放
#define eventgroup __attribute__((cleanup(cleanEventGroup))) eventgroup_t

#endif

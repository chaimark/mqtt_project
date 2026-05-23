#include "EventGroupLib.h"

// 添加事件
int _addEvent(struct _EventGroup This, strnew Name) {
    This.EventsNumber++;
	return 0;
}
// 等待某个事件（阻塞）
int _waitEventForName(struct _EventGroup This, strnew Name) {
    return 0;
}
// 检查是否有事件产生
int _checkEvents(struct _EventGroup This) {
    return 0;
}

eventgroup_t newEventGroup(void) {
    eventgroup_t Temp = {0};
	Temp.addEvent = _addEvent;
	Temp.waitEventForName = _waitEventForName;
	Temp.checkEvents = _checkEvents;
	return Temp;
}

void cleanEventGroup(eventgroup_t This) {
    // 释放事件组链表
}

#include "EventGroupLib.h"
#define NowNode This.Head->prev
static itemevent_t *getItmeForName(struct _EventGroup This, strnew Name) {
    itemevent_t *temp = This.Head;
    while (strcmp((*temp).ItemName.Name._char, Name.Name._char) != 0) {
        temp = temp->next;
    }
    return temp;
}

// 添加事件
int _addEvent(struct _EventGroup This, strnew Name) {
    itemevent_t *Temp = (itemevent_t *)malloc(sizeof(itemevent_t));
    if ((This.epfd == -1) || (Temp == NULL)) {
        return -1;
    }
    // 添加数据
    (*Temp).ItemName = Name;
    (*Temp).efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    // 创建事件文件
    if ((*Temp).efd == -1) {
        free(Temp);
        return -1;
    }
    This.EventsNumber++;
    if (This.Head == NULL) {
        Temp->next = Temp;
        Temp->prev = Temp;
        This.Head = Temp;
        NowNode = This.Head;
    } else {
        Temp->next = This.Head;
        Temp->prev = NowNode;
        NowNode->next = Temp;
        NowNode = Temp;
        This.Head->prev = Temp;
    }
    // 添加到 epfd
    This.ev.events = EPOLLIN | EPOLLET; // 边缘触发
    This.ev.data.fd = (*NowNode).efd;
    if (epoll_ctl(This.epfd, EPOLL_CTL_ADD, (*NowNode).efd, &This.ev) == -1) {
        close((*NowNode).efd);
        return -1;
    }
    return 0;
}

// 获取所有已产生的事件 存放到 Events 数组，This.EventsNumber 为最大事件数
int _waitEvents(struct _EventGroup This, struct epoll_event *Events, uint8_t TimeNumber) {
    int nfds = 0;
    while (nfds <= 0) {
        // 阻塞等待事件发生，-1 表示无限等待, 0 表示非阻塞
        nfds = epoll_wait(This.epfd, Events, This.EventsNumber, TimeNumber);

        if (nfds == -1) {
            if (errno == EINTR) {
                continue; // 被信号中断，属于正常现象，重试
            }
            perror("epoll_wait");
            return -1; // 【优化】遇到真正的系统致命错误，直接返回 -1 告诉上层
        }
    }
    return nfds; // 【核心修复】成功时返回实际就绪的事件个数（> 0）
}

// 检查某个事件是否发生，非阻塞
int _readEventForName(struct _EventGroup This, strnew Name) {
    // 查找 Name 对应的 efd
    itemevent_t *AddrItem = getItmeForName(&This, Name);
    uint64_t cnt = 0;
    ssize_t n = read((*AddrItem).efd, &cnt, sizeof(cnt));
    if (n > 0) {
        return cnt;
    }
    return -1;
}

// 设置事件
int _setEventForName(struct _EventGroup This, strnew Name) {
    pthread_mutex_lock(&This.efdlock);
    // 1. 查找 Name 对应的事件节点
    itemevent_t *AddrItem = getItmeForName(&This, Name);
    if (AddrItem == NULL) {
        return -1; // 事件不存在，置位失败
    }
    // 2. 向 eventfd 写入一个 8 字节的计数器值（通常为 1）
    uint64_t signal_val = 1;
    ssize_t n = write((*AddrItem).efd, &signal_val, sizeof(signal_val));
    pthread_mutex_unlock(&This.efdlock);
    if (n == sizeof(signal_val)) {
        return 0; // 成功置位！epoll_wait 将会被立即唤醒
    }
    return -1; // 写入失败
}

// 初始化事件组
eventGroup newEventGroup(void) {
    eventGroup Temp = {0};
    Temp.addEvent = _addEvent;
    Temp.readEventForName = _readEventForName;
    Temp.waitEvents = _waitEvents;
    Temp.setEventForName = _setEventForName;
    pthread_mutex_init(&Temp.efdlock, NULL);
    Temp.Head = NULL;
    // 创建epoll文件
    Temp.epfd = epoll_create1(EPOLL_CLOEXEC);
    if (Temp.epfd == -1) {
        close(Temp.epfd);
    }
    return Temp;
}

// 清理所有的链表节点和事件文件
void cleanEventGroup(eventGroup This) {
    if (This.Head != NULL) {
        itemevent_t *cur = This.Head;
        itemevent_t *next = NULL;
        do {
            next = cur->next; // 先记住下一个节点
            if (cur->efd != -1) {
                close(cur->efd); // 【修复】关闭内核的 eventfd 资源
            }
            free(cur); // 释放当前节点的堆内存
            cur = next;
        } while (cur != This.Head); // 【修复】当 cur 绕了一圈回到 Head 时，安全跳出循环
        // 通过你的指针宏修改外部实体的 Head
        (This).Head = NULL;
    }
    // 【修复】关闭整个事件组的 epoll 句柄资源
    if (This.epfd != -1) {
        close(This.epfd);
        This.epfd = -1; // 刷成 -1，防止二次释放
    }
    (This).EventsNumber = 0;
}

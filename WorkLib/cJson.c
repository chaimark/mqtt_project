#include "NumberBaseLib.h"
#include "cJson.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//==========================================================================================//
//==========================================================================================//
// #define IsOpenFloatHelp_Ability
#ifdef IsOpenFloatHelp_Ability
static char getNowType(const char *NowAddr, char *UserFromNow) {
    if ((*NowAddr != '%') && (NowAddr + 1 != NULL)) {
        return 0;
    }
    int Len = 0;
    while (((NowAddr + 1) != NULL) && (*(NowAddr + 1) != '%')) {
        UserFromNow[Len++] = *(NowAddr++);
        switch (*NowAddr) {
        case 'l':
            UserFromNow[Len++] = *(NowAddr + 0);
            UserFromNow[Len++] = *(NowAddr + 1);
            NowAddr++;
            return ((*(NowAddr) == 'f' ? 'F' : 'D'));
        case 'd':
            UserFromNow[Len++] = *(NowAddr++);
            return 'd';
        case 'o':
            UserFromNow[Len++] = *(NowAddr++);
            return 'o';
        case 'x':
            UserFromNow[Len++] = *(NowAddr++);
            return 'x';
        case 'u':
            UserFromNow[Len++] = *(NowAddr++);
            return 'u';
        case 'f':
            UserFromNow[Len++] = *(NowAddr++);
            return 'f';
        case 'c':
            UserFromNow[Len++] = *(NowAddr++);
            return 'c';
        case 's':
            UserFromNow[Len++] = *(NowAddr++);
            return 's';
        }
    }
    return 0;
}

static bool getFromTypeCheckDoubleOrFloat(strnew FromStr) {
    // 没有 % 退出
    if (strchr(FromStr.Name._char, '%') == NULL) {
        return false;
    }
    const char *NowAddr = FromStr.Name._char;
    do {
        if (strchr(NowAddr, '%') == NULL) {
            break;
        }
        while (((*NowAddr) != '%') && ((NowAddr + 1) != NULL)) {
            NowAddr++; // 不是 %
            continue;
        }
        if ((*(NowAddr + 1) == '%') && ((NowAddr + 1) != NULL)) { // 是否是 %%
            NowAddr++;
            continue;
        }
        char UserFromNow[10] = {0};
        // 找到格式字符串里的第一个 %
        if ((getNowType(NowAddr, UserFromNow) == 'f') || (getNowType(NowAddr, UserFromNow) == 'F')) {
            return true;
        }
        NowAddr += strlen(UserFromNow); // 准备找下一个 %
    } while ((NowAddr + 1) != NULL);
    return false;
}
#endif

static bool isNeedBySignDivde(strnew InputString, int Addr_Over) {
    int ReFlag = 1; // 先预留, 并寻找最后一个字符 '}'
    for (int i = Addr_Over; i > 0; i--) {
        if (InputString.Name._char[i] == ' ' || InputString.Name._char[i] == '\0') {
            continue;
        }
        if (InputString.Name._char[i] == '\n' || InputString.Name._char[i] == '\r') {
            continue;
        }
        if ((ReFlag > 0) && ((InputString.Name._char[i] == '}') || (InputString.Name._char[i] == ']'))) {
            ReFlag--;
            continue;
        }
        if (ReFlag < 1) {
            if ((InputString.Name._char[i] != '{') && (InputString.Name._char[i] != '[')) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

void addJsonItemData(strnew JsonStringSpace, const char *FromStr, ...) {
    char KeyName[200] = {0};
    // 查找 :
    char *Addr_OverName = NULL;
    Addr_OverName = strchr(FromStr, ':');
    const char *Addr_Start = strchr(FromStr, ',');
    if ((Addr_Start != NULL) && (Addr_OverName != NULL) && (Addr_Start < Addr_OverName)) {
        Addr_Start++;
        catString(KeyName, FromStr, 100, (Addr_Start - FromStr));
    } else {
        Addr_Start = FromStr;
    }

    if (Addr_OverName != NULL) {
        catString(KeyName, "\"", 100, 1);
        catString(KeyName, Addr_Start, 100, (Addr_OverName - Addr_Start));
        catString(KeyName, "\"", 100, 1);
        catString(KeyName, Addr_OverName, 100, strlen(Addr_OverName));
    } else {
        memcpy(KeyName, Addr_Start, (strlen(Addr_Start) < 200 ? strlen(Addr_Start) : 200));
    }

    // 获取当前字符串的长度
    int Addr_Over = strlen(JsonStringSpace.Name._char);
    // 初始化可变参数列表
    va_list args;
    va_start(args, FromStr);
    if (Addr_Over > 2) {
        if (isNeedBySignDivde(JsonStringSpace, Addr_Over) == false) {
            JsonStringSpace.Name._char[Addr_Over--] = '\0';
        } else {
            JsonStringSpace.Name._char[Addr_Over - 1] = ',';
        }
    } else if (Addr_Over == 2) {
        JsonStringSpace.Name._char[Addr_Over--] = '\0';
    }
    vsprintf(&JsonStringSpace.Name._char[Addr_Over], KeyName, args);
    // 结束可变参数处理
    va_end(args);
#ifdef IsOpenFloatHelp_Ability
    if (getFromTypeCheckDoubleOrFloat(NEW_NAME(KeyName)) || getFromTypeCheckDoubleOrFloat(NEW_NAME(KeyName))) {
        int NowStrLen = Addr_Over;
        while (!((JsonStringSpace.Name._char[NowStrLen] == '\0') && (JsonStringSpace.Name._char[NowStrLen + 1] == '\0') && (JsonStringSpace.Name._char[NowStrLen + 2] == '\0'))) {
            NowStrLen++;
        }
        while (Addr_Over < NowStrLen) {
            if (JsonStringSpace.Name._char[Addr_Over] == 0) {
                JsonStringSpace.Name._char[Addr_Over] = '.';
            }
            Addr_Over++;
        }
    }
#endif
    if (JsonStringSpace.Name._char[0] == '{') {
        catString(JsonStringSpace.Name._char, "}", JsonStringSpace.MaxLen, 1);
    } else {
        catString(JsonStringSpace.Name._char, "]", JsonStringSpace.MaxLen, 1);
    }
}

//==========================================================================================//
//==========================================================================================//
#define MAX_STACK_SIZE 100 // 定义栈的最大容量
typedef struct {
    char data[MAX_STACK_SIZE];
    int top;
} Stack;
// 初始化栈
static void initStack(Stack *s) {
    s->top = -1;
}
// 判断栈是否为空
static bool isEmptyStack(Stack *s) {
    return s->top == -1;
}
// 入栈
static void pushStack(Stack *s, char ch) {
    if (s->top < MAX_STACK_SIZE - 1) {
        s->data[++(s->top)] = ch;
    }
}
// 出栈
static char popStack(Stack *s) {
    if (!isEmptyStack(s)) {
        return s->data[(s->top)--];
    }
    return '\0'; // 返回一个空字符
}

//==========================================================================================//
// 通用扫描助手: 统一用 strnew.MaxLen 作硬上限, 同时遇到 '\0' 停止
//==========================================================================================//
static bool jsonIsSpace(char Chr) {
    return ((Chr == ' ') || (Chr == '\t') || (Chr == '\r') || (Chr == '\n'));
}

// 跳过空白
static char *jsonSkipSpace(char *P, strnew StrInput) {
    if ((P == NULL) || (StrInput.Name._char == NULL) || (StrInput.MaxLen <= 0)) {
        return P;
    }
    char *Limit = StrInput.Name._char + StrInput.MaxLen;
    while ((P < Limit) && (*P != '\0') && jsonIsSpace(*P)) {
        P++;
    }
    return P;
}

// 去掉 [Start, End) 尾部的空白
static char *jsonTrimSpaceEnd(char *Start, char *End) {
    while ((End > Start) && jsonIsSpace(*(End - 1))) {
        End--;
    }
    return End;
}

// 查找双重字符位置 (StartP 为起点, 搜索范围由 MotherStr.MaxLen 限定)
static char *getDoubleChrOnString(strnew MotherStr, char *StartP, char HeadChr, char EndChr) {
    Stack DoubleChrS;
    initStack(&DoubleChrS);
    char *result = NULL;
    char *Limit = MotherStr.Name._char + MotherStr.MaxLen;
    bool InString = false;
    for (char *P = StartP; (P < Limit) && (*P != '\0'); P++) {
        char Chr = *P;
        if (InString) { // 字符串内容里的括号和转义不参与匹配
            if (Chr == '\\') {
                if (((P + 1) >= Limit) || (*(P + 1) == '\0')) {
                    break;
                }
                P++; // 跳过被转义的字符
            } else if (Chr == '\"') {
                InString = false;
            }
            continue;
        }
        if (Chr == '\"') {
            InString = true;
            continue;
        }
        if (Chr == HeadChr) {
            pushStack(&DoubleChrS, HeadChr);
        } else if (Chr == EndChr) {
            if (!isEmptyStack(&DoubleChrS)) {
                popStack(&DoubleChrS);
                if (isEmptyStack(&DoubleChrS)) {
                    result = P;
                    break;
                }
            }
        }
    }
    return result;
}

// 跳过 JSON 字符串: P 指向起始引号, 成功返回收尾引号后一位, 失败返回 NULL
// Strict=true 时校验转义并拒绝裸控制字符(供格式校验复用)
static char *skipJsonString(strnew JsonStr, char *P, bool Strict) {
    if ((P == NULL) || (*P != '\"')) {
        return NULL;
    }
    char *Limit = JsonStr.Name._char + JsonStr.MaxLen;
    P++;
    while ((P < Limit) && (*P != '\0')) {
        unsigned char Chr = (unsigned char)(*P);
        if (Chr == '"') {
            return (P + 1);
        }
        if (Chr == '\\') {
            P++;
            if ((P >= Limit) || (*P == '\0')) {
                return NULL;
            }
            if (Strict) {
                if(*P == 'u') {
                    for (int i = 0; i < 4; i++) {
                        P++;
                        if ((P >= Limit) || (*P == '\0')) {
                            return NULL;
                        }
                        if (!(((*P >= '0') && (*P <= '9')) || ((*P >= 'a') && (*P <= 'f')) || ((*P >= 'A') && (*P <= 'F')))) {
                            return NULL;
                        }
                    }
                } else if(*P != '"' && *P != '\\' && *P != '/' && *P != 'b' && *P != 'f' && *P != 'n' && *P != 'r' && *P != 't') {
                    return NULL;
                }
            }
            P++;
            continue;
        }
        if (Strict && (Chr < 0x20)) { // 裸控制字符
            return NULL;
        }
        P++;
    }
    return NULL;
}
// 跳过对象/数组值, P 指向 '{' 或 '[', 成功返回配对括号后一位, 失败返回 NULL
static char *skipJsonComposite(strnew JsonStr, char *P) {
    if ((P == NULL) || ((*P != '{') && (*P != '['))) {
        return NULL;
    }
    char *Limit = JsonStr.Name._char + JsonStr.MaxLen;
    int Depth = 0;
    bool InString = false;
    while ((P < Limit) && (*P != '\0')) {
        char Chr = *P;
        if (InString) {
            if (Chr == '\\') {
                if (((P + 1) >= Limit) || (*(P + 1) == '\0')) {
                    return NULL;
                }
                P++;
            } else if (Chr == '\"') {
                InString = false;
            }
        } else if (Chr == '\"') {
            InString = true;
        } else if ((Chr == '{') || (Chr == '[')) {
            Depth++;
        } else if ((Chr == '}') || (Chr == ']')) {
            Depth--;
            if (Depth <= 0) {
                return (P + 1);
            }
        }
        P++;
    }
    return NULL;
}
// 跳过一个值, 成功返回其后第一个分隔符(逗号/收尾括号)或结束位置
static char *skipJsonValue(strnew JsonStr, char *P) {
    if (P == NULL) {
        return NULL;
    }
    if (*P == '\"') {
        return skipJsonString(JsonStr, P, false);
    }
    if ((*P == '{') || (*P == '[')) {
        return skipJsonComposite(JsonStr, P);
    }
    char *Limit = JsonStr.Name._char + JsonStr.MaxLen;
    while ((P < Limit) && (*P != '\0') && (*P != ',') && (*P != '}')) {
        P++;
    }
    return P;
}

// 查找对象根层级中 "Key" 对应的值首地址, 允许冒号前后有空白
// 跳过嵌套对象/数组和字符串内容; 未找到返回 NULL
static char *getJsonValueStart(strnew InputStr, char Key[]) {
    if ((InputStr.Name._char == NULL) || (InputStr.MaxLen <= 0)) {
        return NULL;
    }
    size_t KeyLen = strlen(Key);
    char *Limit = InputStr.Name._char + InputStr.MaxLen;
    char *P = jsonSkipSpace(InputStr.Name._char, InputStr);
    if ((P >= Limit) || (*P != '{')) { // 只处理对象
        return NULL;
    }
    P++;
    while ((P < Limit) && (*P != '\0')) {
        P = jsonSkipSpace(P, InputStr);
        while ((P < Limit) && (*P == ',')) {
            P++;
            P = jsonSkipSpace(P, InputStr);
        }
        if ((P >= Limit) || (*P == '\0') || (*P == '}')) {
            return NULL;
        }
        if (*P != '\"') { // 非标准成员, 跳到下一个分隔符
            P = skipJsonValue(InputStr, P);
            if (P == NULL) {
                return NULL;
            }
            continue;
        }
        char *KeyStart = P + 1;
        char *AfterKey = skipJsonString(InputStr, P, false);
        if (AfterKey == NULL) {
            return NULL;
        }
        char *Q = jsonSkipSpace(AfterKey, InputStr);
        if ((Q >= Limit) || (*Q != ':')) { // 不是键值对, 跳过
            P = AfterKey;
            continue;
        }
        Q = jsonSkipSpace(Q + 1, InputStr);
        if ((Q >= Limit) || (*Q == '\0')) {
            return NULL;
        }
        size_t KeyTextLen = (size_t)((AfterKey - 1) - KeyStart);
        if ((KeyTextLen == KeyLen) && (memcmp(KeyStart, Key, KeyLen) == 0)) {
            return Q;
        }
        P = skipJsonValue(InputStr, Q);
        if (P == NULL) {
            return NULL;
        }
    }
    return NULL;
}

//==========================================================================================//
JsonArray newJsonArrayByString(strnew DataInit);
JsonObject newJsonObjectByString(strnew DataInit);
//==========================================================================================//
// 判断数组是否为空: 跳过根 '[', 只允许空白后紧跟 ']' 或结束符
static int Arr_sizeItemNum(struct _JsonArray This) {
    if (This.ItemNum != -1) {
        return This.ItemNum;
    }
    if ((This.JsonString.Name._char == NULL) || (This.JsonString.MaxLen <= 0)) {
        return 0;
    }
    char *Limit = This.JsonString.Name._char + This.JsonString.MaxLen;
    char *P = jsonSkipSpace(This.JsonString.Name._char, This.JsonString);
    if ((P >= Limit) || (*P != '[')) {
        return 0;
    }
    P = jsonSkipSpace(P + 1, This.JsonString);
    if ((P >= Limit) || (*P == ']') || (*P == '\0')) { // 空数组
        return 0;
    }
    int Depth = 0;
    int ItemNum = 1; // 至少有一个元素
    bool InString = false;
    while ((P < Limit) && (*P != '\0')) {
        char Chr = *P;
        if (InString) { // 字符串内容不参与切分
            if (Chr == '\\') {
                if (((P + 1) >= Limit) || (*(P + 1) == '\0')) {
                    break;
                }
                P++;
            } else if (Chr == '\"') {
                InString = false;
            }
            P++;
            continue;
        }
        if (Chr == '\"') {
            InString = true;
            P++;
            continue;
        }
        if ((Chr == '{') || (Chr == '[')) {
            Depth++;
            P++;
            continue;
        }
        if ((Chr == '}') || (Chr == ']')) {
            if (Depth > 0) {
                Depth--;
                P++;
                continue;
            }
            if (Chr == ']') { // 根数组结束
                break;
            }
            P++;
            continue;
        }
        if ((Chr == ',') && (Depth == 0)) { // 只统计顶层逗号
            ItemNum++;
        }
        P++;
    }
    This.ItemNum = ItemNum;
    return ItemNum;
}
static signed char Arr_isJsonNull(struct _JsonArray This) {
    if ((This.JsonString.Name._char == NULL) || (This.JsonString.MaxLen <= 0)) {
        return false;
    }
    char *Limit = This.JsonString.Name._char + This.JsonString.MaxLen;
    char *StartP = jsonSkipSpace(This.JsonString.Name._char, This.JsonString);
    if ((StartP >= Limit) || (*StartP != '[')) {
        return false;
    }
    StartP = jsonSkipSpace(StartP + 1, This.JsonString);
    return (((StartP >= Limit) || (*StartP == ']') || (*StartP == '\0')) ? true : false);
}

// 定位数组第 ItemNum 个顶层元素的原始区间, 保留元素自身的括号/引号/嵌套结构
// 跳过字符串内容与转义, 只按顶层逗号和根 ']' 切分; 未找到返回 false
static bool Arr_getItemRange(strnew JsonString, int ItemNum, char **ItemStart, int *ItemLen) {
    if ((JsonString.Name._char == NULL) || (JsonString.MaxLen <= 0) || (ItemNum < 0)) {
        return false;
    }
    char *Limit = JsonString.Name._char + JsonString.MaxLen;
    char *P = jsonSkipSpace(JsonString.Name._char, JsonString);
    if ((P >= Limit) || (*P != '[')) {
        return false;
    }
    P = jsonSkipSpace(P + 1, JsonString);
    if ((P >= Limit) || (*P == ']') || (*P == '\0')) {
        return false;
    }
    int Depth = 0;
    int Index = 0;
    bool InString = false;
    char *Start = P;
    while ((P < Limit) && (*P != '\0')) {
        char Chr = *P;
        if (InString) { // 字符串内容不参与切分
            if (Chr == '\\') {
                if (((P + 1) >= Limit) || (*(P + 1) == '\0')) {
                    break;
                }
                P++;
            } else if (Chr == '\"') {
                InString = false;
            }
            P++;
            continue;
        }
        if (Chr == '\"') {
            InString = true;
            P++;
            continue;
        }
        if ((Chr == '{') || (Chr == '[')) {
            Depth++;
            P++;
            continue;
        }
        if ((Chr == '}') || (Chr == ']')) {
            if (Depth > 0) {
                Depth--;
                P++;
                continue;
            }
            if (Chr == ']') { // 根数组结束
                if (Index == ItemNum) {
                    char *End = jsonTrimSpaceEnd(Start, P);
                    *ItemStart = Start;
                    *ItemLen = (int)(End - Start);
                    return true;
                }
                return false;
            }
            P++;
            continue;
        }
        if ((Chr == ',') && (Depth == 0)) {
            if (Index == ItemNum) {
                char *End = jsonTrimSpaceEnd(Start, P);
                *ItemStart = Start;
                *ItemLen = (int)(End - Start);
                return true;
            }
            Index++;
            Start = jsonSkipSpace(P + 1, JsonString);
            P = Start;
            continue;
        }
        P++;
    }
    return false;
}

static void Arr_getArray(struct _JsonArray This, strnew OutStr, int ItemNum) {
    if (OutStr.Name._char == This.JsonString.Name._char) {
        return;
    }
    memset(OutStr.Name._char, 0, OutStr.MaxLen);
    char *StartP = NULL;
    int ItemLen = 0;
    if (Arr_getItemRange(This.JsonString, ItemNum, &StartP, &ItemLen) == false) {
        return;
    }
    if (ItemLen >= OutStr.MaxLen) { // 输出空间不足时截断, 保证有结束符
        ItemLen = OutStr.MaxLen - 1;
    }
    if (ItemLen > 0) {
        memcpy(OutStr.Name._char, StartP, (size_t)ItemLen);
    }
    OutStr.Name._char[ItemLen] = '\0';
}

static void Arr_get(struct _JsonArray This, strnew OutStr, int ItemNum) {
    memset(OutStr.Name._char, 0, OutStr.MaxLen);
    char *StartP = NULL;
    int ItemLen = 0;
    if (Arr_getItemRange(This.JsonString, ItemNum, &StartP, &ItemLen) == false) {
        return;
    }
    if (StartP[0] == '\"') { // 字符串元素去掉外层引号, 保留内部转义
        int ContentLen = ItemLen - 2;
        if (ContentLen < 0) {
            ContentLen = 0;
        }
        if (ContentLen >= OutStr.MaxLen) {
            ContentLen = OutStr.MaxLen - 1;
        }
        if (ContentLen > 0) {
            memcpy(OutStr.Name._char, (StartP + 1), (size_t)ContentLen);
        }
        OutStr.Name._char[ContentLen] = '\0';
        return;
    }
    if (ItemLen >= OutStr.MaxLen) { // 输出空间不足时截断
        ItemLen = OutStr.MaxLen - 1;
    }
    if (ItemLen > 0) {
        memcpy(OutStr.Name._char, StartP, (size_t)ItemLen);
    }
    OutStr.Name._char[ItemLen] = '\0';
}

JsonArray newJsonArrayByString(strnew DataInit) {
    JsonArray Temp;
    Temp.JsonString = DataInit;
    Temp.ItemNum = -1;
    Temp.sizeItemNum = Arr_sizeItemNum;
    Temp.isJsonNull = Arr_isJsonNull;
    Temp.getArray = Arr_getArray;
    Temp.get = Arr_get;
    return Temp;
}

//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//
//==========================================================================================//

// 统计对象根层级的键值对个数, 跳过嵌套对象/数组与字符串内容
static int Obj_sizeStr(struct _JsonObject This) {
    if ((This.JsonString.Name._char == NULL) || (This.JsonString.MaxLen <= 0)) {
        return 0;
    }
    char *Limit = This.JsonString.Name._char + This.JsonString.MaxLen;
    char *P = jsonSkipSpace(This.JsonString.Name._char, This.JsonString);
    if ((P >= Limit) || (*P != '{')) { // 只处理对象
        return 0;
    }
    P++;
    int Count = 0;
    while ((P < Limit) && (*P != '\0')) {
        P = jsonSkipSpace(P, This.JsonString);
        while ((P < Limit) && (*P == ',')) {
            P++;
            P = jsonSkipSpace(P, This.JsonString);
        }
        if ((P >= Limit) || (*P == '}') || (*P == '\0')) {
            break;
        }
        if (*P != '\"') { // 非标准成员, 跳到下一个分隔符
            P = skipJsonValue(This.JsonString, P);
            if (P == NULL) {
                break;
            }
            continue;
        }
        char *AfterKey = skipJsonString(This.JsonString, P, false);
        if (AfterKey == NULL) {
            break;
        }
        char *Q = jsonSkipSpace(AfterKey, This.JsonString);
        if ((Q >= Limit) || (*Q != ':')) { // 不是键值对, 跳过
            P = AfterKey;
            continue;
        }
        Q = jsonSkipSpace(Q + 1, This.JsonString);
        if ((Q >= Limit) || (*Q == '\0')) {
            break;
        }
        Count++;
        P = skipJsonValue(This.JsonString, Q);
        if (P == NULL) {
            break;
        }
    }
    return Count;
}
static signed char Obj_isJsonNull(struct _JsonObject This, char Key[]) {
    signed char ResOver = -1;
    char *KeyP = getJsonValueStart(This.JsonString, Key);
    if (KeyP != NULL) {
        ResOver = false;
        // 只比较前缀, 不再临时改写输入; strncmp 遇到 '\0' 会停止, 不会越界
        if ((strncmp(KeyP, "null", 4) == 0) || (strncmp(KeyP, "NULL", 4) == 0) || (strncmp(KeyP, "Null", 4) == 0)) {
            ResOver = true;
        }
    }
    return ResOver;
}
static int Obj_getInt(struct _JsonObject This, char Key[]) {
    char *KeyP = getJsonValueStart(This.JsonString, Key);
    if (KeyP != NULL) {
        return doneAsciiStrToAnyBaseNumberData(KeyP, 16);
        // return atol(KeyP);
    }
    return 0;
}
static double Obj_getDouble(struct _JsonObject This, char Key[]) {
    char *KeyP = getJsonValueStart(This.JsonString, Key);
    if (KeyP != NULL) {
        return atof(KeyP);
    }
    return 0.0;
}
static bool Obj_getBool(struct _JsonObject This, char Key[]) {
    bool ResBool = false;
    char *KeyP = getJsonValueStart(This.JsonString, Key);
    if (KeyP != NULL) {
        // 只比较前缀, 不再临时改写输入; "false" 需要比较 5 个字符
        if ((strncmp(KeyP, "true", 4) == 0) || (strncmp(KeyP, "TRUE", 4) == 0) || (strncmp(KeyP, "True", 4) == 0)) {
            ResBool = true;
        } else if ((strncmp(KeyP, "false", 5) == 0) || (strncmp(KeyP, "FALSE", 5) == 0) || (strncmp(KeyP, "False", 5) == 0)) {
            ResBool = false;
        }
    }
    return ResBool;
}
// 不支持原地转换, 避免破环 json 数据
static void Obj_getString(struct _JsonObject This, char Key[], strnew OutStr) {
    char *KeyP = getJsonValueStart(This.JsonString, Key);
    if (KeyP == NULL) {
        return;
    }
    if (*KeyP != '"') { // 值的首字符不是引号, 不是字符串
        return;
    }
    // 用 skipJsonString 跳过转义, 返回收尾引号后一位
    char *EndP = skipJsonString(This.JsonString, KeyP, false);
    if (EndP == NULL) {
        return;
    }
    char Temp = *(EndP - 1);
    *(EndP - 1) = '\0';
    memset(OutStr.Name._char, 0, OutStr.MaxLen);
    copyString(OutStr.Name._char, (KeyP + 1), OutStr.MaxLen, strlen(KeyP + 1));
    *(EndP - 1) = Temp;
}
// 注意输出地址与原json字符串地址一致时, 会破坏原数据
static struct _JsonArray Obj_getArray(struct _JsonObject This, char Key[], strnew OutStr) {
    JsonArray tempJsonArr = newJsonArrayByString(OutStr);
    char *KeyP = getJsonValueStart(This.JsonString, Key);
    if (KeyP != NULL) {
        char *EndP = NULL;
        if ((EndP = getDoubleChrOnString(This.JsonString, KeyP, '[', ']')) != NULL) {
            char Temp = *(EndP + 1);
            *(EndP + 1) = '\0';
            if (OutStr.Name._char != This.JsonString.Name._char) {
                memset(tempJsonArr.JsonString.Name._char, 0, tempJsonArr.JsonString.MaxLen);
                copyString(tempJsonArr.JsonString.Name._char, KeyP, tempJsonArr.JsonString.MaxLen, strlen(KeyP));
                *(EndP + 1) = Temp;
            } else {
                tempJsonArr.JsonString.Name._char = KeyP;
                tempJsonArr.JsonString.MaxLen = strlen(KeyP);
            }
        }
    }
    return tempJsonArr;
}
// 注意输出地址与原json字符串地址一致时, 会破坏原数据
static struct _JsonObject Obj_getObject(struct _JsonObject This, char Key[], strnew OutStr) {
    JsonObject tempJsonObj = newJsonObjectByString(OutStr);
    char *KeyP = getJsonValueStart(This.JsonString, Key);
    if (KeyP != NULL) {
        char *EndP = NULL;
        if ((EndP = getDoubleChrOnString(This.JsonString, KeyP, '{', '}')) != NULL) {
            char Temp = *(EndP + 1);
            *(EndP + 1) = '\0';
            if (OutStr.Name._char != This.JsonString.Name._char) {
                memset(tempJsonObj.JsonString.Name._char, 0, tempJsonObj.JsonString.MaxLen);
                copyString(tempJsonObj.JsonString.Name._char, KeyP, tempJsonObj.JsonString.MaxLen, strlen(KeyP));
                *(EndP + 1) = Temp;
            } else {
                tempJsonObj.JsonString.Name._char = KeyP;
                tempJsonObj.JsonString.MaxLen = strlen(KeyP);
            }
        }
    }
    return tempJsonObj;
}

JsonObject newJsonObjectByString(strnew DataInit) {
    JsonObject Temp;
    Temp.JsonString = DataInit;
    Temp.ItemNum = -1;
    Temp.sizeStr = Obj_sizeStr;
    Temp.isJsonNull = Obj_isJsonNull;
    Temp.getInt = Obj_getInt;
    Temp.getDouble = Obj_getDouble;
    Temp.getBool = Obj_getBool;
    Temp.getString = Obj_getString;
    Temp.getArray = Obj_getArray;
    Temp.getObject = Obj_getObject;
    return Temp;
}

//==========================================================================================//
//==========================================================================================//
#include "CheckCode.h"
#define CheckNum_T "CheckCode"
void addCsToJsonAndPushJsonStr(JsonObject InputJsonStrObj) {
    int AddrOver = strlen(InputJsonStrObj.JsonString.Name._char);
    if ((AddrOver + (int)strlen(",\"" CheckNum_T ":\":xxxxx") > InputJsonStrObj.JsonString.MaxLen) || (AddrOver < 1)) {
        return; // 空间不足以生成 cs 校验
    }
#if 0
    // 计算 cs 或其他检验算法
    uint16_t CheckNum = getModbusCrc(InputJsonStrObj.JsonString, AddrOver);
    addJsonItemData(InputJsonStrObj.JsonString, CheckNum_T ":%d", CheckNum);
    return;
#else
    uint8_t Sum = getSumCs(InputJsonStrObj.JsonString, AddrOver);
    addJsonItemData(InputJsonStrObj.JsonString, CheckNum_T ":%d", Sum);
    return;
#endif
}
bool checkOfCsJsonStrIsRight(strnew JsonInputStr, strnew JsonOutputStr) {
    JsonObject JsonObj = newJsonObjectByString(JsonInputStr);
    // 如果 NowCheckNum 不存在 直接退出
    if (JsonObj.isJsonNull(&JsonObj, CheckNum_T) < 0) {
        return false;
    }
    int NowCheckNum = JsonObj.getInt(&JsonObj, CheckNum_T);
    char *P_CheckStart = strstr(JsonObj.JsonString.Name._char, "\"" CheckNum_T "\":");
    char *P_CheckEnd = strstr(P_CheckStart, ",");
    if (P_CheckStart == NULL) {
        return false;
    }
    uint8_t CheckNumLocatFlag = 0;
    if (*(P_CheckStart - 1) == '{') { // 校验码在开头
        CheckNumLocatFlag = setDataBit(CheckNumLocatFlag, 0, 1);
    }
    if (P_CheckEnd == NULL) { // 校验码在结尾
        CheckNumLocatFlag = setDataBit(CheckNumLocatFlag, 1, 1);
    }

    if (CheckNumLocatFlag == 1) { // 校验码在开头
        (*P_CheckEnd) = '{';
        JsonInputStr.Name._char = P_CheckEnd;
        JsonInputStr.MaxLen -= (P_CheckEnd - P_CheckStart);
    } else if (CheckNumLocatFlag == 2) { // 校验码在结尾
        (*(P_CheckStart - 1)) = '}';
        (*P_CheckStart) = '\0';
    } else if (CheckNumLocatFlag == 3) { // 校验码在开头和结尾
        return false;
    } else {
        *(P_CheckStart) = '\0';
        catString(JsonObj.JsonString.Name._char, (P_CheckEnd + 1), JsonObj.JsonString.MaxLen, strlen((P_CheckEnd + 1)));
    }
    copyString(JsonOutputStr.Name._char, JsonInputStr.Name._char, JsonOutputStr.MaxLen, JsonInputStr.MaxLen);
    int DataLen = strlen(JsonOutputStr.Name._char);
    while (JsonOutputStr.Name._char[DataLen - 1] != '}' && JsonOutputStr.Name._char[DataLen - 1] != ']' && JsonOutputStr.Name._char[DataLen - 1] != '\0') {
        JsonOutputStr.Name._char[DataLen - 1] = '\0';
        DataLen--;
        if (DataLen == 0) {
            return false;
        }
    }
    // 计算 cs 或其他检验算法
#if 0
    JsonInputStr.Name._char[DataLen++] = (((uint16_t)(NowCheckNum)&0xFF00) >> 8);
    JsonInputStr.Name._char[DataLen++] = (((uint16_t)(NowCheckNum)&0x00FF) >> 0);
    JsonOutputStr.Name._char[DataLen] = '\0';
    return isModbusCrc(JsonInputStr, DataLen);
#else
    JsonOutputStr.Name._char[DataLen++] = NowCheckNum;
    JsonOutputStr.Name._char[DataLen] = '\0';
    return isSumCs(JsonOutputStr, DataLen);
#endif
}

/*
 * JSON 格式校验 (isJsonSrring)
 *
 * 规则:
 *  - 只接受对象/数组作为根(与协议帧一致), 允许前后空白
 *  - 严格校验: 双引号键名 / 数字语法 / true,false,null / \uXXXX 转义,
 *    不允许尾随逗号、前导 0、单引号、尾随多余内容
 *  - 字符串里不允许裸控制字符 (< 0x20)
 *  - 迭代状态机, 不使用递归; 嵌套层数上限 JSON_CHECK_MAX_DEPTH
 *  - 扫描统一用 strnew 的 MaxLen 作硬上限, 同时遇 '\0' 停止;
 *    文本真正末尾由 StrLib 的 CheckTextEnd() 取得
 */
#define JSON_CHECK_MAX_DEPTH 32

typedef struct {
    unsigned char CloseChr; // '}' 或 ']'
    unsigned char State;
} JsonCheckFrame;

#define JSON_CHECK_ST_FIRST 0u
#define JSON_CHECK_ST_KEY   1u
#define JSON_CHECK_ST_COLON 2u
#define JSON_CHECK_ST_VALUE 3u
#define JSON_CHECK_ST_AFTER 4u

// 校验 JSON 数字: 成功返回数字后一位
static char *jsonCheckNumber(char *P, strnew JsonInputStr) {
    if (P == NULL) {
        return NULL;
    }
    char *Limit = JsonInputStr.Name._char + JsonInputStr.MaxLen;
    if ((P >= Limit) || (*P == '\0')) {
        return NULL;
    }
    if (*P == '-') {
        P++;
    }
    if ((P >= Limit) || (*P == '\0') || (*P < '0') || (*P > '9')) {
        return NULL;
    }
    if (*P == '0') {
        P++;
        if ((P < Limit) && (*P != '\0') && (*P >= '0') && (*P <= '9')) {
            return NULL; // 不允许前导 0
        }
    } else {
        while ((P < Limit) && (*P != '\0') && (*P >= '0') && (*P <= '9')) {
            P++;
        }
    }
    if ((P < Limit) && (*P != '\0') && (*P == '.')) {
        P++;
        if ((P >= Limit) || (*P == '\0') || (*P < '0') || (*P > '9')) {
            return NULL;
        }
        while ((P < Limit) && (*P != '\0') && (*P >= '0') && (*P <= '9')) {
            P++;
        }
    }
    if ((P < Limit) && (*P != '\0') && ((*P == 'e') || (*P == 'E'))) {
        P++;
        if ((P < Limit) && (*P != '\0') && ((*P == '+') || (*P == '-'))) {
            P++;
        }
        if ((P >= Limit) || (*P == '\0') || (*P < '0') || (*P > '9')) {
            return NULL;
        }
        while ((P < Limit) && (*P != '\0') && (*P >= '0') && (*P <= '9')) {
            P++;
        }
    }
    return P;
}

// 校验整串(允许前后空白), Root 指定根类型
static bool jsonCheckRoot(strnew JsonInputStr, char Root) {
    if ((JsonInputStr.Name._char == NULL) || (JsonInputStr.MaxLen <= 0)) {
        return false;
    }
    char *Start = JsonInputStr.Name._char;
    char *Limit = Start + JsonInputStr.MaxLen;
    char *TextEnd = CheckTextEnd(JsonInputStr);
    if (TextEnd == NULL) {
        return false;
    }
    char *P = jsonSkipSpace(Start, JsonInputStr);
    if ((P >= Limit) || (*P != Root)) {
        return false;
    }

    JsonCheckFrame Frames[JSON_CHECK_MAX_DEPTH];
    int Depth = 0;

    Frames[Depth].CloseChr = (unsigned char)((Root == '{') ? '}' : ']');
    Frames[Depth].State = JSON_CHECK_ST_FIRST;
    Depth++;
    P++; // 跳过根开括号

    while (Depth > 0) {
        JsonCheckFrame *Top = &Frames[Depth - 1];
        P = jsonSkipSpace(P, JsonInputStr);
        if ((P >= Limit) || (*P == '\0')) {
            return false;
        }

        if (Top->State == JSON_CHECK_ST_AFTER) { // 一个值结束
            if (*P == (char)Top->CloseChr) {
                Depth--;
                P++;
                if (Depth > 0) {
                    Frames[Depth - 1].State = JSON_CHECK_ST_AFTER;
                }
                continue;
            }
            if (*P != ',') {
                return false;
            }
            P++;
            Top->State = (Top->CloseChr == (unsigned char)'}') ? JSON_CHECK_ST_KEY : JSON_CHECK_ST_VALUE;
            continue;
        }

        if ((Top->CloseChr == (unsigned char)'}') &&
            ((Top->State == JSON_CHECK_ST_FIRST) || (Top->State == JSON_CHECK_ST_KEY))) {
            if ((Top->State == JSON_CHECK_ST_FIRST) && (*P == '}')) { // 空对象
                Depth--;
                P++;
                if (Depth > 0) {
                    Frames[Depth - 1].State = JSON_CHECK_ST_AFTER;
                }
                continue;
            }
            if (*P != '"') {
                return false;
            }
            P = skipJsonString(JsonInputStr, P, true);
            if (P == NULL) {
                return false;
            }
            Top->State = JSON_CHECK_ST_COLON;
            continue;
        }

        if ((Top->CloseChr == (unsigned char)'}') && (Top->State == JSON_CHECK_ST_COLON)) {
            if (*P != ':') {
                return false;
            }
            P++;
            Top->State = JSON_CHECK_ST_VALUE;
            continue;
        }

        if ((Top->State == JSON_CHECK_ST_FIRST) && (*P == (char)Top->CloseChr)) { // 空数组
            Depth--;
            P++;
            if (Depth > 0) {
                Frames[Depth - 1].State = JSON_CHECK_ST_AFTER;
            }
            continue;
        }

        // 解析一个值 (ST_FIRST / ST_VALUE)
        switch (*P) {
        case '"':
            P = skipJsonString(JsonInputStr, P, true);
            if (P == NULL) {
                return false;
            }
            Top->State = JSON_CHECK_ST_AFTER;
            break;
        case '{':
        case '[':
            if (Depth >= JSON_CHECK_MAX_DEPTH) {
                return false; // 嵌套过深
            }
            Frames[Depth].CloseChr = (unsigned char)((*P == '{') ? '}' : ']');
            Frames[Depth].State = JSON_CHECK_ST_FIRST;
            Depth++;
            P++;
            break;
        case 't':
            if (((TextEnd - P) < 4) || (memcmp(P, "true", 4) != 0)) {
                return false;
            }
            P += 4;
            Top->State = JSON_CHECK_ST_AFTER;
            break;
        case 'f':
            if (((TextEnd - P) < 5) || (memcmp(P, "false", 5) != 0)) {
                return false;
            }
            P += 5;
            Top->State = JSON_CHECK_ST_AFTER;
            break;
        case 'n':
            if (((TextEnd - P) < 4) || (memcmp(P, "null", 4) != 0)) {
                return false;
            }
            P += 4;
            Top->State = JSON_CHECK_ST_AFTER;
            break;
        default:
            P = jsonCheckNumber(P, JsonInputStr);
            if (P == NULL) {
                return false;
            }
            Top->State = JSON_CHECK_ST_AFTER;
            break;
        }
    }

    P = jsonSkipSpace(P, JsonInputStr);
    return (P == TextEnd); // 不允许尾随多余内容
}

// 判断是否符合 json 格式
bool isJsonSrring(const strnew JsonInputStr) {
    if ((JsonInputStr.Name._char == NULL) || (JsonInputStr.MaxLen <= 0)) {
        return false;
    }
    char *P = jsonSkipSpace(JsonInputStr.Name._char, JsonInputStr);
    if (*P == '{') {
        return jsonCheckRoot(JsonInputStr, '{');
    } else if (*P == '[') {
        return jsonCheckRoot(JsonInputStr, '[');
    }
    return false;
}

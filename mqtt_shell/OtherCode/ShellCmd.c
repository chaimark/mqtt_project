#include "./ShellCmd.h"

strnew DoneCmdByMqttStr(strnew InputStr) {
    // strnew_malloc(CmdLen, InputStr.MaxLen);
    memset(InputStr.Name._char, 0, InputStr.MaxLen);
    strcpy(InputStr.Name._char, "res ok\n\0");
    return InputStr;
}

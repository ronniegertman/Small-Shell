#include "shellprompt.h"
#include "shellcommand.h"

ShellPrompt::ShellPrompt()
    :shellcmd("",{},false,0,0),isPromptDone(false),leftover(""){}
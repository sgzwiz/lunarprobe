
#ifndef _TEST_H_
#define _TEST_H_

#include <string>
#include <vector>
#include "lpmain.h"

LUNARPROBE_CPP_NAMESPACE_USE 

// The lua stacks that have been opened
struct NamedLuaStack
{
    bool            debugging;
    std::string     name;
    DebugContext *  pContext;
    LuaStack        pStack;
};

extern const char *strip_initial_spaces(const char *input);

extern NamedLuaStack *GetLuaStack(const std::string &name, bool add = false);

// Tells which mode we are in - either lua or command mode
// ctrl-d switches between the two modes
extern bool                            inCmdMode;

// The current stack that will process commands
extern int                             currStack;

// The list of stacks that have been created
extern std::vector<NamedLuaStack *>    luaStacks;

#endif

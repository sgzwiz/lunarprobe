
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "test.h"

bool processOpenCommand(const char *);
bool processCloseCommand(const char *);
bool processStackCommand(const char *);
bool processStacksCommand(const char *);
bool processLoadCommand(const char *);
bool processAttachCommand(const char *);
bool processDetachCommand(const char *);
bool processQuitCommand(const char *);

// Names of the commands
const char *CMD_NAMES[] =
{
    "open", "close", "stacks", "stack", "load", "attach", "detach", "quit"
};

// The functions to handle the commands
bool (*CMD_FUNCTIONS[])(const char *) =
{
    processOpenCommand,
    processCloseCommand,
    processStacksCommand,
    processStackCommand,
    processLoadCommand,
    processAttachCommand,
    processDetachCommand,
    processQuitCommand,
};

int argToUint(const char *arg)
{
    if (*arg == 0)
        return -1;

    int out = 0;
    while (*arg)
    {
        if (!isdigit(*arg))
            return -1;
        out = (out * 10) + (*arg - '0');
        arg++;
    }
    return out;
}

// Loads/Runs a file on the current stack
bool processLoadCommand(const char *args)
{
    if (currStack < 0)
    {
        fprintf(stderr, "No stacks selected.  Create or select a stack.\n");
    }
    else
    {
        LuaUtils::RunLuaScript(luaStacks[currStack]->pStack, args);
    }
    return true;
}

// opens a new lua stack
bool processOpenCommand(const char *args)
{
    if (*args == 0)
    {
        fprintf(stderr, "\nUsage: open    <stack_name>\n\n");
        return true;
    }

    NamedLuaStack *pStack = GetLuaStack(args, true);

    printf("\nLua stack created: %s - %p\n\n", pStack->name.c_str(), pStack->pStack);

    if (currStack < 0)
        currStack = 0;

    return true;
}

bool processCloseCommand(const char *args)
{
    int index = argToUint(args);
    if (index < 0 || index >= ((int)luaStacks.size()))
    {
        fprintf(stderr, "\nUsage: close   <stack_index>\n\n");
        return true;
    }

    NamedLuaStack *pStack = luaStacks[index];
    if (pStack->debugging)
    {
        LunarProbe::GetInstance()->Detach(pStack->pStack);
    }
    lua_close(pStack->pStack);
    luaStacks.erase(luaStacks.begin() + index);
    delete pStack;
    return true;
}

bool processStackCommand(const char *args)
{
    int index = argToUint(args);
    if (index < 0 || index >= ((int)luaStacks.size()))
    {
        fprintf(stderr, "\nUsage: set     <stack_index>\n\n");
        return true;
    }

    currStack   =   index;

    return true;
}

// Prints the list of stacks created so far
bool processStacksCommand(const char *args)
{
    puts("\nCurrent Stacks:");
    puts("===============");

    for (std::vector<NamedLuaStack *>::iterator iter = luaStacks.begin();
            iter != luaStacks.end(); ++iter)
    {
        printf("    %3d - %p - %s - %s\n",
                    iter - luaStacks.begin(),
                    (*iter)->pStack,
                    (*iter)->name.c_str(),
                    (*iter)->debugging ? "Attached" : "Detached");
    }
    puts("");
    return true;
}

// Starts debugging of a particular stack
bool processAttachCommand(const char *args)
{
    int index = argToUint(args);
    if (index < 0 || index >= ((int)luaStacks.size()))
    {
        fprintf(stderr, "\nUsage: attach  <stack_index>\n\n");
        return true;
    }

    NamedLuaStack *pNamedStack  = luaStacks[index];
    if (LunarProbe::GetInstance()->Attach(pNamedStack->pStack, pNamedStack->name.c_str()) >= 0)
    {
        pNamedStack->debugging = true;
        pNamedStack->pContext  = LunarProbe::GetInstance()->GetDebugger()->GetDebugContext(pNamedStack->pStack);
    }
    return true;
}

// Stops debugging of a particular stack
bool processDetachCommand(const char *args)
{
    int index = argToUint(args);
    if (index < 0 || index >= ((int)luaStacks.size()))
    {
        fprintf(stderr, "\nUsage: detach  <stack_index>\n\n");
        return true;
    }

    NamedLuaStack *pNamedStack  = luaStacks[index];
    LunarProbe::GetInstance()->Detach(pNamedStack->pStack);
    pNamedStack->debugging = false;
    pNamedStack->pContext  = NULL;
    return true;
}

// Quits the debugger test harness
bool processQuitCommand(const char *args)
{
    return false;
}


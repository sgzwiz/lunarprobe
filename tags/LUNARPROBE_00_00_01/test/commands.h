
#ifndef _COMMANDS_H_
#define _COMMANDS_H_

// List of commands
enum
{
    CMD_OPEN,
    CMD_CLOSE,
    CMD_STACKS,
    CMD_STACK,
    CMD_LOAD,
    CMD_ATTACH,
    CMD_DETACH,
    CMD_QUIT,
    CMD_COUNT
};

extern const char *CMD_NAMES[];
extern bool (*CMD_FUNCTIONS[])(const char *);

#endif


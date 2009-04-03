
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "test.h"
#include "commands.h"

#include "halley.h"
#include "DebugServer.h"
#include "BayeuxClientIface.h"

#include <readline/readline.h>
#include <readline/history.h>

// Tells which mode we are in - either lua or command mode
// ctrl-d switches between the two modes
bool                            inCmdMode   = true;

// The current stack that will process commands
int                             currStack   = -1;

// The list of stacks that have been created
std::vector<NamedLuaStack *>    luaStacks;

char *showPrompt(char *inputBuff = NULL)
{
#ifdef USING_READLINE
    char prompt[256];
    sprintf(prompt, " %s [ CurrStack = %d] >> ", (inCmdMode ? "cmd" : "lua"), currStack);
    return readline(prompt);
#else
    std::cout << std::endl << (inCmdMode ? " cmd" : " lua");
    if (currStack >= 0)
    {
        std::cout << " [ CurrStack = " << currStack << " ] >> ";
    }
    else
    {
        std::cout << " [ CurrStack = None ] >> ";
    }
    return fgets(inputBuff, 1024, stdin);
#endif
}

void prog_usage()
{
    puts("\nUsage: options <optional preample files>");
    puts("    Options: \n");
    puts("      -l | --lua      -   Specify folder containing the lua files.  Default: ./lua");
    puts("      -s | --static   -   Specify folder containing the static files (for http).  Default: ./static");
    puts("");
}

void command_usage()
{
    puts("\nCommands: \n");
    puts("    open  <name>      -   Creates a new lua stack with the given name.");
    puts("    close <index>     -   Closes the lua stack at the given index.");
    puts("    stack <index>     -   Sets the index-th stack as the 'current' lua stack.");
    puts("    stacks            -   Prints the list of stacks currently opened.");
    puts("    attach <index>    -   Enables debugging of the index-th lua stack.");
    puts("    detach <index>    -   Disables debugging of the index-th lua stack.");
    puts("    quit              -   Quites the test harness.");
    puts("    -----             -   Switch between cmd and lua mode - same as Ctrl-D.");
    puts("");
}

// Strips spaces from the front of a string and returns a pointer to the
// first non-space char
const char *strip_initial_spaces(const char *input)
{
    while (*input && isspace(*input))
        input ++;
    return input;
}

NamedLuaStack *GetLuaStack(const std::string &name, bool add)
{
    NamedLuaStack *stack = NULL;
    for (std::vector<NamedLuaStack *>::iterator iter = luaStacks.begin();
            stack == NULL && iter != luaStacks.end();
            ++iter)
    {
        if ((*iter)->name == name)
            stack = *iter;
    }

    if (stack == NULL && add)
    {
        stack               = new NamedLuaStack();
        stack->debugging    = false;
        stack->name         = name;
        stack->pStack       = LuaUtils::NewLuaStack(true, true, name.c_str());
        stack->pContext     = NULL;

        luaStacks.push_back(stack);
    }

    return stack;
}


//
// Available commands:
// open     <name>  -   open a new lua stack with the given name
// close    <index> -   closes a given lua stack
// stack    <index> -   Sets the specific stack as the "current" stack
// stacks           -   Print all the lua stacks
// load     <file>  -   Run a lua file in the current stack
//
bool processCmdString(const char *input)
{
    int cmd = 0;

    if (strncmp(input, "help", 4) == 0)
    {
        command_usage();
    }
    else
    {
        for (;cmd < CMD_COUNT;cmd++)
        {
            int cmdlen = strlen(CMD_NAMES[cmd]);
            if (strncmp(input, CMD_NAMES[cmd], cmdlen) == 0)
            {
                input += cmdlen;
                if (*input == 0 || isspace(*input))
                {
                    // command found
                    input = strip_initial_spaces(input);
                    break ;
                }
            }
        }

        if (cmd < CMD_COUNT)
        {
            return CMD_FUNCTIONS[cmd](input);
        }

        std::cerr << std::endl << "Invalid Command: '" << input << "'" << std::endl << std::endl;
        command_usage();
    }
    return true;
}

bool processLuaString(const char *input)
{
    if (currStack < 0)
    {
        fprintf(stderr, "No stacks selected.  Create or select a stack.\n");
    }
    else
    {
        LuaUtils::RunLuaString(luaStacks[currStack]->pStack, input);
    }
    return true;
}

void switchMode()
{
    inCmdMode = !inCmdMode;
    printf("\n\nSwitching to %s mode ...  Press Ctrl-C to exit...\n\n", inCmdMode ? "cmd" : "lua");
}

bool processCommand(char *command)
{
    command = (char *)strip_initial_spaces(command);

    char *end = command + strlen(command) - 1;
    while (end > command && (isspace(*end) || *end == '\n' || *end == '\r'))
        end--;
    end[1] = 0;

    bool running = true;
    if (*command != 0)
    {
        if (strncmp("-----", command, 5) == 0)
        {
            switchMode();
        }
        else
        {
            running = inCmdMode ? processCmdString(command) : processLuaString(command);
        }
    }
    return running;
}

void InitLunarProbe(const std::string &staticPath)
{
    class LPIndexModule : public SHttpModule
    {
    public:
        //! Constructor
        LPIndexModule(SHttpModule *pNext) : SHttpModule(pNext) { }

        //! Called to handle input data from another module
        void ProcessInput(SHttpHandlerData *    pHandlerData,
                          SHttpHandlerStage *   pStage,
                          SBodyPart *           pBodyPart)
        {
            SHttpRequest *pRequest      = pHandlerData->Request();
            SHttpResponse *pResponse    = pRequest->Response();
            SString links =
                    "<p>"
                    "<h2><a href='/ldb/index.html'>Lua Debugger</a></h2>"
                    "<br><h2><a href='/files/'>FileSystem</a></h2>"
                    ;
            SString body    = "<hr><center>"    + links + "</center><hr>";

            SBodyPart *part = pResponse->NewBodyPart();
            part->SetBody("<html><head><title>Lua Debugger</title></head>");
            part->AppendToBody("<body>" + body + "</body></html>");

            pStage->OutputToModule(pHandlerData->pConnection, pNextModule, part);
            pStage->OutputToModule(pHandlerData->pConnection, pNextModule,
                                   pResponse->NewBodyPart(SBodyPart::BP_CONTENT_FINISHED, pNextModule));
        }
    };

    static LUNARPROBE_NS::BayeuxClientIface clientIface("/ldb");
    static LUNARPROBE_NS::HttpDebugServer   debugServer(LUA_DEBUG_PORT, &clientIface);
    static SThread                          serverThread(&debugServer);
    SUrlRouter *                            pUrlRouter = debugServer.GetUrlRouter();
    SFileModule *                           pFileModule = debugServer.GetFileModule();

    static LPIndexModule lpIndexModule(debugServer.GetContentModule());
    static SFileModule gameFilesModule(debugServer.GetContentModule());
    static SContainsUrlMatcher gameFilesUrlMatch("/files/", SContainsUrlMatcher::PREFIX_MATCH, &gameFilesModule);
    static SContainsUrlMatcher ldbUrlMatch("/ldb/", SContainsUrlMatcher::PREFIX_MATCH, pFileModule);
    static SContainsUrlMatcher bayeuxUrlMatch("/bayeux/", SContainsUrlMatcher::PREFIX_MATCH, debugServer.GetBayeuxModule());


    LUNARPROBE_NS::LunarProbe *lpInstance = LUNARPROBE_NS::LunarProbe::GetInstance();
    if (lpInstance->GetClientIface() == NULL)
    {
        // set the different modules we need
        gameFilesModule.AddDocRoot("/files/", "/home/spanyam/");
        pUrlRouter->AddUrlMatch(&gameFilesUrlMatch);

        pFileModule->AddDocRoot("/ldb/", staticPath);
        pUrlRouter->AddUrlMatch(&ldbUrlMatch);

        pUrlRouter->AddUrlMatch(&bayeuxUrlMatch);

        pUrlRouter->SetNextModule(&lpIndexModule);

        lpInstance->SetClientIface(&clientIface);
        serverThread.Start();
    }
}

LUNARPROBE_NS::LunarProbe *GetLPInstance()
{
    return LUNARPROBE_NS::LunarProbe::GetInstance();
}

int main(int argc, char *argv[])
{
    char pathBuff[1024];
    char inputBuff[1024];
    char *command;
    const char *luaPath = "./lua";
    const char *staticPath = "./static";
    bool running = true;

    int argCounter = 1;
    for (;argCounter < argc;argCounter++)
    {
        if (strcmp(argv[argCounter], "--lua") == 0   ||
            strcmp(argv[argCounter], "-lua") == 0    ||
            strcmp(argv[argCounter], "-l") == 0)
        {
            luaPath = argv[++argCounter];
        }
        else if (strcmp(argv[argCounter], "--help") == 0 ||
                 strcmp(argv[argCounter], "-help") == 0     ||
                 strcmp(argv[argCounter], "-h") == 0)
        {
            prog_usage();
            return 0;
        }
        else if (strcmp(argv[argCounter], "--static") == 0 ||
                 strcmp(argv[argCounter], "-static") == 0     ||
                 strcmp(argv[argCounter], "-s") == 0)
        {
            staticPath = argv[++argCounter];
        }
        else
        {
            break ;
        }
    }

    LuaBindings::LUA_SRC_LOCATION = realpath(luaPath, pathBuff);

    InitLunarProbe(realpath(staticPath, pathBuff));

    for (;argCounter < argc;argCounter++)
    {
        FILE *fptr = fopen(argv[argCounter], "r");
        if (fptr == NULL)
        {
            fprintf(stderr, "\nCannot open file: %s\n\n", argv[1]);

            // TODO: Should we quit here?
        }
        else
        {
            while ((command = fgets(inputBuff, 1024, fptr)) != NULL)
            {
                processCommand(command);
            }

            fclose(fptr);
        }
    }

    while (running)
    {
        if ((command = showPrompt(inputBuff)) == NULL)
        {
            switchMode();
        }
        else
        { 
            processCommand(command);
        }
    }

    return 0;
}


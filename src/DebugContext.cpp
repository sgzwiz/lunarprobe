/*****************************************************************************/
/*!
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************
 *
 *  \file   DebugContext.cpp
 *
 *  \brief  Implementation of DebugContext
 *
 *  \version
 *      - S Panyam   05/12/2008
 *      Initial version.
 */
//*****************************************************************************

#include <iostream> 
#include <assert.h> 
#include <stdio.h> 

#include "LuaUtils.h"
#include "Debugger.h"
#include "DebugContext.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \brief  Creates a new debugger context.
 *
 *  \version
 *      - S Panyam  23/10/2008
 *      Initial version.
 */
//*****************************************************************************
DebugContext::DebugContext(LuaStack luaStack, const char *n) :
    running(true),
    pStack(luaStack),
    name(n ? n : ""),
    pausedCond(runStateMutex),
    pDebug(NULL)
{ 
}

//*****************************************************************************
/*!
 *  \brief  Pauses the processing of a lua stack.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
bool DebugContext::Pause(LuaDebug pDbg)
{
    CMutexLock mutexLock(runStateMutex);
    if (running)
    {
        running = false;
        pDebug  = pDbg;
    }
    else
    {
        assert(false && "Already paused.  Cannot pause again.");
    }
    return !running;
}

//*****************************************************************************
/*!
 *  \brief  Resumes the processing of a lua stack.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
bool DebugContext::Resume()
{
    CMutexLock mutexLock(runStateMutex);

    if (!running)
    {
        running = true;

        pDebug  = NULL;

        // signal that we have unpaused!
        pausedCond.Signal();
    }

    return true;
}

//*****************************************************************************
/*!
 *  \brief  Blocks till the processing is paused.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
int DebugContext::WaitWhilePaused()
{
    CMutexLock mutexLock(runStateMutex);

    return running ? 0 : pausedCond.Wait();
}

//*****************************************************************************
/*!
 *  \brief  Loads a file (only if this we are not running).
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
bool DebugContext::LoadFile(const char *filename)
{
    if (!running)
    {
        fprintf(stderr, "====================================\n");
        fprintf(stderr, "Cannot run %s - It is still running.\n", filename);
        return false;
    }

    return LuaUtils::RunLuaScript(pStack, filename) == 0;
}

LUNARPROBE_NS_END


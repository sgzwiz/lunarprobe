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
 *  \file   BayeuxDebugger.h
 *
 *  \brief  A bayeux (comet) implementation of the debugger.
 *
 *  \version
 *        - S Panyam  30/03/2009
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _BAYEUX_DEBUGGER_H_
#define _BAYEUX_DEBUGGER_H_

#include "Debugger.h"
#include "eds/http/bayeux/channel.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  BayeuxDebugger
 *
 *  \brief  A bayeux (comet) implementation of the debugger.
 *****************************************************************************/
class BayeuxDebugger : public Debugger, public SBayeuxChannel
{
public:
    virtual int     SendMessage(const char *data, unsigned datasize);

    //! Handles an event.
    virtual void HandleEvent(const JsonNodePtr &event, JsonNodePtr &output) { }
};

LUNARPROBE_NS_END

#endif


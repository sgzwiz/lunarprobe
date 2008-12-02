//*****************************************************************************
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
 *  \file   MutexUtils.cpp
 *
 *  \brief  Utilities to allow easier access to mutexes and condition
 *  variables.
 *
 *  \version
 *      - S Panyam      28/10/2008
 *        Created
 *
 *****************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <signal.h>

// Enable for doing timing measurements
// #include "PCC.h"

#include "MutexUtils.h"

LUNARPROBE_CPP_NAMESPACE_BEGIN

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
CMutex::CMutex()
{
    pthread_mutex_init(&mutex, NULL);
}

//*****************************************************************************
/*!
 *  \brief  Constructor for creating mutexes of "other" kinds - eg
 *  recursive and so on.
 *
 *  \version
 *      - S Panyam     24/11/2008
 *        Initial Version
 *
 *****************************************************************************/
CMutex::CMutex(int kind)
{
    mutex_attr = new pthread_mutexattr_t;
    pthread_mutexattr_init(mutex_attr);
    pthread_mutexattr_settype(mutex_attr, kind);
    pthread_mutex_init(&mutex, mutex_attr);
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
CMutex::~CMutex()
{
    pthread_mutex_destroy(&mutex);
}

//*****************************************************************************
/*!
 *  \brief  Aquires a lock on the mutex.  Blocks until the lock is
 *  provided.
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
void CMutex::Lock()
{
    pthread_mutex_lock(&mutex);
}

//*****************************************************************************
/*!
 *  \brief  Releases the lock on the mutex.
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
void CMutex::Unlock()
{
    pthread_mutex_unlock(&mutex);
}

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
CMutexLock::CMutexLock(CMutex &mutex) : pTheMutex(&mutex)
{
    pTheMutex->Lock();
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
CMutexLock::~CMutexLock()
{
    pTheMutex->Unlock();
}

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
CCondition::CCondition(CMutex &mutex) : pMutex(&mutex)
{
    pthread_cond_init(&condVar, NULL);
}

//*****************************************************************************
/*!
 *  \brief  Destructor
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
CCondition::~CCondition()
{
    pthread_cond_destroy(&condVar);
}

//*****************************************************************************
/*!
 *  \brief  Waits on the condition variable.
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
int CCondition::Wait()
{
    return pthread_cond_wait(&condVar, &pMutex->mutex);
}

//*****************************************************************************
/*!
 *  \brief  Signals the cond var to wake up.
 *
 *  \version
 *      - S Panyam     28/10/2008
 *        Initial Version
 *
 *****************************************************************************/
int CCondition::Signal()
{
    return pthread_cond_signal(&condVar);
}

LUNARPROBE_CPP_NAMESPACE_END


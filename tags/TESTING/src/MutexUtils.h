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
 *  \file   MutexUtils.h
 *
 *  \brief  Utilities to allow easier access to mutexes and condition
 *  variables.
 *
 *  \version
 *      - S Panyam      28/10/2008
 *        Initial Version
 *
 *****************************************************************************/

#ifndef _MUTEXUTILS_H_
#define _MUTEXUTILS_H_

#include <pthread.h>
#include "lpnsdefs.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  CMutex
 *
 *  \brief  A mutual exclusive object.
 *
 *****************************************************************************/
class CMutex
{
friend class CMutexLock;
friend class CCondition;

public:
    CMutex();
    CMutex(int kind);
    ~CMutex();
    void Lock();
    void Unlock();

private:
    pthread_mutex_t mutex;
    pthread_mutexattr_t *mutex_attr;
};

//*****************************************************************************
/*!
 *  \class  CMutexLock
 *
 *  \brief  A wrapper for locking and unlocking mutual exclusive objects.
 *
 *****************************************************************************/
class CMutexLock
{
public:
    CMutexLock(CMutex &mutex);
    ~CMutexLock();

private:
    CMutexLock();
    CMutex *pTheMutex;
};

//*****************************************************************************
/*!
 *  \class  CCondition
 *
 *  \brief  A wrapper for a condition variable.
 *
 *****************************************************************************/
class CCondition
{
public:
    CCondition(CMutex &mutex);
    ~CCondition();
    int     Wait();
    int     Signal();

private:
    pthread_cond_t  condVar;
    //! The mutex this cond var would be waiting on.
    CMutex *pMutex;
};

LUNARPROBE_NS_END

#endif


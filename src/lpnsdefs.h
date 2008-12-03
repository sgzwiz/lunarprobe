
//***************************************************************************
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
 *  \file   lpnsdefs.h
 *
 *  \brief  Definition of namespaces of LP.
 *
 *  \version
 *      - S Panyam  05/12/2008
 *        Initial version
 *
 *****************************************************************************/

#ifndef _LUNARPROBE_NS_DEFS_H_
#define _LUNARPROBE_NS_DEFS_H_

#include "lpversion.h"

#if defined(LUNARPROBE_HAS_NS)
    #define LUNARPROBE_NS_BEGIN         namespace   LUNARPROBE_NS    { 
    #define LUNARPROBE_NS_END           }
    #define LUNARPROBE_NS_USE           using namespace LUNARPROBE_NS;
    #define LUNARPROBE_NS_QUALIFIER     LUNARPROBE_NS::

    namespace LUNARPROBE_NS { }
    namespace lunarprobe = LUNARPROBE_NS;
#else
    #define LUNARPROBE_NS_BEGIN
    #define LUNARPROBE_NS_END
    #define LUNARPROBE_NS_USE
    #define LUNARPROBE_NS_QUALIFIER
#endif

#endif



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

#if defined(LUNARPROBE_HAS_CPP_NAMESPACE)
    #define LUNARPROBE_CPP_NAMESPACE_BEGIN      namespace   LUNARPROBE_CPP_NAMESPACE    { 
    #define LUNARPROBE_CPP_NAMESPACE_END        }
    #define LUNARPROBE_CPP_NAMESPACE_USE        using namespace LUNARPROBE_CPP_NAMESPACE;
    #define LUNARPROBE_CPP_NAMESPACE_QUALIFIER  LUNARPROBE_CPP_NAMESPACE::

    namespace LUNARPROBE_CPP_NAMESPACE { }
    namespace lunarprobe = LUNARPROBE_CPP_NAMESPACE;
#else
    #define LUNARPROBE_CPP_NAMESPACE_BEGIN
    #define LUNARPROBE_CPP_NAMESPACE_END
    #define LUNARPROBE_CPP_NAMESPACE_USE
    #define LUNARPROBE_CPP_NAMESPACE_QUALIFIER
#endif

#endif


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
 *  \file   lpversion.h
 *
 *  \brief  Definition of all version and namespace related items for
 *  LunarProbe.  This has been taken directly from the Xerces project with
 *  changes to a few project and global names.
 *
 * User Documentation for LunarProbe Version Values:
 *
 * LunarProbe Notes:
 *
 * LunarProbe Committers Documentation:
 *
 *    Normally only need to modify one or two of the following macros:
 *
 *      LUNARPROBE_VERSION_MAJOR
 *      LUNARPROBE_VERSION_MINOR
 *      LUNARPROBE_VERSION_REVISION
 *
 *    The integer values of these macros define the LunarProbe version number. All
 *    other constants and preprocessor macros are automatically generated from
 *    these three definitions.
 *
 *    The macro LUNARPROBE_GRAMMAR_SERIALIZATION_LEVEL has been added so that during
 *    development if users are using the latest code they can use the grammar
 *    serialization/desialization features.  Whenever a change is made to the
 *    serialization code this macro should be incremented.
 *
 * LunarProbe User Documentation:
 *
 *  The following sections in the user documentation have examples based upon
 *  the following three version input values:
 *
 *    #define LUNARPROBE_VERSION_MAJOR 19
 *    #define LUNARPROBE_VERSION_MINOR 3
 *    #define LUNARPROBE_VERSION_REVISION 74
 *
 *  The minor and revision (patch level) numbers have two digits of resolution
 *  which means that '3' becomes '03' in this example. This policy guarantees
 *  that when using preprocessor macros, version 19.3.74 will be greater than
 *  version 1.94.74 since the first will expand to 190374 and the second to
 *  19474.
 *
 *  Preprocessor Macros:
 *
 *    _LUNARPROBE_VERSION defines the primary preprocessor macro that users will
 *    introduce into their code to perform conditional compilation where the
 *    version of LunarProbe is detected in order to enable or disable version
 *    specific capabilities. The value of _LUNARPROBE_VERSION for the above example
 *    will be 190374. To use it a user would perform an operation such as the
 *    following:
 *
 *      #if _LUNARPROBE_VERSION >= 190374
 *        // code specific to new version of LunarProbe ...
 *      #else
 *        // old code here...
 *      #endif
 *
 *    LUNARPROBE_FULLVERSIONSTR is a preprocessor macro that expands to a string
 *    constant whose value, for the above example, will be "19_3_74".
 *
 *    LUNARPROBE_FULLVERSIONDOT is a preprocessor macro that expands to a string
 *    constant whose value, for the above example, will be "19.3.74".
 *
 *    LUNARPROBE_VERSIONSTR is a preprocessor macro that expands to a string
 *    constant whose value, for the above example, will be "19_3". This
 *    particular macro is very dangerous if it were to be used for comparing
 *    version numbers since ordering will not be guaranteed.
 *
 *  String Constants:
 *
 *    LUNARPROBE_VERSION_STR is a global string constant whose value corresponds to
 *    the value "19_3" for the above example.
 *
 *    LUNARPROBE_FULL_VERSION_STR is a global string constant whose value corresponds
 *    to the value "19_3_74" for the above example.
 *
 *  Numeric Constants:
 *
 *    LUNARPROBE_MAJOR_VERSION is a global integer constant whose value corresponds to
 *    the major version number. For the above example its value will be 19.
 *
 *    LUNARPROBE_MINOR_VERSION is a global integer constant whose value corresponds to
 *    the minor version number. For the above example its value will be 3.
 *
 *    LUNARPROBE_REVISION is a global integer constant whose value corresponds to
 *    the revision (patch) version number. For the above example its value will
 *    be 74.
 *
 *  \version
 *      - S Panyam  05/12/2008
 *        Initial version
 *
 *****************************************************************************/

#ifndef _LUNARPROBE_VERSION_H_
#define _LUNARPROBE_VERSION_H_

//
// Only modify these three numbers in this file and nothing else
//
#define LUNARPROBE_VERSION_MAJOR    0
#define LUNARPROBE_VERSION_MINOR    0
#define LUNARPROBE_VERSION_REVISION 1


//////////////////////////////////////////////////////////////////////////////
//                    Do not modify anything else below.                    //
//////////////////////////////////////////////////////////////////////////////

#ifndef LUNARPROBE_HAS_CPP_NAMESPACE
#define LUNARPROBE_HAS_CPP_NAMESPACE    1
#endif

// ---------------------------------------------------------------------------
// T W O   A R G U M E N T   C O N C A T E N A T I O N   M A C R O S
// Courtesy of the Xerces project

// two argument concatenation routines
#define CAT2_SEP_UNDERSCORE(a, b) #a "_" #b
#define CAT2_SEP_PERIOD(a, b) #a "." #b
#define CAT2_SEP_NIL(a, b) #a #b
#define CAT2_RAW_NUMERIC(a, b) a ## b

// two argument macro invokers
#define INVK_CAT2_SEP_UNDERSCORE(a,b) CAT2_SEP_UNDERSCORE(a,b)
#define INVK_CAT2_SEP_PERIOD(a,b)     CAT2_SEP_PERIOD(a,b)
#define INVK_CAT2_STR_SEP_NIL(a,b)    CAT2_SEP_NIL(a,b)
#define INVK_CAT2_RAW_NUMERIC(a,b)    CAT2_RAW_NUMERIC(a,b)

// ---------------------------------------------------------------------------
// T H R E E   A R G U M E N T   C O N C A T E N A T I O N   M A C R O S
// Courtesy of the Xerces project

// three argument concatenation routines
#define CAT3_SEP_UNDERSCORE(a, b, c) #a "_" #b "_" #c
#define CAT3_SEP_PERIOD(a, b, c) #a "." #b "." #c
#define CAT3_SEP_NIL(a, b, c) #a #b #c
#define CAT3_RAW_NUMERIC(a, b, c) a ## b ## c
#define CAT3_RAW_NUMERIC_SEP_UNDERSCORE(a, b, c) a ## _ ## b ## _ ## c

// three argument macro invokers
#define INVK_CAT3_SEP_UNDERSCORE(a,b,c) CAT3_SEP_UNDERSCORE(a,b,c)
#define INVK_CAT3_SEP_PERIOD(a,b,c)     CAT3_SEP_PERIOD(a,b,c)
#define INVK_CAT3_SEP_NIL(a,b,c)        CAT3_SEP_NIL(a,b,c)
#define INVK_CAT3_RAW_NUMERIC(a,b,c)    CAT3_RAW_NUMERIC(a,b,c)
#define INVK_CAT3_RAW_NUMERIC_SEP_UNDERSCORE(a,b,c)    CAT3_RAW_NUMERIC_SEP_UNDERSCORE(a,b,c)

// ---------------------------------------------------------------------------
// C A L C U L A T E   V E R S I O N   -   E X P A N D E D   F O R M

#define MULTIPLY(factor,value) factor * value
#define CALC_EXPANDED_FORM(a,b,c) ( MULTIPLY(10000,a) + MULTIPLY(100,b) + MULTIPLY(1,c) )

// ---------------------------------------------------------------------------
// L U N A R P R O B E      V E R S I O N   I N F O R M A T I O N

// LunarProbe version strings; these particular macros cannot be used for
// conditional compilation as they are not numeric constants

#define LUNARPROBE_FULLVERSIONSTR INVK_CAT3_SEP_UNDERSCORE(LUNARPROBE_VERSION_MAJOR,LUNARPROBE_VERSION_MINOR,LUNARPROBE_VERSION_REVISION)
#define LUNARPROBE_FULLVERSIONDOT INVK_CAT3_SEP_PERIOD(LUNARPROBE_VERSION_MAJOR,LUNARPROBE_VERSION_MINOR,LUNARPROBE_VERSION_REVISION)
#define LUNARPROBE_FULLVERSIONNUM INVK_CAT3_SEP_NIL(LUNARPROBE_VERSION_MAJOR,LUNARPROBE_VERSION_MINOR,LUNARPROBE_VERSION_REVISION)
#define LUNARPROBE_VERSIONSTR     INVK_CAT2_SEP_UNDERSCORE(LUNARPROBE_VERSION_MAJOR,LUNARPROBE_VERSION_MINOR)

// LunarProbe C++ Namespace string, concatenated with full version string
#define LUNARPROBE_PRODUCT  lunarprobe
#define LUNARPROBE_CPP_NAMESPACE INVK_CAT3_RAW_NUMERIC_SEP_UNDERSCORE(LUNARPROBE_PRODUCT,LUNARPROBE_VERSION_MAJOR,LUNARPROBE_VERSION_MINOR)

const char* const    LUNARPROBE_VERSION_STR         = LUNARPROBE_VERSIONSTR;
const char* const    LUNARPROBE_FULL_VERSION_STR    = LUNARPROBE_FULLVERSIONSTR;
const unsigned int   LUNARPROBE_MAJOR_VERSION       = LUNARPROBE_VERSION_MAJOR;
const unsigned int   LUNARPROBE_MINOR_VERSION       = LUNARPROBE_VERSION_MINOR;
const unsigned int   LUNARPROBE_REVISION            = LUNARPROBE_VERSION_REVISION;

// LunarProbe version numeric constants that can be used for conditional
// compilation purposes.

#define _LUNARPROBE_VERSION CALC_EXPANDED_FORM (LUNARPROBE_VERSION_MAJOR,LUNARPROBE_VERSION_MINOR,LUNARPROBE_VERSION_REVISION)

#endif


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

#include "DebugServer.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \brief  Create a HTTP based debug server.
 *
 *  \version
 *      - S Panyam  01/04/2009
 *      Initial version.
 */
//*****************************************************************************
HttpDebugServer::HttpDebugServer(int                    port,
                                 BayeuxClientIface *    pIface,
                                 const std::string &    msgBoundary) :
    SEvServer(port),
    pClientIface(pIface),
    requestReader("Reader", 0),
    requestHandler("Handler", 0),
    requestWriter("Writer", 0),
    bayeuxModule(&contentModule, msgBoundary),
    fileModule(&contentModule, true),
    urlRouter(NULL)
{
    SetReaderStage(&requestReader);
    SetWriterStage(&requestWriter);
    SetStage("RequestReader", &requestReader);
    SetStage("RequestHandler", &requestHandler);
    SetStage("RequestWriter", &requestWriter);

    requestReader.SetHandlerStage(&requestHandler);
    requestHandler.SetReaderStage(&requestReader);
    requestHandler.SetWriterStage(&requestWriter);
    requestHandler.SetRootModule(&urlRouter);

    assert("BayeuxClientIface is NULL." && (pIface != NULL));

    pClientIface->SetBayeuxModule(&bayeuxModule);
    bayeuxModule.RegisterChannel(pClientIface);

    // add more files and all that here to enable static html/js/css files
}

//*****************************************************************************
/*!
 *  \brief  No-op destructor
 *
 *  \version
 *      - S Panyam  01/04/2009
 *      Initial version.
 */
//*****************************************************************************
HttpDebugServer::~HttpDebugServer()
{
}

LUNARPROBE_NS_END


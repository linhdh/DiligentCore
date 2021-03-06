/*     Copyright 2015-2018 Egor Yusov
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence), 
 *  contract, or otherwise, unless required by applicable law (such as deliberate 
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental, 
 *  or consequential damages of any character arising as a result of this License or 
 *  out of the use or inability to use the software (including but not limited to damages 
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and 
 *  all other commercial damages or losses), even if such Contributor has been advised 
 *  of the possibility of such damages.
 */

#pragma once

#include "EngineGLAttribs.h"
#include "SwapChainGL.h"
#include "SwapChainBase.h"
#include "GLObjectWrapper.h"

namespace Diligent
{

class IMemoryAllocator;
/// Implementation of the Diligent::ISwapChainGL interface on IOS
class SwapChainGLIOS final :  public SwapChainBase<ISwapChainGL>
{
public:
    typedef SwapChainBase<ISwapChainGL> TSwapChainBase;
    
    SwapChainGLIOS(IReferenceCounters *pRefCounters,
                    const EngineGLAttribs &InitAttribs,
                    const SwapChainDesc& SwapChainDesc,
                    class RenderDeviceGLImpl* pRenderDeviceGL,
                    class DeviceContextGLImpl* pImmediateContextGL);
    SwapChainGLIOS();
    
    virtual void QueryInterface( const Diligent::INTERFACE_ID &IID, IObject **ppInterface )override final;
    
    virtual void Present()override final;
    
    virtual void Resize( Uint32 NewWidth, Uint32 NewHeight )override final;
    
    virtual GLuint GetDefaultFBO()const override final;
    
private:
    void InitRenderBuffers(bool InitFromDrawable, Uint32 &Width, Uint32 &Height);
    
    GLObjectWrappers::GLRenderBufferObj m_ColorRenderBuffer;
    GLObjectWrappers::GLRenderBufferObj m_DepthRenderBuffer;
    GLObjectWrappers::GLFrameBufferObj m_DefaultFBO;
    void  *m_CALayer;
};

}

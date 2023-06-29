#include "cbase.h"
#include "materialsystem\imaterialsystem.h"
#include "rendertexture.h"
#include "scope_rendertarget.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
ITexture* CScopeRenderTarget::CreateScopeTexture( IMaterialSystem* pMaterialSystem )
{
//	DevMsg("Creating Scope Render Target: _rt_Scope\n");
	return pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Scope",
		1024, 1024, RT_SIZE_OFFSCREEN,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED, 
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR );
}

//-----------------------------------------------------------------------------
// Purpose: Called by the engine in material system init and shutdown.
//			Clients should override this in their inherited version, but the base
//			is to init all standard render targets for use.
// Input  : pMaterialSystem - the engine's material system (our singleton is not yet inited at the time this is called)
//			pHardwareConfig - the user hardware config, useful for conditional render target setup
//-----------------------------------------------------------------------------
void CScopeRenderTarget::InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig )
{ 
	m_ScopeTexture.Init( CreateScopeTexture( pMaterialSystem ) ); 

	m_bScopeTextureActive = false;
	m_iScopeTextureFOV = 10;

	// Water effects & camera from the base class (standard HL2 targets) 
	BaseClass::InitClientRenderTargets( pMaterialSystem, pHardwareConfig );
}
  
//-----------------------------------------------------------------------------
// Purpose: Shut down each CTextureReference we created in InitClientRenderTargets.
//			Called by the engine in material system shutdown.
// Input  :  - 
//-----------------------------------------------------------------------------
void CScopeRenderTarget::ShutdownClientRenderTargets()
{ 
	m_ScopeTexture.Shutdown();
  
	// Clean up standard HL2 RTs (camera and water) 
	BaseClass::ShutdownClientRenderTargets();
}
 
//add the interface!
static CScopeRenderTarget g_ScopeRenderTarget;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CScopeRenderTarget, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_ScopeRenderTarget );
CScopeRenderTarget* ScopeRenderTarget = &g_ScopeRenderTarget;

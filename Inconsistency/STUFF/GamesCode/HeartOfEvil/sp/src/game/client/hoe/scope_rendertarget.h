#ifndef SCOPE_RENDERTARGET_H_
#define SCOPE_RENDERTARGET_H_
#ifdef _WIN32
#pragma once
#endif
 
#include "baseclientrendertargets.h" // Base class, with interfaces called by engine and inherited members to init common render   targets
 
// externs
class IMaterialSystem;
class IMaterialSystemHardwareConfig;
 
class CScopeRenderTarget : public CBaseClientRenderTargets
{ 
	// no networked vars 
	DECLARE_CLASS_GAMEROOT( CScopeRenderTarget, CBaseClientRenderTargets );
public: 
	virtual void InitClientRenderTargets( IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig );
	virtual void ShutdownClientRenderTargets();
 
	ITexture* CreateScopeTexture( IMaterialSystem* pMaterialSystem );
	bool IsScopeTextureActive( void ) { return m_bScopeTextureActive; }
	void SetScopeTextureActive( bool active ) { m_bScopeTextureActive = active; }
	int GetScopeTextureFOV( void ) { return m_iScopeTextureFOV; }
	void SetScopeTextureFOV( int fov ) { m_iScopeTextureFOV = fov; }

private:
	CTextureReference		m_ScopeTexture;
	bool					m_bScopeTextureActive;
	int						m_iScopeTextureFOV;
};
 
extern CScopeRenderTarget* ScopeRenderTarget;
 
#endif //TNERENDERTARGETS_H_

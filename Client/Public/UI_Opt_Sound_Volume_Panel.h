#pragma once

#include "UI_Opt_Sound.h"

BEGIN(Client)

class CUI_Opt_Sound_Volume_Panel final :public CUI_Opt_Sound
{

private:
	CUI_Opt_Sound_Volume_Panel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Opt_Sound_Volume_Panel(const CUI_Opt_Sound_Volume_Panel& Prototype);
	virtual ~CUI_Opt_Sound_Volume_Panel() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Camera_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render(_float fTimeDelta);

private:
	virtual HRESULT Ready_Components();

private:
	void PostionUpdate();
	void CreateNameFont();

private:
	_uint m_iNumUI = { 0 };

public:
	static CUI_Opt_Sound_Volume_Panel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END
#pragma once

#include "UIObject.h"

BEGIN(Client)

class CUI_Chara_SubIcon final :public CUIObject
{
private:
	CUI_Chara_SubIcon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Chara_SubIcon(const CUI_Chara_SubIcon& Prototype);
	virtual ~CUI_Chara_SubIcon() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Camera_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render(_float fTimeDelta);

private:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	_uint m_iCharaID = { 0 };
	_uint  m_iTeamIndex = { 0 };

public:
	static CUI_Chara_SubIcon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg);
	virtual void Free() override;
};

END
#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_VS final : public CLevel
{
private:
	CLevel_VS(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_VS() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

private:
	/* �� ���ӳ��� ����� �����ϴ� ��ü���� �߰��Ѵ�. */
	HRESULT Ready_Layer_BackGround(const _wstring& strLayerTag);
	HRESULT Ready_Sound();

private:
	_float m_fUILightCreateTimer = { 0.f };
	_float m_fNextLevelTimer = { 0.f };
public:
	static CLevel_VS* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

END
#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CQTE_Continuous_Attack_Space final : public CGameObject
{
public:
	struct CONTINUOUS_ATTACK_DESC
	{
		_float	fSizeX{}, fSizeY{}, fX{}, fY{};
	};

private:
	CQTE_Continuous_Attack_Space(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE_Continuous_Attack_Space(const CQTE_Continuous_Attack_Space& Prototype);
	virtual ~CQTE_Continuous_Attack_Space() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

	void Process_Command();
public:
	_float					m_fSizeX{}, m_fSizeY{}, m_fX{}, m_fY{};
	_float4x4				m_ViewMatrix{}, m_ProjMatrix{};

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();
	void Update_Animation(_float fTimeDelta);

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	// ��ġ ���� ����
	_float m_fDefaultY = {  };  // �⺻ ��ġ
	_float m_fTargetY = {  };    // Ÿ�� ��ġ

	// �ִϸ��̼� ���� ����
	_float m_fMoveDownTime = 0.1f;    // �������� �� �ɸ��� �ð�
	_float m_fMoveUpTime = 0.1f;      // �ö󰡴� �� �ɸ��� �ð�
	_float m_fCurrentTime = 0.0f;     // ���� �ִϸ��̼� ���� �ð�

	_bool m_bIsMoving = false;        // �ִϸ��̼� ���� ����
	_bool m_bIsMovingDown = false;    // �������� ������ ����

public:
	static CQTE_Continuous_Attack_Space* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
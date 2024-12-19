#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CQTE_Continuous_Attack_Gauge final : public CGameObject
{
public:
	struct CONTINUOUS_ATTACK_GAUGE_DESC
	{
		_float	fSizeX{}, fSizeY{}, fX{}, fY{};
		class CQTE_Continuous_Attack* pContinuous_Attack = { nullptr };
	};

private:
	CQTE_Continuous_Attack_Gauge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE_Continuous_Attack_Gauge(const CQTE_Continuous_Attack_Gauge& Prototype);
	virtual ~CQTE_Continuous_Attack_Gauge() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

	void	Process_Command();
	_float* const Get_GaugeProcess() { return &m_fGaugeProgress; };
public:
	_float					m_fSizeX{}, m_fSizeY{}, m_fX{}, m_fY{};
	_float4x4				m_ViewMatrix{}, m_ProjMatrix{};

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	CShader* m_pShaderCom = { nullptr };

	//�������ٿ� ������
	CTexture* m_pTextureCom[2] = { nullptr,nullptr };
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

	_float m_fGaugeProgress = 0.5f; // �������� 50%���� ����
	_float m_fGaugeDecreaseRate = 0.2f; // ������ ���� �ӵ� (�ʴ� 10%)
	_float m_fGaugeIncreaseAmount = 0.06f; // Ű �Է� �� ������ ������ (10%)

	class CQTE_Continuous_Attack* m_pContinuous_Attack = { nullptr };

public:
	static CQTE_Continuous_Attack_Gauge* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
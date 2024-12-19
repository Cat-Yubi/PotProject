#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END


BEGIN(Client)

class CQTE_Continuous_Attack final : public CGameObject
{
public:
	enum MISSION_STATE
	{
		MISSION_NOT_DECIDED,
		MISSION_FAILED,
		MISSION_SUCCESS
	};
private:
	CQTE_Continuous_Attack(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE_Continuous_Attack(const CQTE_Continuous_Attack& Prototype);
	virtual ~CQTE_Continuous_Attack() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

	void Notify_Result(MISSION_STATE isResult) { m_eMissionState = isResult; };
	void Start(class CGameObject* callObject);
private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	void Start_QTE(class CGameObject* callObject);
	void End_QTE();
	void End_Offset_QTE(_float fTimeDelta);
	void Handle_QTEInput();
	void Process_Command();

	void Update_Animation(_float fTimeDelta);

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	_float					m_fSizeX{}, m_fSizeY{}, m_fX{}, m_fY{}, m_fAlpha{};
	_float4x4				m_ViewMatrix{}, m_ProjMatrix{};
	_int m_iTextureNumber = {};

	_bool m_bIsQTEActive = { false }; // QTE Ȱ��ȭ ����
	_int m_iCharacterSide = { 1 };

	_bool m_bIsSuccess = { false };

	MISSION_STATE m_eMissionState = MISSION_NOT_DECIDED;

	// ��ġ ���� ����
	_float m_fDefaultY = {  };  // �⺻ ��ġ
	_float m_fTargetY = {  };    // Ÿ�� ��ġ

	// �ִϸ��̼� ���� ����
	_float m_fMoveDownTime = 0.1f;    // �������� �� �ɸ��� �ð�
	_float m_fMoveUpTime = 0.1f;      // �ö󰡴� �� �ɸ��� �ð�
	_float m_fCurrentTime = 0.0f;     // ���� �ִϸ��̼� ���� �ð�

	_bool m_bIsMoving = false;        // �ִϸ��̼� ���� ����
	_bool m_bIsMovingDown = false;    // �������� ������ ����

	class CQTE_Continuous_Attack_Space* m_pContinuous_Space = { nullptr };
	class CQTE_Continuous_Attack_Gauge* m_pContinuous_Gauge = { nullptr };
	class CQTE_Continuous_Attack_Effect* m_pContinuous_Effect = { nullptr };
	vector<class CQTE_Continuous_Attack_Particle*> m_UIParticles;
	vector<class CQTE_Continuous_Attack_Space_Particle*> m_UISpaceParticles;

	//ī�޶�
	class CMain_Camera* m_pMain_Camera = { nullptr };
	//ȣ���� ��ü
	class CGameObject* m_pCall_Object = { nullptr };

	//��ü���� ������� ���� �ð��� ��� ��
	_float m_fOffsetTimer = { 2.f };
	_bool m_bOffsetActive = { false };
public:
	static CQTE_Continuous_Attack* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
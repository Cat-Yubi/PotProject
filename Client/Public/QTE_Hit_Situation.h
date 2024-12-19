#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "QTE_Hit.h"
#include "QTE_Hit_UI_Icon.h"
BEGIN(Client)

class CQTE_Hit_Situation final : public CGameObject
{
public:
	struct QTE_HIT_SITUATION_DESC
	{
		_float lifeTime = {};
		_int create_Num = {};
		CQTE_Hit::Hit_Situation_ID ID = {};
	};

private:
	CQTE_Hit_Situation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE_Hit_Situation(const CQTE_Hit_Situation& Prototype);
	virtual ~CQTE_Hit_Situation() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

	void Notify_Last_UI_Final_Complete() { m_bUI_Final_Complete = true; };
	void Notify_Faild_Result(class CQTE_Hit_UI_Icon* icon);
	void Start(class CGameObject* pCall_Object);
private:
	void Start_QTE(class CGameObject* pCall_Object);
	void End_QTE();
	void End_Offset_QTE(_float fTimeDelta);
	void Handle_QTEInput();
	void Create_UIIcon(); // ������ ���� �Լ� �߰�
	void Process_Command(CQTE_Hit_UI_Icon::KEY_ID input);

	void Create_ResultObject(class CQTE_Hit_UI_Icon* pIcon);
	void Create_ParticleObject(class CQTE_Hit_UI_Icon* pIcon);

private:
	_float m_fLifeTime = {};
	_int m_iCreate_Num = {};

	_bool m_bIsQTEActive = { false }; // QTE Ȱ��ȭ ����
	_float m_fTimer = { 0.f };

	//��ü���� ������� ���� �ð��� ��� ��
	_float m_fOffsetTimer = { 2.f };
	_bool m_bOffsetActive = { false };

	_float m_fElapsedTime = { 0.f }; // ��� �ð�
	_int m_iNextIconIndex = { 0 }; // ���� ������ �ε���

	_int m_iCharacterSide = { 1 };

	vector<_float> m_vecIconCreationTimes;
	vector<class CQTE_Hit_UI_Icon*> m_vecHitUIIcon;
	vector<class CQTE_Hit_UI_Result*> m_vecHitResult;
	vector<class CQTE_Hit_UI_Particle*> m_vecHitParticle;


	CQTE_Hit::Hit_Situation_ID m_currentSituationID = {};

	//������ UI_Final
	_bool m_bUI_Final_Complete = { false };
	//ī�޶�
	class CMain_Camera* m_pMain_Camera = { nullptr };
	//ȣ���� ������Ʈ
	class CGameObject* m_pCall_Object = { nullptr };

public:
	static CQTE_Hit_Situation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
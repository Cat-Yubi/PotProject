#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Client)

class CQTE_Same_Grab final : public CGameObject
{
	enum UI_COMMAND
	{
		UI_COMMAND_LIGHT, //���
		UI_COMMAND_MIDDLE, //�߰�
		UI_COMMAND_HEAVY, //����
		UI_COMMAND_ULTIMATE, //Ư������
		UI_COMMAND_END
	};

private:
	CQTE_Same_Grab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE_Same_Grab(const CQTE_Same_Grab& Prototype);
	virtual ~CQTE_Same_Grab() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

	void Notify_Last_UI_Final_Complete() { m_bUI_Final_Complate = true; };
	void Start(class CGameObject* callObject, _float _lifeTime);
private:
	void Start_QTE(class CGameObject* callObject, _float _lifeTime);
	void End_QTE();
	void Handle_QTEInput();
	void Process_Command(UI_COMMAND input, _int playerID);
	void Create_UIIcons(_int playerID, const vector<UI_COMMAND>& sequence);
	void Clear_UIIcons();
	void Handle_WrongInput(_int playerID); // ���� ó�� �Լ� ����
	void Ascend_UIIcons(vector<class CQTE_Same_Grab_UI_Icon*>& icons);

	void Final_End_QTE();
	_bool Check_GameStart(class CGameObject* callObject);
	_int Determine_QTE_Result() const;
private:
	_bool m_bIsQTEActive = { false }; // QTE Ȱ��ȭ ����
	_float m_fTimer = { 0.f }; // Ÿ�̸�
	_float m_iTotalTime = { 6.f }; // �� �ð� (��: 10��)
	_int m_iSequenceLength = { 10 }; // ������ ���� (N)

	// 1P ����
	queue<UI_COMMAND> m_CommandQueue_P1;
	vector<UI_COMMAND> m_CurrentSequence_P1;
	_int m_iCorrectInputs_P1 = {};
	_int m_CurrentIndex_P1 = { 0 }; // ���� ���õ� ������ �ε���

	// 2P ����
	queue<UI_COMMAND> m_CommandQueue_P2;
	vector<UI_COMMAND> m_CurrentSequence_P2;
	_int m_iCorrectInputs_P2 = {};
	_int m_CurrentIndex_P2 = { 0 }; // ���� ���õ� ������ �ε���

	// UI ������ ��ü ����
	vector<class CQTE_Same_Grab_UI_Icon*> m_UIIcons_P1;
	vector<class CQTE_Same_Grab_UI_Icon*> m_UIIcons_P2;
	vector<class CQTE_Same_Grab_UI_Particle*> m_UIParticles;
	class CQTE_Same_Grab_UI_Gauge* m_UIGauge = { nullptr };

	// �� �÷��̾��� ��ٿ� Ÿ�̸�
	_float m_fCooldown_P1 = {};
	_float m_fCooldown_P2 = {};

	// ��ٿ� ���� �ð� (��)
	const _float COOLDOWN_DURATION = 1.0f; // �ʿ信 ���� ���� ����

	// QTE ���� �� �ʱ�ȭ ����
	_bool m_bIsEndQTE = { false };           // QTE ���� ���μ��� ���� ������ ����
	_float m_fEndQTE_Timer = { 0.f };      // QTE ���� �� �ʱ�ȭ������ ��� �ð�
	const _float m_fEndQTE_Delay = { 1.5f }; // �ʱ�ȭ���� ��ٸ� �ð� (��)


	//������ UI_Final
	_bool m_bUI_Final_Complate = { false };
	//ī�޶�
	class CMain_Camera* m_pMain_Camera = { nullptr };
	//�θ� ��� ȣ���ؾ���
	list<class CGameObject*> m_pListGameObject;
	class CCharacter* m_p1PCharacter = { nullptr };
	class CCharacter* m_p2PCharacter = { nullptr };

public:
	static CQTE_Same_Grab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
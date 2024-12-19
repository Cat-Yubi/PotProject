#include "stdafx.h"
#include "..\Public\QTE_1P_Same_Grab.h"

#include "RenderInstance.h"
#include "GameInstance.h"
#include "QTE_Same_Grab_UI_Icon.h"
#include "QTE_Same_Grab_UI_Gauge.h"
#include "Main_Camera.h"
#include "QTE_Same_Grab_UI_Particle.h"

CQTE_1P_Same_Grab::CQTE_1P_Same_Grab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CQTE_1P_Same_Grab::CQTE_1P_Same_Grab(const CQTE_1P_Same_Grab& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CQTE_1P_Same_Grab::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CQTE_1P_Same_Grab::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	srand(static_cast<_uint>(time(nullptr))); // ���� �õ� �ʱ�ȭ

	return S_OK;
}

void CQTE_1P_Same_Grab::Camera_Update(_float fTimeDelta)
{

}

void CQTE_1P_Same_Grab::Update(_float fTimeDelta)
{
#pragma region �����
	//// F5 Ű �Է� ����
	//if (m_pGameInstance->Key_Down(DIK_F6))
	//{
	//	if (m_bIsQTEActive)
	//	{
	//		// QTE�� Ȱ��ȭ�Ǿ� ������ ��� ����
	//		End_QTE();
	//	}
	//	else
	//	{
	//		// QTE�� ��Ȱ��ȭ�Ǿ� ������ ����
	//		Start_QTE();
	//	}
	//}
#pragma endregion

#pragma region QTE ���� ���μ��� ���� ���� ���

	// QTE ���� ���μ��� ���� ���� ���
	if (m_bIsEndQTE)
	{
		m_fEndQTE_Timer += fTimeDelta;
		if (m_fEndQTE_Timer >= m_fEndQTE_Delay)
		{
			Final_End_QTE();
		}

		// Ascend �ִϸ��̼� ������Ʈ
		for (auto& iter : m_UIIcons)
			iter->Update(fTimeDelta);
		for (auto& iter : m_UIParticles)
			iter->Update(fTimeDelta);
	}
#pragma endregion

#pragma region QTE Ȱ��ȭ

	else if (m_bIsQTEActive)
	{
		// UI ������ ������Ʈ
		for (auto& iter : m_UIIcons)
			iter->Update(fTimeDelta);
		for (auto& iter : m_UIParticles)
			iter->Update(fTimeDelta);

		//������ UI�� �������� �׶����� ������
		if (m_bUI_Final_Complate)
		{
			if (m_UIGauge != nullptr)
				m_UIGauge->Update(fTimeDelta);

			// QTE�� Ȱ��ȭ�� ��� ���� ���� ����
			// Ÿ�̸� ����
			m_fTimer -= fTimeDelta;

			// �ð� ���� Ȯ��
			if (m_fTimer <= 0.0f || m_iCorrectInputs == m_iSequenceLength)
				End_QTE();

			// ����� �Է� ó��
			Handle_QTEInput();

			// ��ٿ� Ÿ�̸� ����
			if (m_fCooldown > 0.0f)
				m_fCooldown -= fTimeDelta;
		}
	}
#pragma endregion
}

void CQTE_1P_Same_Grab::Late_Update(_float fTimeDelta)
{
	if (m_bIsQTEActive)
	{
		for (auto& iter : m_UIIcons)
			iter->Late_Update(fTimeDelta);
		for (auto& iter : m_UIParticles)
			iter->Late_Update(fTimeDelta);

		if (m_UIGauge != nullptr && m_bUI_Final_Complate)
			m_UIGauge->Late_Update(fTimeDelta);
	}
}

void CQTE_1P_Same_Grab::Start(CGameObject* callObject, _float _lifeTime)
{
	if (m_bIsQTEActive)
	{
		// QTE�� Ȱ��ȭ�Ǿ� ������ ��� ����
		End_QTE();
	}
	else
	{
		Start_QTE(callObject, _lifeTime);
	}
}

void CQTE_1P_Same_Grab::Start_QTE(CGameObject* callObject, _float _lifeTime)
{
	if (m_bIsQTEActive)
		return; // �̹� QTE�� Ȱ��ȭ�Ǿ� ������ ����

	//ī�޶� ����ŷ������ �ʿ���
	m_pMain_Camera = static_cast<CMain_Camera*>(m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Main_Camera")));
	//ĳ���� ��� ���� ����
	m_pCharacter = static_cast<CCharacter*>(callObject);
	m_iCharacterTeam = m_pCharacter->Get_iPlayerTeam();

	//���� ��ü�� ���� ����
	Clear_UIIcons();

	m_bIsQTEActive = true;

	//lifeTime�� ���� �־����� �ʾ����� ���� X
	if (_lifeTime != -1.f)
		m_iTotalTime = _lifeTime;

	m_fTimer = static_cast<_float>(m_iTotalTime);

	// 1P �ʱ�ȭ
	// Queue �� �̾Ƴ���
	while (!m_CommandQueue.empty()) m_CommandQueue.pop();
	// vector �� �̾Ƴ���
	m_CurrentSequence.clear();
	// 1P�� �󸶳� ������� Input �ʱ�ȭ
	m_iCorrectInputs = 0;

	vector<UI_COMMAND> sequence;
	for (_int i = 0; i < m_iSequenceLength; ++i)
	{
		UI_COMMAND cmd = static_cast<UI_COMMAND>(rand() % UI_COMMAND_END);
		m_CommandQueue.push(cmd);
		m_CurrentSequence.push_back(cmd);
		sequence.push_back(cmd);
	}

	// UI ������ ����
	Create_UIIcons(sequence);
}

void CQTE_1P_Same_Grab::End_QTE()
{
	// Ascend �ִϸ��̼� ����
	Ascend_UIIcons(m_UIIcons);

	// QTE ���� ���μ��� ���� ǥ��
	m_bIsEndQTE = true;
	m_fEndQTE_Timer = 0.0f;

	//��� ����
	_bool isTimerEnded = (m_fTimer <= 0.0f);
	_bool isCompleted = (m_iCorrectInputs == m_iSequenceLength);

	if (isTimerEnded)
		//�ð� ������ �й�
		m_pCharacter->Notify_QTE_1p_Grab(-1);
	else if (isCompleted)
		//����
		m_pCharacter->Notify_QTE_1p_Grab(1);

	m_pCharacter = nullptr;
	m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::QTE_SUCCESS_SFX, false, 0.7f);
}

void CQTE_1P_Same_Grab::Handle_QTEInput()
{
	if (m_fCooldown <= 0.0f)
	{
		if (m_iCharacterTeam == 1)
		{
			if (m_pGameInstance->Key_Down(DIK_U))
			{
				Process_Command(UI_COMMAND_LIGHT);
			}
			else if (m_pGameInstance->Key_Down(DIK_I))
			{
				Process_Command(UI_COMMAND_MIDDLE);
			}
			else if (m_pGameInstance->Key_Down(DIK_J))
			{
				Process_Command(UI_COMMAND_ULTIMATE);
			}
			else if (m_pGameInstance->Key_Down(DIK_K))
			{
				Process_Command(UI_COMMAND_HEAVY);
			}
		}
		else if (m_iCharacterTeam == 2)
		{
			if (m_pGameInstance->Key_Down(DIK_NUMPAD7))
			{
				Process_Command(UI_COMMAND_LIGHT);
			}
			else if (m_pGameInstance->Key_Down(DIK_NUMPAD8))
			{
				Process_Command(UI_COMMAND_MIDDLE);
			}
			else if (m_pGameInstance->Key_Down(DIK_NUMPAD4))
			{
				Process_Command(UI_COMMAND_ULTIMATE);
			}
			else if (m_pGameInstance->Key_Down(DIK_NUMPAD5))
			{
				Process_Command(UI_COMMAND_HEAVY);
			}
		}
	}
}

void CQTE_1P_Same_Grab::Process_Command(UI_COMMAND input)
{
	if (m_CommandQueue.empty())
		return; // ��� ����� �̹� ó���� ���

	UI_COMMAND expected = m_CommandQueue.front();
	//��Ȯ�� Input�� ������ ��
	if (input == expected)
	{
		m_CommandQueue.pop();
		m_iCorrectInputs++;

		// ���� ���õ� �������� ���� ���� ����
		if (m_CurrentIndex < m_UIIcons.size())
		{
			CQTE_Same_Grab_UI_Particle::QTE_Same_Grab_UI_Particle_DESC Desc{};
			_float offsetY = -50.f;
			Desc.fX = m_UIIcons[m_CurrentIndex]->m_fX;
			Desc.fY = m_UIIcons[m_CurrentIndex]->m_fY + offsetY;
			Desc.fSizeX = 50.f;
			Desc.fSizeY = 50.f;

			CQTE_Same_Grab_UI_Particle* particle = static_cast<CQTE_Same_Grab_UI_Particle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_QTE_Same_Grab_UI_Particle"), &Desc));
			m_UIParticles.push_back(particle);

			m_UIIcons[m_CurrentIndex]->Set_State(CQTE_Same_Grab_UI_Icon::ALREADY_PRESSED);
			m_CurrentIndex++;
		}

		// ���� �������� ���� ���·� ����
		if (m_CurrentIndex < m_UIIcons.size())
		{
			m_UIIcons[m_CurrentIndex]->Set_State(CQTE_Same_Grab_UI_Icon::SELECTED);
		}

		// ��� ����� �Ϸ��� ��� QTE ���� (���� ����)
		if (m_CommandQueue.empty())
		{
			//EndQTE();
		}

		m_pMain_Camera->StartCameraShake(0.1, 0.1);
		m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::QTE_SAMEGRAB_SUCCESS_SFX, false, 0.7f);

	}
	else
	{
		// �߸��� �Է� �� QTE ���� ó��
		Handle_WrongInput();
		m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::QTE_SAMEGRAB_FAIL_SFX, false, 0.7f);
		return;
	}
}

void CQTE_1P_Same_Grab::Create_UIIcons(const vector<UI_COMMAND>& sequence)
{
#pragma region ���� UI ��ü ����
	// �� �÷��̾��� UI ������ ���Ϳ� �߰�

	// �÷��̾� ID�� 1P�� ���⿡ ���� centerX ����
	_float centerX = 960.f;

	// y ��ġ ����
	_float centerY = 300.f;

	// ������ ���� ���� (�ʿ信 ���� ����)
	_float spacing = 50.f;

	// ������ ��
	_int numIcons = static_cast<_int>(sequence.size());

	// �������� x ��ġ�� ������ ����
	vector<_float> iconPositionsX;

	// Ȧ�� ���� ������ ��ġ
	if (numIcons % 2 == 1)
	{
		_int mid = numIcons / 2;
		for (_int i = 0; i < numIcons; ++i)
		{
			_float offset = spacing * (i - mid);
			iconPositionsX.push_back(centerX + offset);
		}
	}
	// ¦�� ���� ������ ��ġ
	else
	{
		_int mid = numIcons / 2;
		for (_int i = 0; i < numIcons; ++i)
		{
			// ¦���� ��� �߾� �� �������� �������� �¿� ��ġ
			_float offset = spacing * (i - mid + 0.5f);
			iconPositionsX.push_back(centerX + offset);
		}
	}

	// �������� �����ϴ� ���� �ð� ���� ���� (��)
	const _float FALL_DELAY_INTERVAL = 0.1f; // �ʿ信 ���� ���� ����

	// �� �������� �����ϰ� ��ġ ����
	for (_int i = 0; i < numIcons; ++i)
	{
		CQTE_Same_Grab_UI_Icon::QTE_Same_Grab_UI_ICON_DESC Desc{};
		Desc.iTextureNumber = static_cast<_int>(sequence[i]);
		Desc.fSizeX = 50.f; // �ʿ信 ���� ũ�� ����
		Desc.fSizeY = 50.f;
		Desc.fX = iconPositionsX[i];
		Desc.fY = centerY;
		Desc.fAlpha = 1.f;
		Desc.fFallDelay = i * FALL_DELAY_INTERVAL; // �� �������� �������� �����ϴ� ���� �ð� ����
		Desc.SameGrab = this;

		//ù��° �༮�� �������� ��ġ�� �޶����
		if (i == 0)
			Desc.isFirst = true;

		//������ �༮�� �������� SameGrab���� ��μ� ���۵Ǿ��ٰ� �˷�����
		if (i == numIcons - 1)
			Desc.isLast = true;

		// UI ������ Ŭ��
		CQTE_Same_Grab_UI_Icon* pIcon = dynamic_cast<CQTE_Same_Grab_UI_Icon*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_QTE_Same_Grab_UI_Icon"), &Desc));
		if (pIcon)
		{
			// ���Ϳ� ����
			m_UIIcons.push_back(pIcon);
		}
	}
#pragma endregion

	// ������ ������ ��ü ����
	if (m_UIGauge != nullptr)
	{
		Safe_Release(m_UIGauge);
		m_UIGauge = nullptr;
	}

#pragma region ������ ��ü ����
	CQTE_Same_Grab_UI_Gauge::QTE_UI_Gauge_DESC Desc{};
	Desc.fSizeX = 400.f;
	Desc.fSizeY = 20.f;
	Desc.fX = 960.f;
	Desc.fY = 810.f;
	Desc.playTime = m_iTotalTime;
	m_UIGauge = dynamic_cast<CQTE_Same_Grab_UI_Gauge*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_QTE_Same_Grab_UI_Gauge"), &Desc));
#pragma endregion
}

void CQTE_1P_Same_Grab::Ascend_UIIcons(vector<CQTE_Same_Grab_UI_Icon*>& icons)
{
	// �������� �������� ó���Ͽ� ������ �����ܺ��� ASCEND ���·� ��ȯ
	const float ASCEND_DELAY_INTERVAL = 0.1f; // �� ������ ���� ��� ���� ���� �ð� (��)
	for (int i = static_cast<int>(icons.size()) - 1; i >= 0; --i)
	{
		CQTE_Same_Grab_UI_Icon* pIcon = icons[i];
		if (pIcon)
		{
			// ASCEND ���·� ��ȯ�ϸ鼭 ���� �ð��� ����
			float ascendDelay = (icons.size() - 1 - i) * ASCEND_DELAY_INTERVAL;
			pIcon->Set_State(CQTE_Same_Grab_UI_Icon::ASCEND);
			pIcon->Set_AscendDelay(ascendDelay);
		}
	}
}

void CQTE_1P_Same_Grab::Final_End_QTE()
{
	// �ʱ�ȭ �۾� ����
	m_bIsEndQTE = false;
	m_bIsQTEActive = false;
	m_fTimer = 0.0f;
	m_bUI_Final_Complate = false;

	// ť�� ������ �ʱ�ȭ
	while (!m_CommandQueue.empty()) m_CommandQueue.pop();
	m_CurrentSequence.clear();
	m_iCorrectInputs = 0;
	m_CurrentIndex = 0;

	// UI ������ ����
	Clear_UIIcons();
}



void CQTE_1P_Same_Grab::Clear_UIIcons()
{
	// 1P UI ������ ����
	for (auto& icon : m_UIIcons)
	{
		if (icon)
		{
			Safe_Release(icon);
		}
	}
	m_UIIcons.clear();

	for (auto& icon : m_UIParticles)
	{
		if (icon)
		{
			Safe_Release(icon);
		}
	}
	m_UIParticles.clear();

	//Gauge ����
	Safe_Release(m_UIGauge);
}

void CQTE_1P_Same_Grab::Handle_WrongInput()
{
	if (m_CurrentIndex < m_UIIcons.size())
	{
		m_UIIcons[m_CurrentIndex]->Set_State(CQTE_Same_Grab_UI_Icon::WRONG_PRESSED);
	}

	m_fCooldown = COOLDOWN_DURATION;
}


HRESULT CQTE_1P_Same_Grab::Render(_float fTimeDelta)
{
	return S_OK;
}

CQTE_1P_Same_Grab* CQTE_1P_Same_Grab::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CQTE_1P_Same_Grab* pInstance = new CQTE_1P_Same_Grab(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CQTE_1P_Same_Grab"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CQTE_1P_Same_Grab::Clone(void* pArg)
{
	CQTE_1P_Same_Grab* pInstance = new CQTE_1P_Same_Grab(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CQTE_1P_Same_Grab"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQTE_1P_Same_Grab::Free()
{
	Clear_UIIcons();

	__super::Free();
}

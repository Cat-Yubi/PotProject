#include "stdafx.h"
#include "..\Public\QTE_Hit_Situation.h"

#include "RenderInstance.h"
#include "GameInstance.h"
#include "QTE_Hit_UI_Icon.h"
#include "QTE_Hit_UI_Result.h"
#include "QTE_Hit_UI_Particle.h"
#include "Main_Camera.h"
#include "Sound_Manager.h"
CQTE_Hit_Situation::CQTE_Hit_Situation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CQTE_Hit_Situation::CQTE_Hit_Situation(const CQTE_Hit_Situation& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CQTE_Hit_Situation::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CQTE_Hit_Situation::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	//�����õ� �߻�
	srand(static_cast<unsigned>(std::time(0)));

	QTE_HIT_SITUATION_DESC* Desc = static_cast<QTE_HIT_SITUATION_DESC*>(pArg);
	//�ش� ��Ȳ�� ��ü ������Ÿ��
	m_fLifeTime = Desc->lifeTime;
	m_fTimer = m_fLifeTime;

	//�ش� ��Ȳ�� ���� ����
	m_iCreate_Num = Desc->create_Num;
	//�ش� ��Ȳ ID
	m_currentSituationID = Desc->ID;

	//�ʱ�ȭ
	m_fElapsedTime = 0.0f;
	m_iNextIconIndex = 0;

	switch (m_currentSituationID)
	{
	case Client::CQTE_Hit::Hit_Situation_ID_Goku:
		//m_iCreate_Num - 1���� ���;���
		m_vecIconCreationTimes = { 1.f, 1.f };
		break;
	case Client::CQTE_Hit::Hit_Situation_ID_21:
		//m_iCreate_Num - 1���� ���;���
		m_vecIconCreationTimes = { 1.f, 1.f };
		break;
	case Client::CQTE_Hit::Hit_Situation_ID_Frieza:
		//m_iCreate_Num - 1���� ���;���
		m_vecIconCreationTimes = { 1.f, 1.f };
		break;
	case Client::CQTE_Hit::Hit_Situation_ID_Hit:
		//m_iCreate_Num - 1���� ���;���
		m_vecIconCreationTimes = { 1.f, 1.f };
		break;
	}

	return S_OK;
}

void CQTE_Hit_Situation::Camera_Update(_float fTimeDelta)
{

}

void CQTE_Hit_Situation::Update(_float fTimeDelta)
{
#pragma region �����
	//// F5 Ű �Է� ����
	//if (m_pGameInstance->Key_Down(DIK_F5))
	//{
	//	if (m_bIsQTEActive)
	//	{
	//		// QTE�� Ȱ��ȭ�Ǿ� ������ ��� ����
	//		End_QTE();

	//		// UI ��ü ���� �ð�
	//		for (auto& iter : m_vecHitUIIcon)
	//			Safe_Release(iter);

	//		for (auto& iter : m_vecHitResult)
	//			Safe_Release(iter);

	//		for (auto& iter : m_vecHitParticle)
	//			Safe_Release(iter);

	//		m_vecHitUIIcon.clear();
	//		m_vecHitResult.clear();
	//		m_vecHitParticle.clear();

	//		m_fOffsetTimer = 0.f;
	//		m_bOffsetActive = false; // ������ �Ⱓ ����

	//	}
	//	else
	//	{
	//		// QTE�� ��Ȱ��ȭ�Ǿ� ������ ����
	//		// ù��° �������� �ﰢ ��������
	//		Start_QTE();
	//	}
	//}
#pragma endregion

#pragma region Ȱ��ȭ
	if (m_bIsQTEActive)
	{
		//��ü Ÿ�̸Ӱ� ���� �ð� �ҿ�ǰų�
		//������ UI ��ü ó���� �Ϸ��ų�
		if (m_fTimer <= 0.0f || m_bUI_Final_Complete)
		{
			// QTE ����
			End_QTE();
		}

		// ����� �Է� ó��
		Handle_QTEInput();

		// �� Hit_UI_Icon ������Ʈ
		for (auto& iter : m_vecHitUIIcon)
			iter->Update(fTimeDelta);

		for (auto& iter : m_vecHitResult)
			iter->Update(fTimeDelta);

		for (auto& iter : m_vecHitParticle)
			iter->Update(fTimeDelta);

		// ���� ������ �����Ұ� �����ִ��� üũ
		if (m_iNextIconIndex < m_vecIconCreationTimes.size())
		{
			//����ð��� ������ �ð����� �þ�ٸ� ������ ����
			if (m_fElapsedTime >= m_vecIconCreationTimes[m_iNextIconIndex])
			{
				// ������ ����
				Create_UIIcon();
				m_iNextIconIndex++;
				m_fElapsedTime = 0.f;
			}
		}

		// ��� �ð� ������Ʈ
		m_fElapsedTime += fTimeDelta;

		// Ÿ�̸� ������Ʈ
		m_fTimer -= fTimeDelta;
	}
#pragma endregion

#pragma region Offset �Ⱓ Ȱ��ȭ

	//offSet �Ⱓ ó��
	else if (m_bOffsetActive)
	{
		End_Offset_QTE(fTimeDelta);
	}
#pragma endregion
}

void CQTE_Hit_Situation::Late_Update(_float fTimeDelta)
{
	//QTE�� Ȱ��ȭ�Ǿ��ٸ�
	if (m_bIsQTEActive || m_bOffsetActive)
	{
		for (auto& iter : m_vecHitUIIcon)
			iter->Late_Update(fTimeDelta);

		for (auto& iter : m_vecHitResult)
			iter->Late_Update(fTimeDelta);

		for (auto& iter : m_vecHitParticle)
			iter->Late_Update(fTimeDelta);
	}
}

HRESULT CQTE_Hit_Situation::Render(_float fTimeDelta)
{
	return S_OK;
}

void CQTE_Hit_Situation::Notify_Faild_Result(CQTE_Hit_UI_Icon* icon)
{
	Create_ResultObject(icon);
}

void CQTE_Hit_Situation::Start(CGameObject* pCall_Object)
{
	if (m_bIsQTEActive)
	{
		// QTE�� Ȱ��ȭ�Ǿ� ������ ��� ����
		End_QTE();

		// UI ��ü ���� �ð�
		for (auto& iter : m_vecHitUIIcon)
			Safe_Release(iter);

		for (auto& iter : m_vecHitResult)
			Safe_Release(iter);

		for (auto& iter : m_vecHitParticle)
			Safe_Release(iter);

		m_vecHitUIIcon.clear();
		m_vecHitResult.clear();
		m_vecHitParticle.clear();

		m_fOffsetTimer = 0.f;
		m_bOffsetActive = false; // ������ �Ⱓ ����

	}
	else
	{
		// QTE�� ��Ȱ��ȭ�Ǿ� ������ ����
		// ù��° �������� �ﰢ ��������
		Start_QTE(pCall_Object);
	}
}

void CQTE_Hit_Situation::Start_QTE(CGameObject* pCall_Object)
{
	if (m_bIsQTEActive)
		return; // �̹� QTE�� Ȱ��ȭ�Ǿ� ������ ����

	//����׿�
	if (pCall_Object == nullptr)
	{
		//1p�� �ޱ�
		m_pCall_Object = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"));
	}
	else
		m_pCall_Object = pCall_Object;

	m_iCharacterSide = static_cast<CCharacter*>(m_pCall_Object)->Get_iPlayerTeam();

	//ī�޶� ����ŷ������ �ʿ���
	m_pMain_Camera = static_cast<CMain_Camera*>(m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Main_Camera")));

	//Ȱ��ȭ
	m_bIsQTEActive = true;
	//������ ��ü �Ϸ� ó�� ���� �ʱ�ȭ
	m_bUI_Final_Complete = false;
	// ù ��° ������ ��� ����
	Create_UIIcon();
}

void CQTE_Hit_Situation::End_QTE()
{
#pragma region ���� ����
	_bool isSuccess = true;

	for (auto& iter : m_vecHitUIIcon)
	{
		//FAIL�̰ų� �������� �ʾҰų�(����ó��) �ϳ��� ������ ���� ����
		if (iter->m_currentResult_ID == CQTE_Hit_UI_Icon::RESULT_ID::HIT_RESULT_FAILED ||
			iter->m_currentResult_ID == CQTE_Hit_UI_Icon::RESULT_ID::HIT_RESULT_NOT_YET_DECIDED)
		{
			isSuccess = false;
			break;
		}
	}

	// isSuccess�� int�� ��ȯ (true -> 1, false -> -1)
	_int hitResult = isSuccess ? 1 : -1;
	// ����� ����
	static_cast<CCharacter*>(m_pCall_Object)->Notify_QTE_Hit(hitResult);

#pragma endregion

#pragma region �ʱ�ȭ
	//Ȱ��ȭ ���� �ʱ�ȭ
	m_bIsQTEActive = false;

	// ������ Ÿ�̸� ����
	m_bOffsetActive = true;
	m_fOffsetTimer = 2.f; // ���ϴ� ������ ���� �ð� ����

	//����ð� �ʱ�ȭ
	m_fElapsedTime = 0.0f;
	//���� ��ü�� ����µ� �ʿ��� ���� �ʱ�ȭ
	m_iNextIconIndex = 0;
	//Ÿ�̸Ӵ� ���� LifeTime���� �ʱ�ȭ
	m_fTimer = m_fLifeTime;
	//������ ��ü �Ϸ� ó�� ���� �ʱ�ȭ
	m_bUI_Final_Complete = false;
	m_pCall_Object = nullptr;
#pragma endregion

}

void CQTE_Hit_Situation::End_Offset_QTE(_float fTimeDelta)
{
	// ������ �Ⱓ ó��
	m_fOffsetTimer -= fTimeDelta;

	// ���̵� �ƿ� ȿ���� ���� UI ��ü ������Ʈ
	for (auto& iter : m_vecHitUIIcon)
		iter->Update(fTimeDelta);

	for (auto& iter : m_vecHitResult)
		iter->Update(fTimeDelta);

	for (auto& iter : m_vecHitParticle)
		iter->Update(fTimeDelta);

	if (m_fOffsetTimer <= 0.0f)
	{
		// UI ��ü ���� �ð�
		for (auto& iter : m_vecHitUIIcon)
			Safe_Release(iter);

		for (auto& iter : m_vecHitResult)
			Safe_Release(iter);

		for (auto& iter : m_vecHitParticle)
			Safe_Release(iter);

		m_vecHitUIIcon.clear();
		m_vecHitResult.clear();
		m_vecHitParticle.clear();

		m_fOffsetTimer = 0.f;
		m_bOffsetActive = false; // ������ �Ⱓ ����
	}
}

void CQTE_Hit_Situation::Create_UIIcon()
{
	CQTE_Hit_UI_Icon::QTE_Hit_UI_ICON_DESC Desc{};
	Desc.fSizeX = { 100.f };
	Desc.fSizeY = { 100.f };

	// ��ġ ���� ����
	// �� ���� ������ Ȯ�������� �� ��
	_float minX = 480.f;
	_float maxX = 1440.f; // ȭ�� �ʺ� - ������ �ʺ� ���
	_float minY = 300.f;
	_float maxY = 780.f; // ȭ�� ���� - ������ ���̸� ���

#pragma region ��ġ�� �ʴ� ��ġ�� ã�� ���� �ִ� �õ� Ƚ��

	const _int maxAttempts = 100;
	_int attempts = 0;
	_bool positionFound = false;

	while (!positionFound && attempts < maxAttempts)
	{
		// ���� ��ġ ����
		Desc.fX = minX + static_cast<_float>(rand()) / RAND_MAX * (maxX - minX);
		Desc.fY = minY + static_cast<_float>(rand()) / RAND_MAX * (maxY - minY);

		// �������� ��ġ���� �˻�
		positionFound = true;
		for (auto& existingIcon : m_vecHitUIIcon)
		{
			// ���� �������� ��ġ�� ũ�� ��������
			_float existingX = existingIcon->m_fX;
			_float existingY = existingIcon->m_fY;
			_float existingSizeX = existingIcon->m_fSizeX * 3.f;
			_float existingSizeY = existingIcon->m_fSizeY * 3.f;

			// AABB �浹 �˻�
			if (Desc.fX < existingX + existingSizeX &&
				Desc.fX + Desc.fSizeX * 3.f > existingX &&
				Desc.fY < existingY + existingSizeY &&
				Desc.fY + Desc.fSizeY * 3.f > existingY)
			{
				// ��ħ �߻�
				positionFound = false;
				break;
			}
		}
		attempts++;
	}

	if (!positionFound)
	{
		// ��ġ�� ã�� ���� ��� �⺻ ��ġ ���� �Ǵ� ó��
		Desc.fX = 960.f - Desc.fSizeX / 2; // ȭ�� �߾� ��
		Desc.fY = 540.f - Desc.fSizeY / 2;
	}

#pragma endregion


	// fTimer�� �ּ� �� �ִ� �� ���̿��� �����ϰ� ����
	_float minTimer = 1.0f; // �ּ� �ð�
	_float maxTimer = 1.9f; // �ִ� �ð�
	Desc.fTimer = minTimer + static_cast<_float>(rand()) / RAND_MAX * (maxTimer - minTimer);


#pragma region Ű����

	vector<CQTE_Hit_UI_Icon::KEY_ID> possibleKeys = {
		CQTE_Hit_UI_Icon::KEY_ID::HIT_KEY_LIGHT,
		CQTE_Hit_UI_Icon::KEY_ID::HIT_KEY_MEDIUM,
		CQTE_Hit_UI_Icon::KEY_ID::HIT_KEY_ULTIMATE,
		CQTE_Hit_UI_Icon::KEY_ID::HIT_KEY_HEAVY
	};


	// �����ϰ� Ű �ϳ� ����
	for (auto& iter : m_vecHitUIIcon)
	{
		_bool isActive = iter->IsActive();
		if (!isActive)
			continue;

		CQTE_Hit_UI_Icon::KEY_ID key = iter->m_Key;

		auto it = std::find(possibleKeys.begin(), possibleKeys.end(), key);
		if (it != possibleKeys.end())
			possibleKeys.erase(it);
	}

	// ������ Ű ����� ����ִ��� Ȯ��
	if (possibleKeys.empty())
		return;

	// ���� Ű �߿��� �����ϰ� ����
	_int randomIndex = rand() % possibleKeys.size();
	Desc.key = possibleKeys[randomIndex];
	Desc.iTextureNumber = (_int)possibleKeys[randomIndex];

#pragma endregion

	Desc.Hit_Situation = this;

	// ������ ����
	CQTE_Hit_UI_Icon* ui_Icon = static_cast<CQTE_Hit_UI_Icon*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_QTE_Hit_UI_Icon"), &Desc));

	m_vecHitUIIcon.push_back(ui_Icon);
}


void CQTE_Hit_Situation::Handle_QTEInput()
{
	//�ش� ����� �����ϴ� ĳ����
	//�̰� ���߿� �����;���

	if (m_iCharacterSide == 1)
	{
		if (m_pGameInstance->Key_Down(DIK_U))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_LIGHT);
		}
		else if (m_pGameInstance->Key_Down(DIK_I))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_MEDIUM);
		}
		else if (m_pGameInstance->Key_Down(DIK_J))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_ULTIMATE);
		}
		else if (m_pGameInstance->Key_Down(DIK_K))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_HEAVY);
		}
	}
	else if (m_iCharacterSide == 2)
	{
		if (m_pGameInstance->Key_Down(DIK_NUMPAD7))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_LIGHT);
		}
		else if (m_pGameInstance->Key_Down(DIK_NUMPAD8))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_MEDIUM);
		}
		else if (m_pGameInstance->Key_Down(DIK_NUMPAD4))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_ULTIMATE);
		}
		else if (m_pGameInstance->Key_Down(DIK_NUMPAD5))
		{
			Process_Command(CQTE_Hit_UI_Icon::HIT_KEY_HEAVY);
		}
	}
}


void CQTE_Hit_Situation::Process_Command(CQTE_Hit_UI_Icon::KEY_ID input)
{
	//Ű�� �Է��� ��, ���� ����ִ� QTE�� ���� �۵��ؾ� �Ѵ�
	for (auto& iter : m_vecHitUIIcon)
	{
		_bool isActive = iter->IsActive();
		if (!isActive)
			continue;

		//����ִ� ��ü�� Ű
		CQTE_Hit_UI_Icon::KEY_ID key = iter->m_Key;

		//for���� �����鼭 Input�� �ȸ����� �н�
		if (input != key)
			continue;
		//Input�� ������ ��� �Ǵ��϶�� ��
		else
		{
			_bool isFinal = false;

			//�̹� ���;��� ��ü�� ���� �����ְ�
			if (m_vecHitUIIcon.size() == m_iCreate_Num)
			{
				isFinal = true;

				for (auto& iter : m_vecHitUIIcon)
				{
					if (iter->m_Key != input)
					{
						_bool isActive = iter->IsActive();

						//Braek�� ������ ���������� if���� ������ �ƴϴ�.
						if (isActive)
						{
							isFinal = false;
							break;
						}
					}
				}
			}

			iter->Send_Input(input, isFinal);


			//�Է��� ������ ���� Situation���� Result ��ü�� ó��
			//�׷��� �Է��� �ȴ����� �ð��� �� �����Ÿ� �ش� ������ ��ü�� Situation ���� Notify�Ѵ�.
#pragma region Result ��ü ����

			// Result ��ü ���� �Լ� ȣ��
			Create_ResultObject(iter);
			Create_ParticleObject(iter);
#pragma endregion
		}
	}
}

void CQTE_Hit_Situation::Create_ResultObject(CQTE_Hit_UI_Icon* pIcon)
{
	int iTextureNum = -1;
	switch (pIcon->m_currentResult_ID)
	{
	case CQTE_Hit_UI_Icon::HIT_RESULT_FAILED:
		m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::QTE_SAMEGRAB_FAIL_SFX, false, 0.7f);
		iTextureNum = 0;
		break;
	case CQTE_Hit_UI_Icon::HIT_RESULT_GOOD:
		iTextureNum = 1;
		m_pMain_Camera->StartCameraShake(0.1, 0.1);
		m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::QTE_SUCCESS_SFX, false, 0.7f);

		break;
	case CQTE_Hit_UI_Icon::HIT_RESULT_EXCELLENT:
		iTextureNum = 2;
		m_pMain_Camera->StartCameraShake(0.1, 0.1);
		m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::QTE_SUCCESS_SFX, false, 0.7f);
		break;
	case CQTE_Hit_UI_Icon::HIT_RESULT_PERFECT:
		iTextureNum = 3;
		m_pMain_Camera->StartCameraShake(0.1, 0.1);
		m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::QTE_SUCCESS_SFX, false, 0.7f);
		break;
	default:
		iTextureNum = 0;
		break;
	}

	CQTE_Hit_UI_Result::Hit_RESULT_DESC Desc{};
	_float OffsetY = -70.f;

	Desc.fX = pIcon->m_fX;
	Desc.fY = pIcon->m_fY + OffsetY;
	Desc.fSizeX = 300.f;
	Desc.fSizeY = 200.f;
	Desc.iTextureNum = iTextureNum;
	Desc.fTimer = 1.5f;

	CQTE_Hit_UI_Result* Result = static_cast<CQTE_Hit_UI_Result*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_QTE_Hit_UI_Result"), &Desc));
	m_vecHitResult.push_back(Result);
}

void CQTE_Hit_Situation::Create_ParticleObject(CQTE_Hit_UI_Icon* pIcon)
{
	//������ �� ��
	if (pIcon->m_currentResult_ID == CQTE_Hit_UI_Icon::HIT_RESULT_FAILED)
		return;

	CQTE_Hit_UI_Particle::Hit_PARTICLE_DESC Desc{};
	Desc.fX = pIcon->m_fX;
	Desc.fY = pIcon->m_fY;
	Desc.fSizeX = 600.f;
	Desc.fSizeY = 600.f;

	CQTE_Hit_UI_Particle* effect = static_cast<CQTE_Hit_UI_Particle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_QTE_Hit_UI_Particle"), &Desc));
	m_vecHitParticle.push_back(effect);

}

CQTE_Hit_Situation* CQTE_Hit_Situation::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CQTE_Hit_Situation* pInstance = new CQTE_Hit_Situation(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CQTE_Hit_Situation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CQTE_Hit_Situation::Clone(void* pArg)
{
	CQTE_Hit_Situation* pInstance = new CQTE_Hit_Situation(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CQTE_Hit_Situation"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQTE_Hit_Situation::Free()
{
	for (auto& iter : m_vecHitUIIcon)
		Safe_Release(iter);

	for (auto& iter : m_vecHitResult)
		Safe_Release(iter);

	for (auto& iter : m_vecHitParticle)
		Safe_Release(iter);

	m_vecHitUIIcon.clear();
	m_vecHitResult.clear();
	m_vecHitParticle.clear();

	__super::Free();
}

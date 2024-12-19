#include "stdafx.h"
#include "Particle_Manager.h"
#include "GameInstance.h"
#include "Particle.h"

IMPLEMENT_SINGLETON(CParticle_Manager)

CParticle_Manager::CParticle_Manager()
	: m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CParticle_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CParticle* pParticle = nullptr;
	CParticle::PARTICLE_DESC Desc{};

	// �� ��ƼŬ Ÿ�Ժ��� �ּ� 5���� ��ƼŬ ��ü�� �̸� �����Ͽ� Ǯ�� �߰�
	for (int i = 0; i < 3; ++i)
	{
		//������ 1��
		Desc.fXScale = 7.f;
		Desc.fYScale = 0.5f;
		Desc.fGlowFactor = 15.f;
		Desc.iPassIndex = 2;
		Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

		pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Focus"), &Desc));
		pParticle->Set_Particle_Active(false);
		m_ParticlePools[FREIZA_ULTIMATE_1_PARTICLE].push_back(pParticle);

		//������ 3��
		Desc.fXScale = 7.f;
		Desc.fYScale = 0.5f;
		Desc.fGlowFactor = 15.f;
		Desc.iPassIndex = 2;
		Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

		pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Spread"), &Desc));
		pParticle->Set_Particle_Active(false);
		m_ParticlePools[FREIZA_ULTIMATE_3_PARTICLE].push_back(pParticle);

		//������ 1�� ��Ʈ
		Desc.fXScale = 7.f;
		Desc.fYScale = 0.5f;
		Desc.fGlowFactor = 15.f;
		Desc.iPassIndex = 2;
		Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

		pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Frieza_1_Ultimate_Hit"), &Desc));
		pParticle->Set_Particle_Active(false);
		m_ParticlePools[FREIZA_ULTIMATE_1_HIT_PARTICLE].push_back(pParticle);

		//������ 3�� ��Ʈ
		Desc.fXScale = 7.f;
		Desc.fYScale = 0.5f;
		Desc.fGlowFactor = 15.f;
		Desc.iPassIndex = 2;
		Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

		pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Frieza_3_Ultimate_Hit"), &Desc));
		pParticle->Set_Particle_Active(false);
		m_ParticlePools[FREIZA_ULTIMATE_3_HIT_PARTICLE].push_back(pParticle);

		//21 3��
		Desc.fXScale = 1.f;
		Desc.fYScale = 0.07f;
		Desc.fGlowFactor = 15.f;
		Desc.iPassIndex = 2;
		Desc.vColor = _float4(1.f, 0.f, 0.8f, 1.0f);

		pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_21_3_Ultimate"), &Desc));
		pParticle->Set_Particle_Active(false);
		m_ParticlePools[ULTIMATE_3_21_PARTICLE].push_back(pParticle);
	}

	

	for (size_t i = 0; i < 10; i++)
	{
		//���� Hit
		Desc.fXScale = 3.f;
		Desc.fYScale = 0.5f;
		Desc.fGlowFactor = 20.f;
		Desc.iPassIndex = 2;
		Desc.vColor = _float4(1.f, 1.f, 0.f, 1.0f);

		pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Common_Hit"), &Desc));
		pParticle->Set_Particle_Active(false);
		m_ParticlePools[COMMON_HIT_PARTICLE].push_back(pParticle);
	}
	return S_OK;
}

void CParticle_Manager::Camera_Update(_float fTimeDelta)
{
}


void CParticle_Manager::Update(_float fTimeDelta)
{
	for (auto& poolPair : m_ParticlePools)
	{
		auto& pool = poolPair.second;
		for (auto& particle : pool)
		{
			if (particle->IsActive())
			{
				particle->Update(fTimeDelta);
			}
		}
	}
}

void CParticle_Manager::Late_Update(_float fTimeDelta)
{
	for (auto& poolPair : m_ParticlePools)
	{
		auto& pool = poolPair.second;
		for (auto& particle : pool)
		{
			if (particle->IsActive())
			{
				particle->Late_Update(fTimeDelta);
			}
		}
	}
}

HRESULT CParticle_Manager::Render(_float fTimeDelta)
{
	return S_OK;
}

HRESULT CParticle_Manager::Play(PARTICLE_ID eID, const _float3& vPosition)
{
	auto& pool = m_ParticlePools[eID];
	CParticle* pParticle = nullptr;

	//Ǯ�� ����� 0�� �ƴѻ�Ȳ��
	if (pool.size() != 0)
	{
		// ��Ȱ��ȭ�� ��ƼŬ�� Ǯ���� �˻�
		for (auto& particle : pool)
		{
			if (!particle->IsActive())
			{
				pParticle = particle;
				break;
			}
		}
	}

	if (pParticle == nullptr)
	{
		CParticle::PARTICLE_DESC Desc{};

		// ��Ȱ��ȭ�� ��ƼŬ�� ������ ���� ����
		switch (eID)
		{
		case FREIZA_ULTIMATE_3_PARTICLE:
			//������ 3��
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Spread"), &Desc));
			pool.push_back(pParticle);
			break;

		case FREIZA_ULTIMATE_1_PARTICLE:
			//������ 1��
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Focus"), &Desc));
			pool.push_back(pParticle);
			break;

		case FREIZA_ULTIMATE_1_HIT_PARTICLE:
			//������ 1�� ��Ʈ
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Frieza_1_Ultimate_Hit"), &Desc));
			pool.push_back(pParticle);
			break;

		case FREIZA_ULTIMATE_3_HIT_PARTICLE:
			//������ 3�� ��Ʈ
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Frieza_3_Ultimate_Hit"), &Desc));
			pool.push_back(pParticle);
			break;

		case ULTIMATE_3_21_PARTICLE:
			//21 3��
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_21_3_Ultimate"), &Desc));
			pool.push_back(pParticle);
			break;

		case COMMON_HIT_PARTICLE:
			//���� Hit
			Desc.fXScale = 3.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 20.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(1.f, 1.f, 0.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Common_Hit"), &Desc));
			pool.push_back(pParticle);
			break;
		}
	}

	//��ƼŬ Ȱ��ȭ �� �ʱ�ȭ
	pParticle->Set_Particle_Active(true); // �ʱ⿡�� ��Ȱ��ȭ ����
	pParticle->Set_Position(vPosition);

	// �߰����� �ʱ�ȭ�� �ʿ��ϸ� ���⿡ ����

	return S_OK;
}

HRESULT CParticle_Manager::Play(PARTICLE_ID eID, const _vector& vPosition)
{
	auto& pool = m_ParticlePools[eID];
	CParticle* pParticle = nullptr;

	//Ǯ�� ����� 0�� �ƴѻ�Ȳ��
	if (pool.size() != 0)
	{
		// ��Ȱ��ȭ�� ��ƼŬ�� Ǯ���� �˻�
		for (auto& particle : pool)
		{
			if (!particle->IsActive())
			{
				pParticle = particle;
				break;
			}
		}
	}

	if (pParticle == nullptr)
	{
		CParticle::PARTICLE_DESC Desc{};

		// ��Ȱ��ȭ�� ��ƼŬ�� ������ ���� ����
		switch (eID)
		{
		case FREIZA_ULTIMATE_3_PARTICLE:
			//������ 3��
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Spread"), &Desc));
			pool.push_back(pParticle);
			break;

		case FREIZA_ULTIMATE_1_PARTICLE:
			//������ 1��
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Focus"), &Desc));
			pool.push_back(pParticle);
			break;

		case FREIZA_ULTIMATE_1_HIT_PARTICLE:
			//������ 1�� ��Ʈ
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Frieza_1_Ultimate_Hit"), &Desc));
			pool.push_back(pParticle);
			break;

		case FREIZA_ULTIMATE_3_HIT_PARTICLE:
			//������ 3�� ��Ʈ
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Frieza_3_Ultimate_Hit"), &Desc));
			pool.push_back(pParticle);
			break;

		case ULTIMATE_3_21_PARTICLE:
			//21 3��
			Desc.fXScale = 7.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 15.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(0.4f, 0.f, 1.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_21_3_Ultimate"), &Desc));
			pool.push_back(pParticle);
			break;

		case COMMON_HIT_PARTICLE:
			//���� Hit
			Desc.fXScale = 3.f;
			Desc.fYScale = 0.5f;
			Desc.fGlowFactor = 20.f;
			Desc.iPassIndex = 2;
			Desc.vColor = _float4(1.f, 1.f, 0.f, 1.0f);

			pParticle = static_cast<CParticle*>(m_pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Particle_Common_Hit"), &Desc));
			pool.push_back(pParticle);
			break;
		}
	}

	//��ƼŬ Ȱ��ȭ �� �ʱ�ȭ
	pParticle->Set_Particle_Active(true); // �ʱ⿡�� ��Ȱ��ȭ ����
	pParticle->Set_Position(vPosition);

	// �߰����� �ʱ�ȭ�� �ʿ��ϸ� ���⿡ ����

	return S_OK;
}

void CParticle_Manager::Stop(PARTICLE_ID eID)
{
	auto& pool = m_ParticlePools[eID];

	for (auto& iter : pool)
	{
		if (iter->IsActive())
			iter->Set_Particle_Active(false);
	}
}


void CParticle_Manager::Free()
{
	Safe_Release(m_pGameInstance);

	for (auto& poolPair : m_ParticlePools)
	{
		auto& pool = poolPair.second;
		for (auto& particle : pool)
		{
			Safe_Release(particle);
		}
		pool.clear();
	}

	__super::Free();
}

#include "..\Public\Collider_Manager.h"

#include "GameObject.h"
#include "Component.h"
#include "GameInstance.h"
#include <chrono> // �ð� ������ ���� �߰�
CCollider_Manager::CCollider_Manager()
	:m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

void CCollider_Manager::Update(_float fTimeDelta)
{
	Check_Collision(fTimeDelta);
}

HRESULT CCollider_Manager::Add_ColliderObject(COLLIDERGROUP eColliderGroup, CCollider* pColliderCom)
{
	if (pColliderCom == nullptr || eColliderGroup >= CG_END)
		return E_FAIL;

	Safe_AddRef(pColliderCom);

	m_Colliders[eColliderGroup].emplace_back(pColliderCom);

	return S_OK;
}


HRESULT CCollider_Manager::Check_Collision(_float fTimeDelta)
{
	// ���� �������� �浹 ��� �ʱ�ȭ
	m_CollisionResults.clear();

	// ��� �ݶ��̴��� m_isColl �÷��׸� false�� �ʱ�ȭ
	for (auto& group : m_Colliders)
	{
		for (auto& collider : group)
			collider->m_isColl = false;
	}


	// �浹 �� ����
	vector<pair<CCollider*, CCollider*>> collisionPairs;

	// ���� �͸��Լ��� �浹 �� ����
	auto AddCollisionPairs = [&](COLLIDERGROUP group1, COLLIDERGROUP group2) {
		for (auto& collider1 : m_Colliders[group1]) {
			for (auto& collider2 : m_Colliders[group2]) {
				collisionPairs.emplace_back(collider1, collider2);
			}
		}
		};

	// 1P_BODY vs 2P_BODY
	AddCollisionPairs(CG_1P_BODY, CG_2P_BODY);

	// 1P_BODY vs 2P_SKILL
	AddCollisionPairs(CG_1P_BODY, CG_2P_Energy_Attack);

	// 1P_SKILL vs 2P_BODY
	AddCollisionPairs(CG_1P_Energy_Attack, CG_2P_BODY);

	// 1P_SKILL vs 2P_SKILL
	AddCollisionPairs(CG_1P_Energy_Attack, CG_2P_Energy_Attack);

	// 1P_RANGED_Attack vs 2P_Body
	AddCollisionPairs(CG_1P_Ranged_Attack, CG_2P_BODY);

	AddCollisionPairs(CG_1P_Ranged_Attack, CG_2P_Energy_Attack);

	AddCollisionPairs(CG_1P_Ranged_Attack, CG_2P_Ranged_Attack);

	AddCollisionPairs(CG_1P_Melee_Attack, CG_2P_BODY);

	AddCollisionPairs(CG_1P_BODY, CG_2P_Ranged_Attack);

	AddCollisionPairs(CG_1P_Energy_Attack, CG_2P_Ranged_Attack);

	AddCollisionPairs(CG_1P_BODY, CG_2P_Melee_Attack);

	// 1P_MELEE_ATTACK vs 2P_MELEE_ATTACK
	AddCollisionPairs(CG_1P_Melee_Attack, CG_2P_Melee_Attack);

	// 1P_RANGE_ATTACK vs 2P_MELEE_ATTACK
	AddCollisionPairs(CG_1P_Ranged_Attack, CG_2P_Melee_Attack);
	// 1P_MELEE_ATTACK vs 2P_RANGE_ATTACK
	AddCollisionPairs(CG_1P_Melee_Attack, CG_2P_Ranged_Attack);

	// 1P_REFLECT vs 2P_Energy_Attack
	AddCollisionPairs(CG_1P_REFLECT, CG_2P_Energy_Attack);
	// 1P_REFLECT vs 2P_Ranged_Attack
	AddCollisionPairs(CG_1P_REFLECT, CG_2P_Ranged_Attack);
	// 1P_REFLECT vs 2P_Melee_Attack
	AddCollisionPairs(CG_1P_REFLECT, CG_2P_Melee_Attack);

	// 1P_Energy_Attack vs 2P_REFLECT
	AddCollisionPairs(CG_1P_Energy_Attack, CG_2P_REFLECT);
	// 1P_Ranged_Attack vs 2P_REFLECT
	AddCollisionPairs(CG_1P_Ranged_Attack, CG_2P_REFLECT);
	// 1P_Melee_Attack vs 2P_REFLECT
	AddCollisionPairs(CG_1P_Melee_Attack, CG_2P_REFLECT);

	// �ð� ���� ����
	auto startTime = std::chrono::high_resolution_clock::now();

	// ������Ǯ�� ����Ͽ� �۾� ���� �� �浹 �˻� ����
	size_t numThreads = m_pGameInstance->Get_ThreadNumber();
	size_t totalPairs = collisionPairs.size();
	size_t pairsPerThread = (totalPairs + numThreads - 1) / numThreads; // �ø� ���

	//future<void>�� ��ȯ���� ���� �񵿱� �۾��� ��Ÿ����
	vector<future<void>> futures;

	for (size_t t = 0; t < numThreads; ++t) {
		size_t startIdx = t * pairsPerThread;
		if (startIdx >= totalPairs)
			break;

		size_t endIdx = min(startIdx + pairsPerThread, totalPairs);

		// �� �����尡 ó���� �浹 �� �κ� ���� ����
		vector<pair<CCollider*, CCollider*>> taskPairs(
			collisionPairs.begin() + startIdx,
			collisionPairs.begin() + endIdx
		);

		// ������Ǯ�� �۾� ����
		futures.emplace_back(m_pGameInstance->EnqueueTask(

			//Task�� �����Լ�
			[this, taskPairs, fTimeDelta]() {
				vector<pair<CCollider*, CCollider*>> localCollisions;

				for (const auto& pair : taskPairs) {
					if (IsColliding(pair.first, pair.second)) {
						localCollisions.emplace_back(pair);
					}
				}

				// ������ �����ϰ� ����� ����
				{
					lock_guard<mutex> lock(m_ResultMutex);
					m_CollisionResults.insert(m_CollisionResults.end(), localCollisions.begin(), localCollisions.end());
				}
			}));
	}

	// ��� �۾� �Ϸ� ���
	for (auto& future : futures) {
		future.get();
	}

	// �ð� ���� ����
	auto endTime = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> elapsedTime = endTime - startTime;
	double elapsedSeconds = elapsedTime.count();

	// �浹 ��� ó��
	ProcessCollisionResults(fTimeDelta);

	return S_OK;
}


//�浹 ��� ó��
void CCollider_Manager::ProcessCollisionResults(_float fTimeDelta)
{
	map<pair<CCollider*, CCollider*>, _bool> currentCollisions;

	// �׷캰�� �浹 ���� �����ϴ� �����̳�
	// �ѹ��� ó���ϱ� ������
	vector<pair<CCollider*, CCollider*>> Energy_1P_Skill_VS_Energy_2P_Skill_Collisions;
	vector<pair<CCollider*, CCollider*>> Energy_1P_Skill_VS_Body_2P_Collisions;
	vector<pair<CCollider*, CCollider*>> Body_1P_VS_Energy_2P_Skill_Collisions;

	vector<pair<CCollider*, CCollider*>> Ranged_Attack_1P_VS_Energy_2P_Skill_Collisions;
	vector<pair<CCollider*, CCollider*>> Energy_1P_Skill_VS_Ranged_2P_Skill_Collisions;

	vector<pair<CCollider*, CCollider*>> Reflect_1P_VS_Energy_2P_Skill_Collisions;
	vector<pair<CCollider*, CCollider*>> Energy_Attack_1P_VS_Reflect_2P_Collisions;


	// �� �����忡�� ������ �浹 ����� �ִٸ� m_CollisionResults�� ����
	for (const auto& pair : m_CollisionResults) {

		CCollider* colliderA = pair.first;
		CCollider* colliderB = pair.second;

		if (colliderA->GetMineGameObject()->m_bDead || colliderB->GetMineGameObject()->m_bDead)
			continue;

		//�浹 ó�� �Ϸ�
		colliderA->m_isColl = true;
		colliderB->m_isColl = true;

		// ������ bool ������ ����
		_bool is_1P_Body_Vs_2P_Energy_Skill = (colliderA->m_ColliderGroup == CG_1P_BODY && colliderB->m_ColliderGroup == CG_2P_Energy_Attack);
		_bool is_1P_Body_Vs_2P_Body = (colliderA->m_ColliderGroup == CG_1P_BODY && colliderB->m_ColliderGroup == CG_2P_BODY);
		_bool is_1P_Energy_Skill_Vs_2P_Energy_Skill = (colliderA->m_ColliderGroup == CG_1P_Energy_Attack && colliderB->m_ColliderGroup == CG_2P_Energy_Attack);
		_bool is_1P_Energy_Skill_Vs_2P_Body = (colliderA->m_ColliderGroup == CG_1P_Energy_Attack && colliderB->m_ColliderGroup == CG_2P_BODY);

		_bool is_1P_Ranged_Attack_Vs_2P_Body = (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_BODY);
		_bool is_1P_Ranged_Attack_Vs_2P_Energy_Skill = (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_Energy_Attack);
		_bool is_1P_Ranged_Attack_Vs_2P_Ranged_Attack = (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack);
		_bool is_1P_Melee_Attack_Vs_2P_Body = (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_BODY);
		_bool is_1P_Body_Vs_2P_Ranged_Attack = (colliderA->m_ColliderGroup == CG_1P_BODY && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack);
		_bool is_1P_Energy_Skill_Vs_2P_Ranged_Attack = (colliderA->m_ColliderGroup == CG_1P_Energy_Attack && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack);
		_bool is_1P_Body_Vs_2P_Melee_Attack = (colliderA->m_ColliderGroup == CG_1P_BODY && colliderB->m_ColliderGroup == CG_2P_Melee_Attack);
		_bool is_1P_Melee_Vs_2P_Melee_Attack = (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_Melee_Attack);

		_bool is_1P_Range_Vs_2P_Melee_Attack = (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_Melee_Attack);
		_bool is_1P_Melee_Vs_2P_Range_Attack = (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack);

		_bool is_1P_Reflect_Vs_2P_Energy_Attack = (colliderA->m_ColliderGroup == CG_1P_REFLECT && colliderB->m_ColliderGroup == CG_2P_Energy_Attack);
		_bool is_1P_Reflect_Vs_2P_Range_Attack = (colliderA->m_ColliderGroup == CG_1P_REFLECT && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack);
		_bool is_1P_Reflect_Vs_2P_Melee_Attack = (colliderA->m_ColliderGroup == CG_1P_REFLECT && colliderB->m_ColliderGroup == CG_2P_Melee_Attack);

		_bool is_1P_Energy_Attack_Vs_2P_Reflect = (colliderA->m_ColliderGroup == CG_1P_Energy_Attack && colliderB->m_ColliderGroup == CG_2P_REFLECT);
		_bool is_1P_Range_Attack_Vs_2P_Reflect = (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_REFLECT);
		_bool is_1P_Melee_Attack_Vs_2P_Reflect = (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_REFLECT);


		// �浹 �׷쿡 ���� ó��


		if (is_1P_Body_Vs_2P_Body)
		{
			//1P_Body VS 2P_Body
			Process_1P_Body_2P_Body(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Energy_Skill_Vs_2P_Energy_Skill)
		{
			//1P_Skill VS 2P_Skill
			Energy_1P_Skill_VS_Energy_2P_Skill_Collisions.push_back(pair);
		}

		if (is_1P_Ranged_Attack_Vs_2P_Ranged_Attack)
		{
			Process_1P_Ranged_Skill_2P_Ranged_Skill(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Melee_Vs_2P_Melee_Attack)
		{
			Process_1P_Melee_2P_Melee_Skill(pair, fTimeDelta, currentCollisions);
		}






		if (is_1P_Body_Vs_2P_Energy_Skill)
		{
			//1P_Body VS 2P_Skill
			Body_1P_VS_Energy_2P_Skill_Collisions.push_back(pair);
		}

		if (is_1P_Energy_Skill_Vs_2P_Body)
		{
			//1P_Skill VS 2P_Body
			Energy_1P_Skill_VS_Body_2P_Collisions.push_back(pair);
		}





		if (is_1P_Ranged_Attack_Vs_2P_Body)
		{
			Process_1P_Ranged_Skill_2P_Body(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Body_Vs_2P_Ranged_Attack)
		{
			Process_1P_Body_2P_Ranged_Skill(pair, fTimeDelta, currentCollisions);
		}





		if (is_1P_Melee_Attack_Vs_2P_Body)
		{
			Process_1P_Melee_Skill_2P_Body(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Body_Vs_2P_Melee_Attack)
		{
			Process_1P_Body_2P_Melee_Skill(pair, fTimeDelta, currentCollisions);
		}





		if (is_1P_Energy_Skill_Vs_2P_Ranged_Attack)
		{
			Energy_1P_Skill_VS_Ranged_2P_Skill_Collisions.push_back(pair);
		}

		if (is_1P_Ranged_Attack_Vs_2P_Energy_Skill)
		{
			Ranged_Attack_1P_VS_Energy_2P_Skill_Collisions.push_back(pair);
		}




		if (is_1P_Range_Vs_2P_Melee_Attack)
		{
			Process_1P_Range_2P_Melee_Skill(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Melee_Vs_2P_Range_Attack)
		{
			Process_1P_Melee_2P_Range_Skill(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Reflect_Vs_2P_Energy_Attack)
		{
			Reflect_1P_VS_Energy_2P_Skill_Collisions.push_back(pair);
		}

		if (is_1P_Reflect_Vs_2P_Range_Attack)
		{
			Process_1P_Reflect_2P_Range_Skill(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Reflect_Vs_2P_Melee_Attack)
		{
			Process_1P_Reflect_2P_Melee_Skill(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Energy_Attack_Vs_2P_Reflect)
		{
			Energy_Attack_1P_VS_Reflect_2P_Collisions.push_back(pair);
		}

		if (is_1P_Range_Attack_Vs_2P_Reflect)
		{
			Process_1P_Range_2P_Reflect(pair, fTimeDelta, currentCollisions);
		}

		if (is_1P_Melee_Attack_Vs_2P_Reflect)
		{
			Process_1P_Melee_2P_Reflect(pair, fTimeDelta, currentCollisions);
		}

	}

	// �� �׷캰�� �� ���� ó��
	if (!Energy_1P_Skill_VS_Energy_2P_Skill_Collisions.empty())
	{
		Process_1P_Energy_Skill_2P_Energy_Skill_Group(Energy_1P_Skill_VS_Energy_2P_Skill_Collisions, fTimeDelta, currentCollisions);
	}

	// �� �׷캰�� �� ���� ó��
	if (!Body_1P_VS_Energy_2P_Skill_Collisions.empty())
	{
		Process_1P_Body_2P_Energy_Skill_Group(Body_1P_VS_Energy_2P_Skill_Collisions, fTimeDelta, currentCollisions);
	}

	if (!Energy_1P_Skill_VS_Body_2P_Collisions.empty())
	{
		Process_1P_Energy_Skill_2P_Body_Group(Energy_1P_Skill_VS_Body_2P_Collisions, fTimeDelta, currentCollisions);
	}

	if (!Ranged_Attack_1P_VS_Energy_2P_Skill_Collisions.empty())
	{
		Process_1P_Ranged_Skill_2P_Energy_Skill_Group(Ranged_Attack_1P_VS_Energy_2P_Skill_Collisions, fTimeDelta, currentCollisions);
	}

	if (!Energy_1P_Skill_VS_Ranged_2P_Skill_Collisions.empty())
	{
		Process_1P_Energy_Skill_2P_Ranged_Skill_Group(Energy_1P_Skill_VS_Ranged_2P_Skill_Collisions, fTimeDelta, currentCollisions);
	}

	if (!Reflect_1P_VS_Energy_2P_Skill_Collisions.empty())
	{
		Process_1P_Reflect_2P_Energy_Skill_Group(Reflect_1P_VS_Energy_2P_Skill_Collisions, fTimeDelta, currentCollisions);
	}
	if (!Energy_Attack_1P_VS_Reflect_2P_Collisions.empty())
	{
		Process_1P_Energy_2P_Reflect_Group(Energy_Attack_1P_VS_Reflect_2P_Collisions, fTimeDelta, currentCollisions);
	}


	// �浹 �����丮 ������Ʈ
	m_CollisionHistory = currentCollisions;
}


void CCollider_Manager::Process_1P_Body_2P_Body(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� 1P Body�� 2P Body ���� �浹 �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		// �浹 ���� 1P Body�� 2P Body���� Ȯ��
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_BODY && colliderB->m_ColliderGroup == CG_2P_BODY)
		{

			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}
}

void CCollider_Manager::Process_1P_Energy_Skill_2P_Energy_Skill_Group(const vector<pair<CCollider*, CCollider*>>& collisions, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	// ù ��° �浹 �ָ� ����Ͽ� ó��
	if (!collisions.empty())
	{
		const auto& pairCollider = collisions.front();

		if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
			pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
		}
		else {
			// �浹 ����
			pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
		}

		// ���� �浹 ���� ������Ʈ
		currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;
	}
}

void CCollider_Manager::Process_1P_Body_2P_Energy_Skill_Group(const vector<pair<CCollider*, CCollider*>>& collisions, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	// ù ��° �浹 �ָ� ����Ͽ� ó��
	if (!collisions.empty())
	{
		const auto& pairCollider = collisions.front();

		if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
			pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
		}
		else {
			// �浹 ����
			pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
		}

		// ���� �浹 ���� ������Ʈ
		currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;
	}
}

void CCollider_Manager::Process_1P_Energy_Skill_2P_Body_Group(const vector<pair<CCollider*, CCollider*>>& collisions, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	// ù ��° �浹 �ָ� ����Ͽ� ó��
	if (!collisions.empty())
	{
		const auto& pairCollider = collisions.front();

		if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
			pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
		}
		else {
			// �浹 ����
			pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
		}

		// ���� �浹 ���� ������Ʈ
		currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;
	}
}

void CCollider_Manager::Process_1P_Ranged_Skill_2P_Body(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_BODY)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}

	Destroy_Reserve(pairCollider.first);
}

void CCollider_Manager::Process_1P_Ranged_Skill_2P_Energy_Skill_Group(const vector<pair<CCollider*, CCollider*>>& collisions, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	// ù ��° �浹 �ָ� ����Ͽ� ó��
	if (!collisions.empty())
	{
		const auto& pairCollider = collisions.front();

		if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
			pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
		}
		else {
			// �浹 ����
			pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
		}

		// ���� �浹 ���� ������Ʈ
		currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

		//Melee_Attack
		Destroy_Reserve(pairCollider.first);
	}
}

void CCollider_Manager::Process_1P_Ranged_Skill_2P_Ranged_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}

	Destroy_Reserve(pairCollider.first);
	Destroy_Reserve(pairCollider.second);
}

void CCollider_Manager::Process_1P_Melee_Skill_2P_Body(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_BODY)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}

	Destroy_Reserve(pairCollider.first);
}

void CCollider_Manager::Process_1P_Body_2P_Ranged_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_BODY && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}

	Destroy_Reserve(pairCollider.second);
}

void CCollider_Manager::Process_1P_Energy_Skill_2P_Ranged_Skill_Group(const vector<pair<CCollider*, CCollider*>>& collisions, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	// ù ��° �浹 �ָ� ����Ͽ� ó��
	if (!collisions.empty())
	{
		const auto& pairCollider = collisions.front();

		if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
			pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
		}
		else {
			// �浹 ����
			pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
		}

		// ���� �浹 ���� ������Ʈ
		currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

		//Ranged_Attack
		Destroy_Reserve(pairCollider.second);
	}
}

void CCollider_Manager::Process_1P_Body_2P_Melee_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_BODY && colliderB->m_ColliderGroup == CG_2P_Melee_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}

	Destroy_Reserve(pairCollider.second);
}

void CCollider_Manager::Process_1P_Melee_2P_Melee_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_Melee_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}

	Destroy_Reserve(pairCollider.first);
	Destroy_Reserve(pairCollider.second);
}

void CCollider_Manager::Process_1P_Range_2P_Melee_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_Melee_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}
}

void CCollider_Manager::Process_1P_Melee_2P_Range_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}
}

void CCollider_Manager::Process_1P_Reflect_2P_Energy_Skill_Group(const vector<pair<CCollider*, CCollider*>>& collisions, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	// ù ��° �浹 �ָ� ����Ͽ� ó��
	if (!collisions.empty())
	{
		const auto& pairCollider = collisions.front();

		if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
			pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
		}
		else {
			// �浹 ����
			pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
		}

		// ���� �浹 ���� ������Ʈ
		currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;
	}
}

void CCollider_Manager::Process_1P_Reflect_2P_Range_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_REFLECT && colliderB->m_ColliderGroup == CG_2P_Ranged_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}
}

void CCollider_Manager::Process_1P_Reflect_2P_Melee_Skill(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_REFLECT && colliderB->m_ColliderGroup == CG_2P_Melee_Attack)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}
}

void CCollider_Manager::Process_1P_Energy_2P_Reflect_Group(const vector<pair<CCollider*, CCollider*>>& collisions, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	// ù ��° �浹 �ָ� ����Ͽ� ó��
	if (!collisions.empty())
	{
		const auto& pairCollider = collisions.front();

		if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
			pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
		}
		else {
			// �浹 ����
			pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
			pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
		}

		// ���� �浹 ���� ������Ʈ
		currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;
	}
}


void CCollider_Manager::Process_1P_Range_2P_Reflect(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Ranged_Attack && colliderB->m_ColliderGroup == CG_2P_REFLECT)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}
}

void CCollider_Manager::Process_1P_Melee_2P_Reflect(pair<CCollider*, CCollider*> pairCollider, _float fTimeDelta, map<pair<CCollider*, CCollider*>, _bool>& currentCollisions)
{
	if (m_CollisionHistory.find(pairCollider) == m_CollisionHistory.end() || !m_CollisionHistory[pairCollider]) {
		pairCollider.first->OnCollisionEnter(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionEnter(pairCollider.first, fTimeDelta);
	}
	else {
		// �浹 ����
		pairCollider.first->OnCollisionStay(pairCollider.second, fTimeDelta);
		pairCollider.second->OnCollisionStay(pairCollider.first, fTimeDelta);
	}

	// ���� �浹 ���� ������Ʈ
	currentCollisions[make_pair(pairCollider.first, pairCollider.second)] = true;

	// ���� �����ӿ��� �浹�� �͵� �� �̹� �����ӿ��� �浹���� ���� ��� ó��
	for (auto& pair : m_CollisionHistory) {
		CCollider* colliderA = pair.first.first;
		CCollider* colliderB = pair.first.second;

		if (colliderA->m_ColliderGroup == CG_1P_Melee_Attack && colliderB->m_ColliderGroup == CG_2P_REFLECT)
		{
			if (currentCollisions.find(pair.first) == currentCollisions.end()) {
				colliderA->OnCollisionExit(colliderB);
				colliderB->OnCollisionExit(colliderA);
				break;
			}
		}
	}
}


_bool CCollider_Manager::IsColliding(CCollider* _pSourCollider, CCollider* _pDestCollider)
{
	_bool isCol = _pSourCollider->isCollision(_pDestCollider);

	return isCol;
}

HRESULT CCollider_Manager::Release_Collider(const CCollider* targetCollider)
{
	// ����, m_CollisionHistory���� �ش� �ݶ��̴��� ���õ� ��� �浹 ������ �����ϰų� OnCollisionExit �̺�Ʈ�� ȣ���մϴ�.
	for (auto it = m_CollisionHistory.begin(); it != m_CollisionHistory.end(); )
	{
		if (it->first.first == targetCollider || it->first.second == targetCollider)
		{
			// �浹 ���� ����
			it = m_CollisionHistory.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (int i = CG_1P_BODY; i < CG_END; ++i) {

		auto it = find(m_Colliders[i].begin(), m_Colliders[i].end(), targetCollider);
		if (it != m_Colliders[i].end()) {
			CCollider* collider = *it;
			if (!collider->m_bDead)
			{
				Safe_Release(collider);

			}
			m_Colliders[i].erase(it);

			return S_OK; // ���������� ���� �� ����
		}
	}

	return E_FAIL; // �ش� collider�� ã�� ����
}

HRESULT CCollider_Manager::Destory_ColliderGroup()
{
	// ���� ���� �ݶ��̴� ����
	for (auto& pCollider : m_Destory_Reserve_Collider)
	{
		Release_Collider(pCollider);
	}

	m_Destory_Reserve_Collider.clear();

	// �׷� ��ü ����
	for (auto& group : m_Destroy_Reserve_Collider_Group)
	{
		for (auto& pCollider : m_Colliders[group])
			Safe_Release(pCollider);
		m_Colliders[group].clear();
	}
	m_Destroy_Reserve_Collider_Group.clear();
	return S_OK;
}

void CCollider_Manager::Destroy_Reserve(COLLIDERGROUP eRenderGroup)
{
	// ����Ʈ�� �̹� �����ϴ��� Ȯ��
	if (find(m_Destroy_Reserve_Collider_Group.begin(), m_Destroy_Reserve_Collider_Group.end(), eRenderGroup) == m_Destroy_Reserve_Collider_Group.end())
	{
		// �������� ������ �߰�
		m_Destroy_Reserve_Collider_Group.push_back(eRenderGroup);
	}
}

void CCollider_Manager::Destroy_Reserve(CCollider* pCollider)
{
	// �̹� ����Ʈ�� �����ϴ��� Ȯ��
	if (find(m_Destory_Reserve_Collider.begin(), m_Destory_Reserve_Collider.end(), pCollider) == m_Destory_Reserve_Collider.end())
	{
		m_Destory_Reserve_Collider.push_back(pCollider);
	}
}




CCollider_Manager* CCollider_Manager::Create()
{
	return new CCollider_Manager();
}

void CCollider_Manager::Free()
{
	__super::Free();

	for (size_t i = 0; i < CG_END; i++)
	{
		for (auto& pCollider : m_Colliders[i])
			Safe_Release(pCollider);
		m_Colliders[i].clear();
	}

	Safe_Release(m_pGameInstance);
}

#include "stdafx.h"
#include "..\Public\Virtual_Camera.h"
#include "GameInstance.h"
#include "Imgui_Manager.h"
#include "Character.h"
CVirtual_Camera::CVirtual_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{

}

CVirtual_Camera::CVirtual_Camera(const CVirtual_Camera& Prototype)
	: CCamera{ Prototype }
{

}

HRESULT CVirtual_Camera::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CVirtual_Camera::Initialize(void* pArg)
{
	CAMERA_DESC Desc{};

	//�ʱ� ��ġ
	m_vEye = _float3(0.f, 10.f, -10.f);
	//�ʱ� ����
	m_vAt = _float3(0.f, 0.f, 0.f);
	//ī�޶� �̸�
	m_Name = *static_cast<_char**>(pArg);
	//ī�޶� ���콺 �ΰ���
	m_fMouseSensor = 0.1f;
	//ī�޶� �̵��ӵ�
	Desc.fSpeedPerSec = 1.f;
	//���� ������ ���ǵ�
	m_fMoveSpeed = Desc.fSpeedPerSec;

	//�þ߰�
	Desc.fFovy = XMConvertToRadians(40.0f);
	//Near
	Desc.fNear = 0.1f;
	//Far
	Desc.fFar = 10000.f;
	//ī�޶� ȸ���ӵ�
	Desc.fRotationPerSec = XMConvertToRadians(90.0f);

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetW(XMLoadFloat3(&m_vEye), 1.f));
	m_pTransformCom->LookAt(XMVectorSetW(XMLoadFloat3(&m_vAt), 1.f));

	m_p1pPlayer = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"), 0);
	m_p2pPlayer = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"), 1);
	return S_OK;
}

void CVirtual_Camera::Camera_Update(_float fTimeDelta)
{
	switch (m_currentMode)
	{
	case CAMERA_FREE_MODE:
		Free_Camera(fTimeDelta);
		break;
	case CAMERA_NORMAL_MODE:
		Default_Camera(fTimeDelta);
		break;
	case CAMERA_MAP_MODE:
		Map_Camera(fTimeDelta);
		break;
	case CAMERA_CINEMATIC_MODE:
		if (m_currentPlayMode == Playing)
			Play(fTimeDelta);
		break;
	}

	if (m_bIsShaking)
		ApplyCameraShake(fTimeDelta);
}

void CVirtual_Camera::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CVirtual_Camera::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CVirtual_Camera::Render(_float fTimeDelta)
{
	return S_OK;
}

void CVirtual_Camera::Play(_float fTimeDelta)
{
	if (m_currentPlayMode != CAMERA_PLAY_MODE::Playing || m_AnimationIndex == -1)
		return; // ���� ���°� Playing�� �ƴϸ� ������Ʈ���� ����

	// ���� �ִϸ��̼��� ����Ʈ �� Ȯ��
	if (m_currentPointIndex >= m_mapPoints[m_AnimationIndex].size())
	{
		// ��� ����Ʈ�� Duration�� ������ Play ��� ����
		Stop();
		return;
	}

	CGameObject* player = { nullptr };

	if (m_pEnemy == nullptr)
		player = m_iTeam == 1 ? m_p1pPlayer : m_p2pPlayer;
	else
		player = m_pEnemy;

	// �÷��̾��� ���� �������� (1�̸� �״��, -1�̸� ����)
	CCharacter* character = static_cast<CCharacter*>(player);
	_int direction = character->Get_iDirection();

	// ���� ����Ʈ�� ���� ����Ʈ ����
	CameraPoint currentPoint = m_mapPoints[m_AnimationIndex][m_currentPointIndex];

	// ���� ȸ�� ������� üũ
	if (m_bIsCirclePlay)
	{
		m_circleDuration = currentPoint.duration;
		CirclePlay(fTimeDelta, currentPoint);
	}
	else
	{
		CameraPoint nextPoint = {};

		if (m_currentPointIndex + 1 < m_mapPoints[m_AnimationIndex].size())
			nextPoint = m_mapPoints[m_AnimationIndex][m_currentPointIndex + 1];
		else
			nextPoint = currentPoint;

		m_elapsedTime += fTimeDelta;

		if (m_elapsedTime >= currentPoint.duration)
		{
			// ���� ����Ʈ�� �̵�
			m_currentPointIndex++;
			m_elapsedTime = 0.0f;

			if (m_currentPointIndex >= m_mapPoints[m_AnimationIndex].size())
			{
				Stop();
				return;
			}

			currentPoint = m_mapPoints[m_AnimationIndex][m_currentPointIndex];
			if (m_currentPointIndex + 1 < m_mapPoints[m_AnimationIndex].size())
				nextPoint = m_mapPoints[m_AnimationIndex][m_currentPointIndex + 1];
			else
				nextPoint = currentPoint;
		}

		// ���� ���� ���
		float t = m_elapsedTime / currentPoint.duration;

		// ���� ��Ŀ� ���� t �� ����
		switch (currentPoint.interpolationType)
		{
		case InterpolationType::INTERPOLATION_LINEAR_MODE:
			// t�� �״�� ���
			break;
		case InterpolationType::INTERPOLATION_DAMPING_MODE:
			t = AdjustT_Damping(t, currentPoint.damping);
			break;
		case InterpolationType::INTERPOLATION_SKIP_MODE:
			t = 1.0f;
			break;
		}

		// **1. ���� ������ ����**
		_vector interpolatedPositionLocal;
		if (currentPoint.interpolationType != InterpolationType::INTERPOLATION_SKIP_MODE)
		{
			interpolatedPositionLocal = XMVectorLerp(XMLoadFloat3(&currentPoint.position), XMLoadFloat3(&nextPoint.position), t);
		}
		else
		{
			interpolatedPositionLocal = XMLoadFloat3(&nextPoint.position);
		}

		// **3. ���� ���� ��� �ε� (�����ϸ� ����)**
		_matrix modelWorldMatrix = Float4x4ToMatrix(*currentPoint.pWorldFloat4x4);

		// �¿� ������ ī�޶� ������ ���� ������� �༮�� �����ϸ� �����ؾ� ��
		if (direction == -1 && m_bIsIgnoreFlip) {
			// **�����ϸ� ���Ÿ� ���� ��� ����**
			_vector modelScale;
			_vector modelRotationQuat;
			_vector modelTranslation;
			XMMatrixDecompose(&modelScale, &modelRotationQuat, &modelTranslation, modelWorldMatrix);

			// **�����ϸ��� ���ŵ� ���� ���� ��� �籸��**
			_matrix modelRotationMatrix = XMMatrixRotationQuaternion(modelRotationQuat);
			_matrix modelTranslationMatrix = XMMatrixTranslationFromVector(modelTranslation);
			modelWorldMatrix = modelRotationMatrix * modelTranslationMatrix;
		}


		// **4. ���� �������� ���� ���������� ��ȯ (�����ϸ� ����)**
		_vector interpolatedPositionWorld = XMVector3TransformCoord(interpolatedPositionLocal, modelWorldMatrix);

		// **2. ���� ȸ�� ���� (Quaternion Slerp ���)**
		_vector interpolatedRotationLocal;
		if (currentPoint.interpolationType != InterpolationType::INTERPOLATION_SKIP_MODE)
		{
			_vector q1 = XMLoadFloat4(&currentPoint.rotation);
			_vector q2 = XMLoadFloat4(&nextPoint.rotation);
			interpolatedRotationLocal = XMQuaternionSlerp(q1, q2, t);
		}
		else
		{
			interpolatedRotationLocal = XMLoadFloat4(&nextPoint.rotation);
		}

		//**direction�� ���� ȸ�� ����**
		//���� Flip�� �����ϴ� �Ӽ��� false���� ������Ű��
		if (direction == -1 && m_bIsIgnoreFlip == false)
		{
			// ���ʹϾ��� Y ���� ����
			interpolatedRotationLocal = XMVectorSet(
				XMVectorGetX(interpolatedRotationLocal),
				-XMVectorGetY(interpolatedRotationLocal),
				XMVectorGetZ(interpolatedRotationLocal),
				XMVectorGetW(interpolatedRotationLocal));
		}

		// **6. ���� ȸ���� ���� ȸ������ ��ȯ**
		_matrix interpolatedRotationMatrixLocal = XMMatrixRotationQuaternion(interpolatedRotationLocal);

		// **7. ī�޶��� ���� ��� ���� (�����ϸ� ����)**
		_matrix NewWorldMatrix = interpolatedRotationMatrixLocal;
		NewWorldMatrix.r[3] = XMVectorSetW(interpolatedPositionWorld, 1.0f); // ��ġ ����

		// ���� ���� ����
		_vector right = NewWorldMatrix.r[0];
		_vector up = NewWorldMatrix.r[1];
		_vector look = NewWorldMatrix.r[2];

		// ��ġ ���� (����ũ ������ ����)
		_vector position = interpolatedPositionWorld + m_vShakeOffset;

		// Transform ������Ʈ ������Ʈ
		m_pTransformCom->Set_State(CTransform::STATE_RIGHT, right);
		m_pTransformCom->Set_State(CTransform::STATE_UP, up);
		m_pTransformCom->Set_State(CTransform::STATE_LOOK, look);
		m_pTransformCom->Set_State(CTransform::STATE_POSITION, position);

		//cout << XMVectorGetX(m_pTransformCom->Get_State(CTransform::STATE_POSITION)) << "  " << XMVectorGetY(m_pTransformCom->Get_State(CTransform::STATE_POSITION)) << "  " << XMVectorGetZ(m_pTransformCom->Get_State(CTransform::STATE_POSITION)) << endl;

	}
}

// ���� ȸ�� �Լ� ����
// ���� ȸ�� �Լ� ����
void CVirtual_Camera::CirclePlay(float fTimeDelta, const CameraPoint& currentPoint)
{
	// ���� ���� ��� �ε�
	XMMATRIX modelWorldMatrix = Float4x4ToMatrix(*currentPoint.pWorldFloat4x4);

	// �¿� ������ ���� �����ϸ� ����
	CCharacter* character = static_cast<CCharacter*>(m_pEnemy != nullptr ? m_pEnemy : (m_iTeam == 1 ? m_p1pPlayer : m_p2pPlayer));
	int direction = character->Get_iDirection();

	if (direction == -1 && m_bIsIgnoreFlip)
	{
		// �����ϸ� ���Ÿ� ���� ��� ����
		XMVECTOR modelScale, modelRotationQuat, modelTranslation;
		XMMatrixDecompose(&modelScale, &modelRotationQuat, &modelTranslation, modelWorldMatrix);

		// �����ϸ��� ���ŵ� ���� ���� ��� �籸��
		XMMATRIX modelRotationMatrix = XMMatrixRotationQuaternion(modelRotationQuat);
		XMMATRIX modelTranslationMatrix = XMMatrixTranslationFromVector(modelTranslation);
		modelWorldMatrix = modelRotationMatrix * modelTranslationMatrix;
	}

	// ���� ���� ��ġ ��������
	XMVECTOR modelPosition = modelWorldMatrix.r[3];

	// �ʱ� ī�޶� ��ġ (����)
	XMVECTOR initialCameraPositionLocal = XMLoadFloat3(&currentPoint.position);

	// �ʱ� ī�޶� ��ġ (����)
	XMVECTOR initialCameraPositionWorld = XMVector3TransformCoord(initialCameraPositionLocal, modelWorldMatrix);

	// ������ ��� (ī�޶�� �� �� �Ÿ�)
	m_circleRadius = XMVectorGetX(XMVector3Length(initialCameraPositionWorld - modelPosition));

	// �ð� ������Ʈ
	m_circleElapsedTime += fTimeDelta;

	// ���� ������Ʈ (ȸ�� �ӵ��� ���� ���� ����)
	float angleIncrement = m_rotationSpeed * fTimeDelta;
	m_circleAngle += m_bIsClockwise ? angleIncrement : -angleIncrement;

	// ��ü ȸ�� ���� ���� (�ʿ� ��)
	// ���� ���, m_circleAngle�� 2�� �ʰ��ϰų� -2�� �̸��� �� �ʱ�ȭ�� �� �ֽ��ϴ�.
	// ����� ��� �����ǹǷ�, ū ���������� ������ ������ �մϴ�.

	// ���ο� ī�޶� ��ġ ��� (Y�� ���� ȸ��)
	_float x = m_circleRadius * cosf(m_circleAngle);
	_float z = m_circleRadius * sinf(m_circleAngle);
	// Y�� ���̴� �ʱ� ī�޶��� Y ��ġ�� ����
	_float y = XMVectorGetY(initialCameraPositionWorld - modelPosition);

	XMVECTOR newCameraPositionOffset = XMVectorSet(x, y, z, 0.0f);
	XMVECTOR newCameraPosition = modelPosition + newCameraPositionOffset;

	// ī�޶� ���� �ٶ󺸵��� ���� ���� ���
	XMVECTOR lookDirection = XMVector3Normalize(modelPosition - newCameraPosition);

	// Up ���� ���� (Y��)
	XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Right ���� ���
	XMVECTOR rightDirection = XMVector3Normalize(XMVector3Cross(upDirection, lookDirection));

	// Up ���� ���� (������ȭ)
	upDirection = XMVector3Cross(lookDirection, rightDirection);

	// ���ο� ���� ��� ����
	XMMATRIX NewWorldMatrix;
	NewWorldMatrix.r[0] = rightDirection;
	NewWorldMatrix.r[1] = upDirection;
	NewWorldMatrix.r[2] = lookDirection;
	NewWorldMatrix.r[3] = XMVectorSetW(newCameraPosition, 1.0f);

	// Transform ������Ʈ ������Ʈ
	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, NewWorldMatrix.r[0]);
	m_pTransformCom->Set_State(CTransform::STATE_UP, NewWorldMatrix.r[1]);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, NewWorldMatrix.r[2]);
	_vector position = m_vShakeOffset + NewWorldMatrix.r[3];
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, position);

	// ���� ȸ���� �Ϸ�Ǿ����� üũ
	if (m_circleElapsedTime >= m_circleDuration)
	{
		Stop();
	}
}

void CVirtual_Camera::Set_Camera_Position(_float averageX, _float distanceX, _float higherY, _gvector pos1, _gvector pos2)
{

	const float fixedZ = -5.f; //Z�� ����
	const float baseFixedY = 1.5f;  // Y�� 0�� �� ����� ������

	//�÷��̾� ���� thresholdDistance ���� �־����� �������� ī�޶� �־�����
	const float thresholdDistance = 2.f;
	//maxDistance������ ����ϸ鼭 �þ�� ���⼭���ʹ� �� �þ�� ����
	const float maxDistance = 5.17f;

	//���� Y ����
	float dynamicY = baseFixedY;

	// higherY�� 0���� ū ��� ���� �������� �߰�
	if (higherY > 0.f)
	{
		// higherY ���� ���� baseFixedY���� ���� ���� ����
		const float maxReduction = 1.5f;  // �ִ� Y ���Ұ�

		// higherY�� Ŀ������ dynamicY�� 1���� �� ���� ����
		dynamicY = baseFixedY - min(maxReduction, higherY * 0.8f);
	}

	// ���� Ÿ�� ��ġ�� ����
	_float3 targetPosition = _float3(averageX, higherY + dynamicY, fixedZ);

	// DistanceX�� thresholdDistance�� ���� �� ī�޶��� Z�� Y�� �����ϴ� ���� ���� ����
	if (distanceX > thresholdDistance)
	{
		float t = (distanceX - thresholdDistance) / (maxDistance - thresholdDistance);
		t = max(0.f, min(t, 1.f)); // [0, 1] ������ Ŭ����

		const float maxYOffset = 0.8f;  // �ִ� Y �̵� �Ÿ�
		const float maxZOffset = 9.f;   // �ִ� Z �̵� �Ÿ�

		_float3 tangent1, tangent2;
		m_pGameInstance->Get_ParallelVectorsInPlane(tangent1, tangent2, m_fFovy);

		// Y�� Z �������� ����
		targetPosition.y += tangent1.y * maxYOffset * t;
		targetPosition.z += tangent1.z * maxZOffset * t;
	}

	_vector targetVector = XMLoadFloat3(&targetPosition);
	// ��ġ ���� (����ũ ������ ����)
	_vector position = targetVector + m_vShakeOffset;

	// ī�޶� ��ġ ����
	m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSetW(position, 1.f));
}

void CVirtual_Camera::Set_Camera_Direction(_float averageX, _gvector pos1, _gvector pos2)
{
	// ī�޶��� Look ������ ������ ������ ���� (��: Z���� ���ϵ���)
	_vector fixedRight = XMVectorSet(1.f, 0.f, 0.0f, 0.f);
	_vector fixedUp = XMVectorSet(0.f, 1.f, 0.0f, 0.f);
	_vector fixedLook = XMVectorSet(0.0f, 0.0, 1.0f, 0.f);

	m_pTransformCom->Set_State(CTransform::STATE_RIGHT, fixedRight);
	m_pTransformCom->Set_State(CTransform::STATE_UP, fixedUp);
	m_pTransformCom->Set_State(CTransform::STATE_LOOK, fixedLook);
}

void CVirtual_Camera::Print_Flip_Rotation()
{
	// �Է� ���ʹϾ� �� ���� (������ ���Ͽ��� �������ų� �ϵ��ڵ�)
	float x = -0.162778f;
	float y = -0.584241f;
	float z = -0.121022f;
	float w = 0.785824f;

	// ���ʹϾ� �ε� �� ����ȭ
	XMVECTOR quat = XMVectorSet(x, y, z, w);
	quat = XMQuaternionNormalize(quat);

	// ���� ���ʹϾ� ���
	cout << "Original Quaternion: (" << x << ", " << y << ", " << z << ", " << w << ")" << endl;

	// �ݻ� ��ȯ�� �����ϴ� �Լ� ����
	auto ReflectQuaternion = [](XMVECTOR q, XMMATRIX reflectionMatrix) {
		// ���ʹϾ��� ȸ�� ��ķ� ��ȯ
		XMMATRIX rotMatrix = XMMatrixRotationQuaternion(q);
		// �ݻ�� ȸ�� ��� ���: M' = R * M * R
		XMMATRIX reflectedMatrix = reflectionMatrix * rotMatrix * reflectionMatrix;
		// �ݻ�� ȸ�� ����� ���ʹϾ����� ��ȯ
		XMVECTOR qReflected = XMQuaternionRotationMatrix(reflectedMatrix);
		// ����ȭ
		qReflected = XMQuaternionNormalize(qReflected);
		return qReflected;
		};

	// �� �࿡ ���� �ݻ� ��� ����
	XMMATRIX reflectX = XMMatrixScaling(-1.0f, 1.0f, 1.0f);
	XMMATRIX reflectY = XMMatrixScaling(1.0f, -1.0f, 1.0f);
	XMMATRIX reflectZ = XMMatrixScaling(1.0f, 1.0f, -1.0f);
	XMMATRIX reflectXY = XMMatrixScaling(-1.0f, -1.0f, 1.0f);
	XMMATRIX reflectXZ = XMMatrixScaling(-1.0f, 1.0f, -1.0f);
	XMMATRIX reflectYZ = XMMatrixScaling(1.0f, -1.0f, -1.0f);
	XMMATRIX reflectXYZ = XMMatrixScaling(-1.0f, -1.0f, -1.0f);

	// ����� ������ ����
	XMFLOAT4 reflectedQuaternion;

	// X�� ����
	XMVECTOR qReflectX = ReflectQuaternion(quat, reflectX);
	XMStoreFloat4(&reflectedQuaternion, qReflectX);
	cout << "Reflection over X-axis: (" << reflectedQuaternion.x << ", " << reflectedQuaternion.y << ", "
		<< reflectedQuaternion.z << ", " << reflectedQuaternion.w << ")" << endl;

	// Y�� ����
	XMVECTOR qReflectY = ReflectQuaternion(quat, reflectY);
	XMStoreFloat4(&reflectedQuaternion, qReflectY);
	cout << "Reflection over Y-axis: (" << reflectedQuaternion.x << ", " << reflectedQuaternion.y << ", "
		<< reflectedQuaternion.z << ", " << reflectedQuaternion.w << ")" << endl;

	// Z�� ����
	XMVECTOR qReflectZ = ReflectQuaternion(quat, reflectZ);
	XMStoreFloat4(&reflectedQuaternion, qReflectZ);
	cout << "Reflection over Z-axis: (" << reflectedQuaternion.x << ", " << reflectedQuaternion.y << ", "
		<< reflectedQuaternion.z << ", " << reflectedQuaternion.w << ")" << endl;

	// XY�� ����
	XMVECTOR qReflectXY = ReflectQuaternion(quat, reflectXY);
	XMStoreFloat4(&reflectedQuaternion, qReflectXY);
	cout << "Reflection over X and Y axes: (" << reflectedQuaternion.x << ", " << reflectedQuaternion.y << ", "
		<< reflectedQuaternion.z << ", " << reflectedQuaternion.w << ")" << endl;

	// XZ�� ����
	XMVECTOR qReflectXZ = ReflectQuaternion(quat, reflectXZ);
	XMStoreFloat4(&reflectedQuaternion, qReflectXZ);
	cout << "Reflection over X and Z axes: (" << reflectedQuaternion.x << ", " << reflectedQuaternion.y << ", "
		<< reflectedQuaternion.z << ", " << reflectedQuaternion.w << ")" << endl;

	// YZ�� ����
	XMVECTOR qReflectYZ = ReflectQuaternion(quat, reflectYZ);
	XMStoreFloat4(&reflectedQuaternion, qReflectYZ);
	cout << "Reflection over Y and Z axes: (" << reflectedQuaternion.x << ", " << reflectedQuaternion.y << ", "
		<< reflectedQuaternion.z << ", " << reflectedQuaternion.w << ")" << endl;

	// XYZ�� ����
	XMVECTOR qReflectXYZ = ReflectQuaternion(quat, reflectXYZ);
	XMStoreFloat4(&reflectedQuaternion, qReflectXYZ);
	cout << "Reflection over X, Y, and Z axes: (" << reflectedQuaternion.x << ", " << reflectedQuaternion.y << ", "
		<< reflectedQuaternion.z << ", " << reflectedQuaternion.w << ")" << endl;
}

void CVirtual_Camera::Set_Player(CGameObject* pPlayer, CGameObject* pEnemy)
{
	_uint uTeam = static_cast<CCharacter*>(pPlayer)->Get_iPlayerTeam();

	if (uTeam == 1)
		m_p1pPlayer = pPlayer;
	else if (uTeam == 2)
		m_p2pPlayer = pPlayer;

	for (auto& iter : m_mapPoints)
	{
		vector<CameraPoint> vecPoints = iter.second;
		for (auto& iter : vecPoints)
		{
			iter.pWorldFloat4x4 = static_cast<CTransform*>(pPlayer->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrixPtr();
		}
	}

	m_pEnemy = pEnemy;
}

void CVirtual_Camera::Start_Play(_int animationIndex, _bool isImguiPlay, CGameObject* gameObject, _bool ignoreFlip)
{
	if (m_mapPoints[animationIndex].size() == 0)
		return;

	for (auto& iter : m_mapPoints)
		for (auto& iter2 : iter.second)
			iter2.pWorldFloat4x4 = static_cast<CTransform*>(gameObject->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrixPtr();

	m_bIsIgnoreFlip = ignoreFlip;
	m_AnimationIndex = animationIndex;
	m_iTeam = static_cast<CCharacter*>(gameObject)->Get_iPlayerTeam();
	// Stopped ���¿��� Play�� �����ϸ� �ʱ�ȭ
	if (m_currentPlayMode == Stopped) {
		m_currentPointIndex = 0;
		m_elapsedTime = 0.f;
	}

	m_currentMode = CAMERA_CINEMATIC_MODE;
	m_currentPlayMode = Playing;

	m_bIsImguiPlay = isImguiPlay;
	//�÷��̸� �ϸ� �����
	Move_Point(0, m_AnimationIndex);
}

void CVirtual_Camera::Pause()
{
	if (m_currentPlayMode == CAMERA_PLAY_MODE::Playing)
		m_currentPlayMode = CAMERA_PLAY_MODE::Paused;
}

void CVirtual_Camera::Stop()
{
	m_currentMode = m_bIsImguiPlay ? CAMERA_FREE_MODE : CAMERA_NORMAL_MODE;

	m_currentPlayMode = CAMERA_PLAY_MODE::Stopped;
	m_currentPointIndex = 0;
	m_elapsedTime = 0.f;
	m_bIsIgnoreFlip = false;
	m_pEnemy = nullptr;
	m_bIsCirclePlay = false;
	m_circleElapsedTime = 0.0f;
	m_circleAngle = 0.0f;
	m_circleRadius = 0.0f;
}

// ���� ȸ�� ��� ���� �Լ�
void CVirtual_Camera::SetCirclePlay(_bool isClockwise, _float rotationSpeed)
{
	m_bIsCirclePlay = true;
	m_bIsClockwise = isClockwise;
	m_rotationSpeed = rotationSpeed;
	m_circleElapsedTime = 0.0f;
	m_circleAngle = 0.0f;
}

void CVirtual_Camera::Button_Stop()
{
	m_currentMode = CAMERA_FREE_MODE;
	m_currentPlayMode = CAMERA_PLAY_MODE::Stopped;
	m_currentPointIndex = 0;
	m_elapsedTime = 0.f;
	Move_Point(0, m_AnimationIndex);
}

void CVirtual_Camera::Free_Camera(_float fTimeDelta)
{
	//�⺻ �̵� �ӵ�
	_float fMoveSpeed = m_fMoveSpeed;

	// ������ ��ư�� ���ȴ��� Ȯ��
	if (m_pGameInstance->Mouse_Pressing(1))
	{
		// Shift Ű�� ���ȴ��� Ȯ���ϰ�, ���ȴٸ� �̵� �ӵ��� ����
		if (m_pGameInstance->Key_Pressing(DIK_LSHIFT))
		{
			fMoveSpeed *= 10.f;
		}

		if (m_pGameInstance->Key_Pressing(DIK_A))
		{
			m_pTransformCom->Go_Left(fTimeDelta * fMoveSpeed);
		}

		if (m_pGameInstance->Key_Pressing(DIK_D))
		{
			m_pTransformCom->Go_Right(fTimeDelta * fMoveSpeed);
		}

		if (m_pGameInstance->Key_Pressing(DIK_W))
		{
			m_pTransformCom->Go_Straight(fTimeDelta * fMoveSpeed);
		}

		if (m_pGameInstance->Key_Pressing(DIK_S))
		{
			m_pTransformCom->Go_Backward(fTimeDelta * fMoveSpeed);
		}

		if (m_pGameInstance->Key_Pressing(DIK_Q))
		{
			m_pTransformCom->Go_Down(fTimeDelta * fMoveSpeed);
		}

		if (m_pGameInstance->Key_Pressing(DIK_E))
		{
			m_pTransformCom->Go_Up(fTimeDelta * fMoveSpeed);
		}

		_long MouseMove = {};

		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM_X))
		{
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fMouseSensor * MouseMove * fTimeDelta);
		}

		if (MouseMove = m_pGameInstance->Get_DIMouseMove(DIMM_Y))
		{
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), m_fMouseSensor * MouseMove * fTimeDelta);
		}
	}

	POINT ptMouse{};
	GetCursorPos(&ptMouse);
	ScreenToClient(g_hWnd, &ptMouse);

	CImgui_Manager::IMGUI_SCREEN tDesc = CImgui_Manager::Get_Instance()->Get_Screen_Desc();

	// ���콺�� ImGui â ���� �ִ��� Ȯ��
	_bool isOverShaderImGui = (ptMouse.x >= tDesc.ShaderImGuiPos.x &&
		ptMouse.x <= tDesc.ShaderImGuiPos.x + tDesc.ShaderImGuiSize.x &&
		ptMouse.y >= tDesc.ShaderImGuiPos.y &&
		ptMouse.y <= tDesc.ShaderImGuiPos.y + tDesc.ShaderImGuiSize.y);

	_bool isOverMainImGui = (ptMouse.x >= tDesc.MainImGuiPos.x &&
		ptMouse.x <= tDesc.MainImGuiPos.x + tDesc.MainImGuiSize.x &&
		ptMouse.y >= tDesc.MainImGuiPos.y &&
		ptMouse.y <= tDesc.MainImGuiPos.y + tDesc.MainImGuiSize.y);

	if (isOverShaderImGui == true || isOverMainImGui == true)
	{
		__super::Camera_Update(fTimeDelta);

		return;
	}

	_long MouseMoveX = {};
	_long MouseMoveY = {};

	// ����Ʈ + ��� ���콺 ��ư�� ���� ���¿��� ī�޶� �̵�
	if (m_pGameInstance->Key_Pressing(DIK_LSHIFT) && (m_pGameInstance->Get_DIMouseState(DIMK_WHEEL) & 0x80))
	{
		MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM_X);
		MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM_Y);

		// �¿� ���콺 �����ӿ� ���� Go_Left�� Go_Right�� ī�޶� �̵�
		if (MouseMoveX < 0)
			m_pTransformCom->Go_Right(-MouseMoveX * m_fMouseSensor * fTimeDelta);
		else if (MouseMoveX > 0)
			m_pTransformCom->Go_Left(MouseMoveX * m_fMouseSensor * fTimeDelta);

		// ���� ���콺 �����ӿ� ���� Go_Up�� Go_Down�� ī�޶� �̵�
		if (MouseMoveY > 0)
			m_pTransformCom->Go_Up(MouseMoveY * m_fMouseSensor * fTimeDelta);
		else if (MouseMoveY < 0)
			m_pTransformCom->Go_Down(-MouseMoveY * m_fMouseSensor * fTimeDelta);
	}
	else if (m_pGameInstance->Get_DIMouseState(DIMK_WHEEL) & 0x80) // �ٸ� ������ �� ȸ��
	{
		if ((MouseMoveX = m_pGameInstance->Get_DIMouseMove(DIMM_X)))
		{
			// Y�� ȸ��
			m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), m_fMouseSensor * MouseMoveX * fTimeDelta);
		}

		if ((MouseMoveY = m_pGameInstance->Get_DIMouseMove(DIMM_Y)))
		{
			// ���� ȸ�� (������ ���͸� ������)
			m_pTransformCom->Turn(m_pTransformCom->Get_State(CTransform::STATE_RIGHT), m_fMouseSensor * MouseMoveY * fTimeDelta);
		}
	}

	// �� ��ũ�ѷ� ���� �ܾƿ�
	_long MouseWheel = m_pGameInstance->Get_DIMouseMove(DIMM_WHEEL);
	if (MouseWheel > 0)
	{
		// ���� ���� ���� ����
		m_pTransformCom->Go_Straight(m_fMouseSensor * MouseWheel * fTimeDelta);
	}
	else if (MouseWheel < 0)
	{
		// ���� �Ʒ��� ���� �ܾƿ�
		m_pTransformCom->Go_Backward(-m_fMouseSensor * MouseWheel * fTimeDelta);
	}
}

void CVirtual_Camera::Default_Camera(_float fTimeDelta)
{
	if (m_p1pPlayer == nullptr || m_p2pPlayer == nullptr)
		return;

	//// �÷��̾���� ��ġ�� �����ɴϴ�.
	_vector pos1 = static_cast<CTransform*>(m_p1pPlayer->Get_Component(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);
	_vector pos2 = static_cast<CTransform*>(m_p2pPlayer->Get_Component(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);

	//// �� �÷��̾� ���� X �Ÿ� ���
	_float distanceX = ComputeDistanceX(pos1, pos2);

	// ��� X ��ġ ���
	_float averageX = (XMVectorGetX(pos1) + XMVectorGetX(pos2)) * 0.5f;

	_float Player1P_Y = XMVectorGetY(pos1);
	_float Player2P_Y = XMVectorGetY(pos2);
	// �� �÷��̾��� Y �� �� (�� �� ���� ���� ����)
	_float higherY = max(Player1P_Y, Player2P_Y);

	// ī�޶��� ��ġ ����, �߰��� higherY�� ���
	Set_Camera_Position(averageX, distanceX, higherY, pos1, pos2);

	//// ī�޶��� ���� ���� ����
	Set_Camera_Direction(averageX, pos1, pos2);

}

void CVirtual_Camera::Map_Camera(_float fTimeDelta)
{
	if (m_isEastFinish == true)
	{
		if (m_bIsShaking == true)
		{
			_vector position = m_vBaseCameraPosition + m_vShakeOffset;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, position);
		}
	}
	else if (m_isDestructive == true)
	{
		if (m_isDyingTeam == 0)
			return;

		_vector vPlayerPos = { 0.f,0.f,0.f,1.f };

		switch (m_isDyingTeam)
		{
		case 1:
			vPlayerPos = static_cast<CTransform*>(m_p1pPlayer->Get_Component(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);
			break;
		case 2:
			vPlayerPos = static_cast<CTransform*>(m_p2pPlayer->Get_Component(TEXT("Com_Transform")))->Get_State(CTransform::STATE_POSITION);
			break;
		}

		vPlayerPos = XMVectorSetY(vPlayerPos, XMVectorGetY(vPlayerPos) + 1.f);

		m_pTransformCom->LookAt(vPlayerPos);
		if (m_bIsShaking == true)
		{
			_vector position = m_vBaseCameraPosition + m_vShakeOffset;
			m_pTransformCom->Set_State(CTransform::STATE_POSITION, position);
		}
	}
}


_float CVirtual_Camera::ComputeDistanceX(_gvector pos1, _gvector pos2)
{
	// �� �÷��̾� ���� X�� ���� ���
	_float deltaX = XMVectorGetX(XMVectorSubtract(pos2, pos1));
	return abs(deltaX);
}


void CVirtual_Camera::Add_Point(_float duration, _int type, const _float4x4* pModelFloat4x4, _float damping, _bool hasWorldFloat4x4, _int animationIndex)
{
	CameraPoint cameraPoint{};

	//�ش� ���� ������� ����

	//����Ʈ ī�޶�� ���� ������ ��� ��������� �����ϰ�
	//������ ī�޶�� ���� ������ ��ȯ�� ������� �����ؾ���
	if (hasWorldFloat4x4 == true)
	{
		_matrix matrix = Float4x4ToMatrix(*pModelFloat4x4);

		// �� ����� ����� ���
		_vector determinant = XMVectorZero();
		_matrix inverseModelMatrix = XMMatrixInverse(&determinant, matrix);

		// ���� ���� ī�޶��� ���� ��ġ ��������
		_vector worldPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		// ���� ��ġ�� ���� ���� ��ǥ�� ��ȯ
		_vector localPosition = XMVector3TransformCoord(worldPosition, inverseModelMatrix);

		// ��ȯ�� ���� ��ġ�� CameraPoint�� ����
		XMStoreFloat3(&cameraPoint.position, localPosition);

		// ī�޶��� ���� ȸ�� ��� ����
		_vector right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
		_vector up = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_UP));
		_vector look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

		_matrix cameraRotationMatrix = XMMatrixIdentity();
		cameraRotationMatrix.r[0] = right;
		cameraRotationMatrix.r[1] = up;
		cameraRotationMatrix.r[2] = look;
		cameraRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ����

		// �� ����Ŀ��� ȸ�� �κ� ����
		_matrix inverseModelRotationMatrix = inverseModelMatrix;
		inverseModelRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ����

		// ���� ȸ�� ��� ���
		_matrix localRotationMatrix = inverseModelRotationMatrix * cameraRotationMatrix;

		// ���� ȸ�� ����� ���ʹϾ����� ��ȯ�Ͽ� ����
		_vector localQuaternion = XMQuaternionRotationMatrix(localRotationMatrix);
		XMStoreFloat4(&cameraPoint.rotation, localQuaternion);

		// �߰� ���� ����
		cameraPoint.duration = duration;
		cameraPoint.interpolationType = type;
		cameraPoint.damping = damping;
		cameraPoint.pWorldFloat4x4 = pModelFloat4x4;
		cameraPoint.hasWorldFloat4x4 = hasWorldFloat4x4;

		// ī�޶� ����Ʈ�� �ʿ� �߰�
		m_mapPoints[animationIndex].push_back(cameraPoint);
	}
	else
	{
		//�׳� ������� ������ ��
		_vector worldPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		XMStoreFloat3(&cameraPoint.position, worldPosition);

		// **ī�޶��� ���� ȸ�� ��� ���� ����**
		_vector right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
		_vector up = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_UP));
		_vector look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

		_matrix cameraRotationMatrix = XMMatrixIdentity();
		cameraRotationMatrix.r[0] = right;
		cameraRotationMatrix.r[1] = up;
		cameraRotationMatrix.r[2] = look;
		cameraRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ���� ����

		_vector worldQuaternion = XMQuaternionRotationMatrix(cameraRotationMatrix);
		XMStoreFloat4(&cameraPoint.rotation, worldQuaternion);

		cameraPoint.duration = duration;
		cameraPoint.interpolationType = type;
		cameraPoint.damping = damping;
		cameraPoint.pWorldFloat4x4 = nullptr;
		cameraPoint.hasWorldFloat4x4 = hasWorldFloat4x4;
		m_mapPoints[animationIndex].push_back(cameraPoint);
	}
}

void CVirtual_Camera::Add_NormalPoint(_float duration, _int type, const _float4x4* pModelFloat4x4, _float damping, _bool hasWorldFloat4x4, _int animationIndex, CTransform* transform)
{
	CameraPoint cameraPoint{};

	//�ش� ���� ������� ����

	//����Ʈ ī�޶�� ���� ������ ��� ��������� �����ϰ�
	//������ ī�޶�� ���� ������ ��ȯ�� ������� �����ؾ���
	_matrix matrix = Float4x4ToMatrix(*pModelFloat4x4);

	// ���� ���� ����� ����� ���
	_vector determinant = XMVectorZero();
	_matrix inverseModelMatrix = XMMatrixInverse(&determinant, matrix);

	// ���� ����ī�޶��� ���� ������ ��������
	_vector worldPosition = transform->Get_State(CTransform::STATE_POSITION);

	// ���� �������� ���� ���� ��ǥ�� ��ȯ
	_vector localPosition = XMVector3TransformCoord(worldPosition, inverseModelMatrix);

	// ��ȯ�� ���� �������� CameraPoint ����ü�� ����
	XMStoreFloat3(&cameraPoint.position, localPosition);

	// **ī�޶��� ���� ȸ�� ��� ����**
	_vector right = XMVector3Normalize(transform->Get_State(CTransform::STATE_RIGHT));
	_vector up = XMVector3Normalize(transform->Get_State(CTransform::STATE_UP));
	_vector look = XMVector3Normalize(transform->Get_State(CTransform::STATE_LOOK));

	_matrix cameraRotationMatrix = XMMatrixIdentity();
	cameraRotationMatrix.r[0] = right;
	cameraRotationMatrix.r[1] = up;
	cameraRotationMatrix.r[2] = look;
	cameraRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ���� ����

	// **�� ����Ŀ��� ȸ�� �κ� ����**
	_matrix inverseModelRotationMatrix = inverseModelMatrix;
	inverseModelRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ���� ����

	// ���� ȸ�� ��� ��� (���� ����)
	_matrix localRotationMatrix = inverseModelRotationMatrix * cameraRotationMatrix;

	// **���� ȸ�� ����� ���ʹϾ����� ��ȯ�Ͽ� ����**
	_vector localQuaternion = XMQuaternionRotationMatrix(localRotationMatrix);
	XMStoreFloat4(&cameraPoint.rotation, localQuaternion);

	cameraPoint.duration = duration;
	cameraPoint.interpolationType = type;
	cameraPoint.damping = damping;
	cameraPoint.pWorldFloat4x4 = pModelFloat4x4;
	cameraPoint.hasWorldFloat4x4 = hasWorldFloat4x4;

	m_mapPoints[animationIndex].push_back(cameraPoint);
}


void CVirtual_Camera::Remove_Point(_int currentIndex, _int animationIndex)
{
	if (currentIndex < 0 || currentIndex >= static_cast<int>(m_mapPoints[animationIndex].size())) {
		return; // ��ȿ���� ���� �ε����� ��� �ƹ� �۾��� ���� ����
	}

	auto it = m_mapPoints[animationIndex].begin();
	advance(it, currentIndex);
	m_mapPoints[animationIndex].erase(it);
}

//list�� index ������ ã�ư��� ���� Position�� Rotation���� ī�޶��� Ʈ�������� �����ϸ� �ǰڴ�
void CVirtual_Camera::Move_Point(_int index, _int animationIndex)
{
	if (index < 0 || index >= m_mapPoints[animationIndex].size()) {
		return;
	}

	const CameraPoint& targetPoint = m_mapPoints[animationIndex][index];  // �ε��� ����

	if (targetPoint.hasWorldFloat4x4 == true)
	{
		// 1. ���� ������
		_float3 localPosition = targetPoint.position;

		// 2. ���� ȸ��
		_float4 localQuaternion = targetPoint.rotation;


		_matrix modelWorldMatrix = {};

		modelWorldMatrix =
			targetPoint.pWorldFloat4x4 == nullptr
			? Float4x4ToMatrix(*static_cast<CTransform*>(m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"))->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrixPtr())
			: Float4x4ToMatrix(*targetPoint.pWorldFloat4x4);

		// **�����ϸ� ���Ÿ� ���� ��� ����**
		_vector modelScale;
		_vector modelRotationQuat;
		_vector modelTranslation;
		XMMatrixDecompose(&modelScale, &modelRotationQuat, &modelTranslation, modelWorldMatrix);

		// **�����ϸ��� ���ŵ� ���� ���� ��� �籸��**
		_matrix modelRotationMatrix = XMMatrixRotationQuaternion(modelRotationQuat);
		_matrix modelTranslationMatrix = XMMatrixTranslationFromVector(modelTranslation);
		_matrix modelWorldMatrixNoScale = modelRotationMatrix * modelTranslationMatrix;

		// **4. ���� �������� ���� ���������� ��ȯ**
		_vector interpolatedPositionWorld = XMVector3TransformCoord(XMLoadFloat3(&localPosition), modelWorldMatrixNoScale);

		// **5. ���� ȸ���� ���� ȸ������ ��ȯ**
		// ���� ȸ�� ��� ����
		_matrix interpolatedRotationMatrixLocal = XMMatrixRotationQuaternion(XMLoadFloat4(&localQuaternion));

		// ���� ȸ�� ��� ���
		_matrix interpolatedRotationMatrixWorld = interpolatedRotationMatrixLocal * modelRotationMatrix;

		// **6. ī�޶��� ���� ��� ����**
		_matrix NewWorldMatrix = interpolatedRotationMatrixWorld;
		NewWorldMatrix.r[3] = XMVectorSetW(interpolatedPositionWorld, 1.0f); // ��ġ ����

		// ���� ��Ʈ�������� Right, Up, Look ���� ����
		_vector right = NewWorldMatrix.r[0];
		_vector up = NewWorldMatrix.r[1];
		_vector look = NewWorldMatrix.r[2];
		_vector position = NewWorldMatrix.r[3];

		// CTransform ������Ʈ�� ����
		CTransform* cameraTransform = static_cast<CTransform*>(Get_Component(TEXT("Com_Transform")));

		// ���� ���� ����
		cameraTransform->Set_State(CTransform::STATE_RIGHT, right);
		cameraTransform->Set_State(CTransform::STATE_UP, up);
		cameraTransform->Set_State(CTransform::STATE_LOOK, look);
		// ��ġ ����
		cameraTransform->Set_State(CTransform::STATE_POSITION, position);
	}
	else
	{
		// CTransform ������Ʈ�� ����
		CTransform* cameraTransform = static_cast<CTransform*>(Get_Component(TEXT("Com_Transform")));

		// ���� ������ �ε�
		_float3 worldPosition = targetPoint.position;

		// ���� ȸ�� �ε�
		_float4 worldQuaternion = targetPoint.rotation;

		// ���� ȸ�� ��� ����
		_matrix interpolatedRotationMatrixWorld = XMMatrixRotationQuaternion(XMLoadFloat4(&worldQuaternion));

		// ī�޶��� ���� ��� ����
		_matrix NewWorldMatrix = interpolatedRotationMatrixWorld;
		NewWorldMatrix.r[3] = XMVectorSetW(XMLoadFloat3(&worldPosition), 1.f); // ��ġ ����

		// ���� ��Ʈ�������� Right, Up, Look ���� ����
		_vector right = NewWorldMatrix.r[0];
		_vector up = NewWorldMatrix.r[1];
		_vector look = NewWorldMatrix.r[2];
		_vector position = NewWorldMatrix.r[3];

		// ���� ���� ����
		cameraTransform->Set_State(CTransform::STATE_RIGHT, right);
		cameraTransform->Set_State(CTransform::STATE_UP, up);
		cameraTransform->Set_State(CTransform::STATE_LOOK, look);
		// ��ġ ����
		cameraTransform->Set_State(CTransform::STATE_POSITION, position);
	}
}

void CVirtual_Camera::Modify_Transform(_int index, _int animationIndex)
{
	CameraPoint& targetPoint = m_mapPoints[animationIndex][index];  // �ε��� ����

	//���� �����Ϸ��� �ϴ� �ε����� Point�� �����ؼ�
	//���� ����ī�޶��� �����ǰ� Rotation�� ���÷� ������ �����ؾ���
	if (targetPoint.pWorldFloat4x4 != nullptr)
	{
		_matrix matrix = Float4x4ToMatrix(*targetPoint.pWorldFloat4x4);

		// ���� ���� ����� ����� ���
		_vector determinant = XMVectorZero();
		_matrix inverseModelMatrix = XMMatrixInverse(&determinant, matrix);

		// ���� ����ī�޶��� ���� ������ ��������
		_vector worldPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

		// ���� �������� ���� ���� ��ǥ�� ��ȯ
		_vector localPosition = XMVector3TransformCoord(worldPosition, inverseModelMatrix);

		// ��ȯ�� ���� �������� CameraPoint ����ü�� ����
		XMStoreFloat3(&targetPoint.position, localPosition);

		// **ī�޶��� ���� ȸ�� ��� ����**
		_vector right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
		_vector up = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_UP));
		_vector look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

		_matrix cameraRotationMatrix = XMMatrixIdentity();
		cameraRotationMatrix.r[0] = right;
		cameraRotationMatrix.r[1] = up;
		cameraRotationMatrix.r[2] = look;
		cameraRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ���� ����

		// **�� ����Ŀ��� ȸ�� �κ� ����**
		_matrix inverseModelRotationMatrix = inverseModelMatrix;
		inverseModelRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ���� ����

		// **ī�޶��� ���� ȸ�� ��� ���**
		_matrix localRotationMatrix = cameraRotationMatrix * inverseModelRotationMatrix;

		// **���� ȸ�� ����� ���ʹϾ����� ��ȯ�Ͽ� ����**
		_vector localQuaternion = XMQuaternionRotationMatrix(localRotationMatrix);
		_float4 rotation = {};
		XMStoreFloat4(&rotation, localQuaternion);
		targetPoint.rotation = rotation;
	}
	//����Ʈ�� ���� �����ǰ� �����̼Ǹ�
	else
	{
		// ���� ����ī�޶��� ���� ������ ������ �� ����
		_vector worldPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
		XMStoreFloat3(&targetPoint.position, worldPosition);

		// **ī�޶��� ���� ȸ�� ��� ����**
		_vector right = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_RIGHT));
		_vector up = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_UP));
		_vector look = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

		_matrix cameraRotationMatrix = XMMatrixIdentity();
		cameraRotationMatrix.r[0] = right;
		cameraRotationMatrix.r[1] = up;
		cameraRotationMatrix.r[2] = look;
		cameraRotationMatrix.r[3] = XMVectorSet(0, 0, 0, 1); // ��ġ ���� ����

		_vector worldQuaternion = XMQuaternionRotationMatrix(cameraRotationMatrix);
		XMStoreFloat4(&targetPoint.rotation, worldQuaternion);
	}
}

_float CVirtual_Camera::AdjustT_Damping(_float t, _float damping)
{
	// Damping ����� ���� t ���� ����
	 // damping > 1.0f: �� ������ �����ϰ� ������ ���� (ease-in)
	 // damping < 1.0f: �� ������ �����ϰ� ������ ���� (ease-out)
	 // damping = 1.0f: �⺻ Smoothstep �Լ��� ����
	if (damping == 1.0f)
	{
		return t * t * (3.0f - 2.0f * t); // �⺻ Smoothstep
	}
	else if (damping > 1.0f)
	{
		// Smoothstep�� pow(t, damping)�� ȥ��
		float smooth = t * t * (3.0f - 2.0f * t); // Smoothstep
		float easeIn = pow(t, damping); // Ease-In
		float weight = 0.1f; // Smoothstep�� ����ġ ���� (0.0f ~ 1.0f)
		return weight * smooth + (1.0f - weight) * easeIn;
	}
	else // damping < 1.0f
	{
		// Ease-out ȿ�� ��ȭ
		return t * t * (3.0f - 2.0f * t) + (1.0f - t) * t * (1.0f - damping);
	}
}

void CVirtual_Camera::Delete_Points(_int animationIndex)
{
	m_mapPoints[animationIndex].clear();
}

void CVirtual_Camera::ApplyCameraShake(_float fTimeDelta)
{
	m_fElapsedShakeTime += fTimeDelta;

	if (m_fElapsedShakeTime >= m_fShakeDuration)
	{
		// ��鸲 ����
		StopCameraShake();
		return;
	}

	// ��鸲 ������ ���
	_float progress = m_fElapsedShakeTime / m_fShakeDuration;
	_float damper = 1.0f - progress; // ��鸲 ���� ����

	// ���� ������ ���
	_float offsetX = ((rand() % 1000) / 500.0f - 1.0f) * m_fShakeMagnitude * damper;
	_float offsetY = ((rand() % 1000) / 500.0f - 1.0f) * m_fShakeMagnitude * damper;
	_float offsetZ = ((rand() % 1000) / 500.0f - 1.0f) * m_fShakeMagnitude * damper;

	// ��鸲 ������ ���� ����
	m_vShakeOffset = XMVectorSet(offsetX, offsetY, offsetZ, 0.0f);

	// **����� ���� ��ġ�� ��鸲 ������ ����**
	_vector shakenPosition = m_vBaseCameraPosition + m_vShakeOffset;
}

void CVirtual_Camera::StartCameraShake(_float fDuration, _float fMagnitude)
{
	m_vBaseCameraPosition = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	m_bIsShaking = true;
	m_fShakeDuration = fDuration;
	m_fShakeMagnitude = fMagnitude;
	m_fElapsedShakeTime = 0.0f;
}

void CVirtual_Camera::StopCameraShake()
{
	m_bIsShaking = false;
	m_fShakeDuration = 0.0f;
	m_fElapsedShakeTime = 0.0f;
	m_vShakeOffset = XMVectorZero();
}

void CVirtual_Camera::Set_CameraMode(CMain_Camera::VIRTUAL_CAMERA cameraMode)
{
	if (cameraMode == CMain_Camera::VIRTUAL_CAMERA_NORMAL)
		m_currentMode = CAMERA_NORMAL_MODE;
	else if (cameraMode == CMain_Camera::VIRTUAL_CAMERA_FREE)
		m_currentMode = CAMERA_FREE_MODE;
	else if (cameraMode == CMain_Camera::VIRTUAL_CAMERA_MAP)
		m_currentMode = CAMERA_MAP_MODE;
	else
		m_currentMode = CAMERA_FREE_MODE;
}


void CVirtual_Camera::Set_DyingTeam(_uint iTeamIndex, _matrix CamWorldMatrix)
{
	m_isDyingTeam = iTeamIndex;
	_float4x4 ResultMatrix;
	XMStoreFloat4x4(&ResultMatrix, CamWorldMatrix);
	m_pTransformCom->Set_WorldMatrix(ResultMatrix);
	m_isDestructive = true;
	m_isEastFinish = false;
}

void CVirtual_Camera::Set_EastFinish()
{
	m_isEastFinish = true;
	m_isDestructive = false;
}

CVirtual_Camera* CVirtual_Camera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CVirtual_Camera* pInstance = new CVirtual_Camera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CVirtual_Camera"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CVirtual_Camera::Clone(void* pArg)
{
	CVirtual_Camera* pInstance = new CVirtual_Camera(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CVirtual_Camera"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CVirtual_Camera::Free()
{
	__super::Free();

}

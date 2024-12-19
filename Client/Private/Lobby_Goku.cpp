#include "stdafx.h"
#include "..\Public\Lobby_Goku.h"

#include "RenderInstance.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "Level_Lobby.h"

#include "Lobby_Frieza.h"
#include "UI_Lobby_Text.h"

#include "BattleInterface.h"
#include "UI_Define.h"
#include "UI_Manager.h"

CLobby_Goku::CLobby_Goku(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CLobby_Goku::CLobby_Goku(const CLobby_Goku& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CLobby_Goku::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLobby_Goku::Initialize(void* pArg)
{
	CTransform::TRANSFORM_DESC Desc{};
	Desc.fRotationPerSec = 1.f;
	Desc.fSpeedPerSec = 1.f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_Scaled(1.f, 1.f, 1.f);
	m_pTransformCom->Set_State_Position(_float3(0.f, 0.f, -32.f));
	//���̵�
	m_pModelCom->SetUp_Animation(1, true, 0.1f);

	return S_OK;
}

void CLobby_Goku::Camera_Update(_float fTimeDelta)
{

}

void CLobby_Goku::Update(_float fTimeDelta)
{
	// �Է� ó��
	_float3 vTargetDir = { 0.f, 0.f, 0.f }; // ��ǥ ����
	_bool bInput = false;

	// ���� Ű ���� ����
	bool bCurrentKeyUp = m_pGameInstance->Key_Pressing(DIK_UP);
	bool bCurrentKeyDown = m_pGameInstance->Key_Pressing(DIK_DOWN);
	bool bCurrentKeyLeft = m_pGameInstance->Key_Pressing(DIK_LEFT);
	bool bCurrentKeyRight = m_pGameInstance->Key_Pressing(DIK_RIGHT);

	// Ű �Է¿� ���� ��ǥ ���� ����
	if (bCurrentKeyUp)
	{
		vTargetDir.z += 1.f;
		bInput = true;
	}
	if (bCurrentKeyDown)
	{
		vTargetDir.z -= 1.f;
		bInput = true;
	}
	if (bCurrentKeyLeft)
	{
		vTargetDir.x -= 1.f;
		bInput = true;
	}
	if (bCurrentKeyRight)
	{
		vTargetDir.x += 1.f;
		bInput = true;
	}

	if (bInput)
	{
		m_pModelCom->SetUp_Animation(0, true, 0.1f);

		// ȸ�� ó��
		RotateTowardsTarget(vTargetDir, fTimeDelta);

		// �̵� ó��
		MoveForward(fTimeDelta);

		// �߼Ҹ� Ÿ�̸� ������Ʈ
		m_fFootstepTimer += fTimeDelta;
		if (m_fFootstepTimer >= m_fFootstepInterval)
		{
			m_pGameInstance->Play_Sound(CSound_Manager::SOUND_KEY_NAME::LOBBY_FOOT_SFX, false, 0.5f);
			m_fFootstepTimer = 0.f; // Ÿ�̸� ����
		}
	}
	else
	{
		// �ִϸ��̼� ����: �Է��� ���� �� �⺻ �ִϸ��̼� ��� (�ε��� 1)
		m_pModelCom->SetUp_Animation(1, true, 0.1f);

		// �뽬 �ִϸ��̼��� �������� ǥ��
		m_bDashTriggered = false;

		// Ÿ�̸� ����
		m_fFootstepTimer = 0.f;
	}

	//�ɾ��� �� ����Ʈ ����
	CreateRunDustEffect(bInput ,0.25f ,fTimeDelta);

	// ���� �������� �ִϸ��̼� ���
	m_pModelCom->Play_Animation(fTimeDelta * 3.f);

	// ���� Ű ���� ������Ʈ
	m_bPrevKeyUp = bCurrentKeyUp;
	m_bPrevKeyDown = bCurrentKeyDown;
	m_bPrevKeyLeft = bCurrentKeyLeft;
	m_bPrevKeyRight = bCurrentKeyRight;

	// ���� �̵�
	
	 
	Entry_Level();

	Talk_Frieza(5.f);
}

void CLobby_Goku::Late_Update(_float fTimeDelta)
{
	m_pRenderInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CLobby_Goku::Render(_float fTimeDelta)
{ 
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		/* ���� ������ �ִ� ���׸��� �� i��° �޽ð� ����ؾ��ϴ� ���׸�����ü�� aiTextureType_DIFFUSE��° �ؽ��ĸ� */
		/* m_pShaderCom�� �ִ� g_DiffuseTexture������ ����. */
		if (FAILED(m_pModelCom->Bind_MaterialSRV(m_pShaderCom, aiTextureType_DIFFUSE, "g_DiffuseTexture", i)))
			return E_FAIL;

		//m_pModelCom->Bind_MaterialSRV(m_pShaderCom, aiTextureType_NORMALS, "g_NormalTexture", i);

		/* ���� ������ �ִ� ���� �߿��� ���� �������ҷ��� �ߴ� i��°�� �޽ð� ����ϴ� ������ �迭�� ���� ���̴��� �����ش�.  */
		if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pShaderCom, "g_BoneMatrices", i)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Begin(3)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLobby_Goku::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Model_Lobby_Goku"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLobby_Goku::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;


	return S_OK;
}

inline float Lobby_Goku_Clamp(float value, float minVal, float maxVal)
{
	if (value < minVal)
		return minVal;
	if (value > maxVal)
		return maxVal;
	return value;
}

void CLobby_Goku::RotateTowardsTarget(const _float3& vTargetDir, _float fTimeDelta)
{
	// ȸ�� �ӵ� (����/��)
	const _float ROTATION_SPEED = XM_PI * 5.f; // 180��/��

	// ��ǥ ���� ���͸� �ε��ϰ� y�� 0���� �����Ͽ� xz ��鿡 ����
	XMVECTOR vTargetLook = XMLoadFloat3(&vTargetDir);
	vTargetLook = XMVectorSetY(vTargetLook, 0.f);
	vTargetLook = XMVector3Normalize(vTargetLook);

	// ���� Look ���͸� �����ͼ� y�� 0���� �����Ͽ� xz ��鿡 ����
	XMVECTOR vCurrentLook = m_pTransformCom->Get_State(CTransform::STATE_LOOK);
	vCurrentLook = XMVectorSetY(vCurrentLook, 0.f);
	vCurrentLook = XMVector3Normalize(vCurrentLook);

	// �� ���� ������ ���� ���
	float fDot = XMVectorGetX(XMVector3Dot(vCurrentLook, vTargetLook));
	fDot = Lobby_Goku_Clamp(fDot, -1.0f, 1.0f); // acos�� ���ǿ��� ����� �ʵ��� Ŭ����
	float fAngle = acosf(fDot);

	// ȸ�� ���� ���� (������ y ���� ���)
	XMVECTOR vCross = XMVector3Cross(vCurrentLook, vTargetLook);
	float fSign = (XMVectorGetY(vCross) < 0.0f) ? -1.0f : 1.0f;

	// ȸ�� ���� ����
	float fRotationAngle = min(fAngle, ROTATION_SPEED * fTimeDelta);

	// ȸ�� ������ ȸ�� ���� ����
	float fDeltaAngle = fRotationAngle * fSign;

	// y�� ȸ�� ��� ����
	XMMATRIX mRotation = XMMatrixRotationY(fDeltaAngle);

	// ���� ���� ��� ��������
	XMMATRIX mWorld = m_pTransformCom->Get_WorldMatrix();

	// ȸ�� ����� ���� ���� ��Ŀ� ����
	mWorld = mRotation * mWorld;

	_float4x4 matrix{};
	XMStoreFloat4x4(&matrix, mWorld);

	// ���ο� ���� ��� ����
	m_pTransformCom->Set_WorldMatrix(matrix);
}

void CLobby_Goku::MoveForward(_float fTimeDelta)
{
	// ��� ����
	const _float MOVE_SPEED = 15.f; // �̵� �ӵ� (����/��)

	// ���� Look ���͸� �����ͼ� ����ȭ
	_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(CTransform::STATE_LOOK));

	// �̵� �Ÿ� ��� (�ӵ� * �ð�)
	_float3 vMoveDistance;
	XMStoreFloat3(&vMoveDistance, vLook * fTimeDelta * MOVE_SPEED);

	// ���� ��ġ ��������
	_float3 vCurrentPos{};
	XMStoreFloat3(&vCurrentPos, m_pTransformCom->Get_State(CTransform::STATE_POSITION));

	// ���ο� ��ġ ���
	_float3 vNewPos = _float3(vCurrentPos.x + vMoveDistance.x, vCurrentPos.y + vMoveDistance.y, vCurrentPos.z + vMoveDistance.z);

	// ���ο� ��ġ ����
	m_pTransformCom->Set_State_Position(vNewPos);
}

void CLobby_Goku::Entry_Level()
{
	//_vector position = m_pTransformCom->Get_State(CTransform::STATE_POSITION);
	//_float x = XMVectorGetX(position);
	//_float z = XMVectorGetZ(position);

	_bool bTalkToGamePlay = dynamic_cast<CUI_Lobby_Text*>(m_pGameInstance->Get_GameObject(LEVEL_LOBBY, TEXT("Layer_Lobby_TextBox")))->Get_Finish();
	_bool bBuildingToGamePlay =  (10.f >= ObjectDistance(TEXT("Layer_Lobby_Arcade_Building"), 0));

	if (bTalkToGamePlay)
	{
		Set_CharacterInfo();
		m_bOnMessageBox = FALSE;
		// m_eLevelID 
		CUI_Manager::Get_Instance()->m_eLevelID = LEVEL_GAMEPLAY;
		CLevel_Lobby* level_Lobby = static_cast<CLevel_Lobby*>( m_pGameInstance->Get_Level());
		level_Lobby->Change_Level();
	}

	if (bBuildingToGamePlay)
	{
		CUI_Manager::Get_Instance()->m_eLevelID = LEVEL_CHARACTER;
		CLevel_Lobby* level_Lobby = static_cast<CLevel_Lobby*>(m_pGameInstance->Get_Level());
		level_Lobby->Change_Level();
	}
}

void CLobby_Goku::Set_CharacterInfo()
{
	CBattleInterface_Manager::Get_Instance()->Set_CharaDesc(0, 1, CUI_Define::LPLAYER1, TEXT("Prototype_GameObject_Play_Goku"), CUI_Define::GOKU);
	CBattleInterface_Manager::Get_Instance()->Set_CharaDesc(1, 2, CUI_Define::RPLAYER1, TEXT("Prototype_GameObject_Play_Frieza"), CUI_Define::FRIEZA);
	CBattleInterface_Manager::Get_Instance()->Set_b1VS1(true);
}

_float CLobby_Goku::ObjectDistance(_wstring strLayerTag, _uint iLayerIndex)
{
	CTransform* pObjectTransform = dynamic_cast<CTransform*>(m_pGameInstance->Get_Component(LEVEL_LOBBY, strLayerTag, TEXT("Com_Transform"), iLayerIndex));
	_vector vObjectPos = pObjectTransform->Get_State(CTransform::STATE_POSITION);
	_vector vMyPos = m_pTransformCom->Get_State(CTransform::STATE_POSITION);

	return  GetVectorLength(vObjectPos - vMyPos);;
}

void CLobby_Goku::Talk_Frieza(_float fEnableDistance)
{
	_float fDistance = ObjectDistance(TEXT("Layer_Lobby_Frieza"));
	
	if (fEnableDistance >= fDistance)
	{
		m_pGameInstance->Get_GameObject(LEVEL_LOBBY, TEXT("Layer_Lobby_Key_Enter"))->SetActive(!m_bOnMessageBox);

		if (m_pGameInstance->Key_Down(DIK_RETURN))
		{
			m_pGameInstance->Get_GameObject(LEVEL_LOBBY, TEXT("Layer_Lobby_TextBox"))->SetActive(TRUE);
			m_bOnMessageBox = TRUE;
		}
	}
	else 
		m_pGameInstance->Get_GameObject(LEVEL_LOBBY, TEXT("Layer_Lobby_Key_Enter"))->SetActive(FALSE);
}

void CLobby_Goku::CreateRunDustEffect(_bool bOnInput ,_float fCreateDuration , _float fTimeDelta)
{
	CreateDustTimer += fTimeDelta;
	if (bOnInput && CreateDustTimer >= fCreateDuration)
	{
		m_pGameInstance->Add_GameObject_ToLayer(LEVEL_LOBBY, TEXT("Prototype_GameObject_Lobby_Goku_RunEff"), TEXT("Layer_Lobby_Goku_RunEff"));
		CreateDustTimer = 0.f;
	}
}

CLobby_Goku* CLobby_Goku::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLobby_Goku* pInstance = new CLobby_Goku(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CLobby_Goku"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLobby_Goku::Clone(void* pArg)
{
	CLobby_Goku* pInstance = new CLobby_Goku(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CLobby_Goku"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLobby_Goku::Free()
{
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	__super::Free();
}

#include "stdafx.h"
#include "..\Public\Loading_Camera.h"

#include "GameInstance.h"

CLoading_Camera::CLoading_Camera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{

}

CLoading_Camera::CLoading_Camera(const CLoading_Camera& Prototype)
	: CCamera{ Prototype }
{

}

HRESULT CLoading_Camera::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CLoading_Camera::Initialize(void* pArg)
{
	CAMERA_DESC Desc{};

	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(90.0f);

	m_fMouseSensor = 0.1f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	m_pTransformCom->Set_State_Position(_float3(0.f, -0.5f, 0.0f));
	m_pTransformCom->LookAt(XMVectorSet(0.f, 1.f, 1.f,1.f));

	//m_pLobby_Goku_Transform = static_cast<CTransform*>(m_pGameInstance->Get_GameObject(LEVEL_LOBBY, TEXT("Layer_Lobby_Goku"))->Get_Component(TEXT("Com_Transform")));

	return S_OK;
}

void CLoading_Camera::Camera_Update(_float fTimeDelta)
{
	//_vector playerPosition = m_pLobby_Goku_Transform->Get_State(CTransform::STATE_POSITION);
	//_vector offset = XMVectorSet(0.f, 15.f, -15.f, 0.f);
	//
	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, playerPosition + offset);
	//m_pTransformCom->LookAt(playerPosition + XMVectorSet(0.f, 0.f, 10.f, 0.f));

	//�⺻ �̵� �ӵ�
	_float fMoveSpeed = 1.f;
	
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

	__super::Camera_Update(fTimeDelta);
}

void CLoading_Camera::Update(_float fTimeDelta)
{
}

void CLoading_Camera::Late_Update(_float fTimeDelta)
{
}

HRESULT CLoading_Camera::Render(_float fTimeDelta)
{
	return S_OK;
}

CLoading_Camera* CLoading_Camera::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLoading_Camera* pInstance = new CLoading_Camera(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CLoading_Camera"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLoading_Camera::Clone(void* pArg)
{
	CLoading_Camera* pInstance = new CLoading_Camera(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CLoading_Camera"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLoading_Camera::Free()
{
	__super::Free();

}

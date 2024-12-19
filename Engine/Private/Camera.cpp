#include "..\Public\Camera.h"
#include "GameInstance.h"
CCamera::CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CCamera::CCamera(const CCamera& Prototype)
	: CGameObject{ Prototype }
	, m_fViewportWidth{ Prototype.m_fViewportWidth }
	, m_fViewportHeight{ Prototype.m_fViewportHeight }
	, m_fFovy{ Prototype.m_fFovy }
	, m_fNear{ Prototype.m_fNear }
	, m_fFar{ Prototype.m_fFar }
{
}

HRESULT CCamera::Initialize_Prototype()
{
	m_fViewportWidth = 1920;
	m_fViewportHeight = 1080;

	return S_OK;
}

HRESULT CCamera::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		////�þ߰�
		//m_fFovy = XMConvertToRadians(40.0f);
		////Near
		//m_fNear = 0.1f;
		////Far
		//m_fFar = 10000.f;

		//����ī�޶�
		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}
	else
	{
		CAMERA_DESC* pDesc = static_cast<CAMERA_DESC*>(pArg);

		m_fFovy = pDesc->fFovy;
		m_fNear = pDesc->fNear;
		m_fFar = pDesc->fFar;

		if (FAILED(__super::Initialize(pArg)))
			return E_FAIL;
	}

	return S_OK;
}

//Main_Camera���� ȣ��Ǵ� �� �Լ�
void CCamera::Camera_Update(_float fTimeDelta)
{
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());

	//����ī�޶� ó�� ������Ʈ Ż�� Ÿ�� �����
	if (m_fFovy != 0.f)
		m_pGameInstance->Set_Transform(CPipeLine::D3DTS_PROJ, XMMatrixPerspectiveFovLH(m_fFovy, m_fViewportWidth / m_fViewportHeight, m_fNear, m_fFar));
}

void CCamera::Update(_float fTimeDelta)
{

}

void CCamera::Late_Update(_float fTimeDelta)
{
}

HRESULT CCamera::Render(_float fTimeDelta)
{
	return S_OK;
}

//��� ����ī�޶��.
void CCamera::Update_Camera(CCamera* camera, _float fTimeDelta)
{
	
	//���⼭�� ����ī�޶��� ������ ���� �Ѵ�.
	_matrix Inverse_Matrix = static_cast<CTransform*>(camera->Get_Component(TEXT("Com_Transform")))->Get_WorldMatrix_Inverse();
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_VIEW, Inverse_Matrix);
	m_pGameInstance->Set_Transform(CPipeLine::D3DTS_PROJ, XMMatrixPerspectiveFovLH(camera->m_fFovy, camera->m_fViewportWidth / camera->m_fViewportHeight, camera->m_fNear, camera->m_fFar));
}

void CCamera::Free()
{
	__super::Free();
}

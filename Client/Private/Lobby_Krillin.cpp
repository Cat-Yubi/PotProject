#include "stdafx.h"
#include "..\Public\Lobby_Krillin.h"

#include "RenderInstance.h"
#include "GameInstance.h"
#include "Level_Loading.h"
#include "Level_Lobby.h"

CLobby_Krillin::CLobby_Krillin(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CLobby_Krillin::CLobby_Krillin(const CLobby_Krillin& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CLobby_Krillin::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLobby_Krillin::Initialize(void* pArg)
{
	CTransform::TRANSFORM_DESC Desc{};
	Desc.fRotationPerSec = 1.f;
	Desc.fSpeedPerSec = 10.f;

	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State_Position(_float3(-56.6f, 0.f, -1.8f));
	m_pTransformCom->Rotation({ 0.f , 1.f, 0.f }, XMConvertToRadians(86.f));

	//���̵�
	m_pModelCom->SetUp_Animation(0, true, 0.1f);

	return S_OK;
}

void CLobby_Krillin::Camera_Update(_float fTimeDelta)
{

}

void CLobby_Krillin::Update(_float fTimeDelta)
{
	m_pModelCom->Play_Animation(fTimeDelta);

}

void CLobby_Krillin::Late_Update(_float fTimeDelta)
{
	m_pRenderInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CLobby_Krillin::Render(_float fTimeDelta)
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

HRESULT CLobby_Krillin::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Model_Lobby_Krillin"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLobby_Krillin::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

CLobby_Krillin* CLobby_Krillin::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLobby_Krillin* pInstance = new CLobby_Krillin(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CLobby_Krillin"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLobby_Krillin::Clone(void* pArg)
{
	CLobby_Krillin* pInstance = new CLobby_Krillin(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CLobby_Krillin"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLobby_Krillin::Free()
{
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	__super::Free();
}

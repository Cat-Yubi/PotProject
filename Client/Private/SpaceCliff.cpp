#include "stdafx.h"
#include "..\Public\SpaceCliff.h"

#include "RenderInstance.h"
#include "GameInstance.h"

CSpaceCliff::CSpaceCliff(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
	: CGameObject { pDevice, pContext }
{

}

CSpaceCliff::CSpaceCliff(const CSpaceCliff & Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CSpaceCliff::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSpaceCliff::Initialize(void * pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	//m_pTransformCom->Set_Scaled(0.01f, 0.01f, 0.01f);
	//m_pTransformCom->Rotation(XMVectorSet(1.f, 0.f, 0.f, 0.f), XMConvertToRadians(180.f));
	
	CMap_Manager::Get_Instance()->Push_MapObject(CMap_Manager::MAP_SPACE,
		static_cast<CMap_Manager::Map_Object_Key*>(pArg)->m_PrototypeKey, this);
	return S_OK;
}

void CSpaceCliff::Camera_Update(_float fTimeDelta)
{
	//m_fAccTime += fTimeDelta * 5;
}

void CSpaceCliff::Update(_float fTimeDelta)
{

}

void CSpaceCliff::Late_Update(_float fTimeDelta)
{
	m_pRenderInstance->Add_RenderObject(CRenderer::RG_SPACEMAP, this);
}

HRESULT CSpaceCliff::Render(_float fTimeDelta)
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		
		if (FAILED(m_pShaderCom->Begin(4)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CSpaceCliff::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_VtxSpace"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Space_Cliff"),
		TEXT("Com_Texture_Diffuse"), reinterpret_cast<CComponent**>(&m_pTextureCom_Diffuse))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_SpaceCliff"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CSpaceCliff::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pTextureCom_Diffuse->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
		return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_RawValue("g_fSpriteSize", &m_fSpriteSize, sizeof(_float2))))
	//	return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_RawValue("g_fSpriteCurPos", &m_fSpriteCurPos, sizeof(_float2))))
	//	return E_FAIL;

	//if (FAILED(m_pShaderCom->Bind_RawValue("g_Time", &m_fAccTime, sizeof(float))))
	//	return E_FAIL;
	
	return S_OK;
}

CSpaceCliff * CSpaceCliff::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CSpaceCliff*		pInstance = new CSpaceCliff(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CSpaceCliff"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject * CSpaceCliff::Clone(void * pArg)
{
	CSpaceCliff*		pInstance = new CSpaceCliff(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CSpaceCliff"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSpaceCliff::Free()
{

	Safe_Release(m_pTextureCom_Diffuse);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	__super::Free();
}
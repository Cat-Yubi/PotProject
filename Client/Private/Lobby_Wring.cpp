#include "stdafx.h"
#include "..\Public\Lobby_Wring.h"

#include "RenderInstance.h"
#include "GameInstance.h"

CLobby_Wring::CLobby_Wring(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CLobby_Wring::CLobby_Wring(const CLobby_Wring& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CLobby_Wring::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CLobby_Wring::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_pTransformCom->Set_State_Position(_float3(0.072f, 0.018f, 0.832f));
	m_pTransformCom->Rotation({ 0.f, 1.f, 0.f }, XMConvertToRadians(180.f));
	m_pTransformCom->Set_Scaled(2.5f, 2.5f, 2.5f);

	return S_OK;
}

void CLobby_Wring::Camera_Update(_float fTimeDelta)
{

}

void CLobby_Wring::Update(_float fTimeDelta)
{
}

void CLobby_Wring::Late_Update(_float fTimeDelta)
{
	m_pRenderInstance->Add_RenderObject(CRenderer::RG_NONBLEND, this);
}

HRESULT CLobby_Wring::Render(_float fTimeDelta)
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		if (FAILED(m_pDiffTexture[i]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
			return E_FAIL;
		/* ���� ������ �ִ� ���׸��� �� i��° �޽ð� ����ؾ��ϴ� ���׸�����ü�� aiTextureType_DIFFUSE��° �ؽ��ĸ� */
		/* m_pShaderCom�� �ִ� g_DiffuseTexture������ ����. */
		if (FAILED(m_pModelCom->Bind_MaterialSRV(m_pShaderCom, aiTextureType_DIFFUSE, "g_DiffuseTexture", i)))
			return E_FAIL;
		// m_pModelCom->Bind_MaterialSRV(m_pShaderCom, aiTextureType_NORMALS, "g_NormalTexture", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CLobby_Wring::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Model_Lobby_Wring"),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* Com_DiffTexture0 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_RingSide03"),
		TEXT("Com_DiffTexture0"), reinterpret_cast<CComponent**>(&m_pDiffTexture[0]))))
		return E_FAIL;

	/* Com_DiffTexture1 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_seat01"),
		TEXT("Com_DiffTexture1"), reinterpret_cast<CComponent**>(&m_pDiffTexture[1]))))
		return E_FAIL;

	/* Com_DiffTexture2 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_lawn"),
		TEXT("Com_DiffTexture2"), reinterpret_cast<CComponent**>(&m_pDiffTexture[2]))))
		return E_FAIL;

	/* Com_DiffTexture3 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_road"),
		TEXT("Com_DiffTexture3"), reinterpret_cast<CComponent**>(&m_pDiffTexture[3]))))
		return E_FAIL;

	/* Com_DiffTexture4 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_banner"),
		TEXT("Com_DiffTexture4"), reinterpret_cast<CComponent**>(&m_pDiffTexture[4]))))
		return E_FAIL;

	/* Com_DiffTexture5 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_LIlandSide"),
		TEXT("Com_DiffTexture5"), reinterpret_cast<CComponent**>(&m_pDiffTexture[5]))))
		return E_FAIL;

	/* Com_DiffTexture6 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_seat_outside"),
		TEXT("Com_DiffTexture6"), reinterpret_cast<CComponent**>(&m_pDiffTexture[6]))))
		return E_FAIL;

	/* Com_DiffTexture7 */
	if (FAILED(__super::Add_Component(LEVEL_LOBBY, TEXT("Prototype_Component_Texture_Lobby_lob_lawn_Edge"),
		TEXT("Com_DiffTexture7"), reinterpret_cast<CComponent**>(&m_pDiffTexture[7]))))
		return E_FAIL;

	return S_OK;
}

HRESULT CLobby_Wring::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;



	return S_OK;
}

CLobby_Wring* CLobby_Wring::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLobby_Wring* pInstance = new CLobby_Wring(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CLobby_Wring"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CLobby_Wring::Clone(void* pArg)
{
	CLobby_Wring* pInstance = new CLobby_Wring(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CLobby_Wring"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLobby_Wring::Free()
{
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);

	for(int i = 0 ; i < 8 ; i++)
		Safe_Release(m_pDiffTexture[i]);

	__super::Free();
}

#include "stdafx.h"
#include "..\Public\QTE_Hit_UI_Result.h"

#include "RenderInstance.h"
#include "GameInstance.h"
#include "QTE_Hit_Situation.h"
CQTE_Hit_UI_Result::CQTE_Hit_UI_Result(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CQTE_Hit_UI_Result::CQTE_Hit_UI_Result(const CQTE_Hit_UI_Result& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CQTE_Hit_UI_Result::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CQTE_Hit_UI_Result::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	Hit_RESULT_DESC* Desc = static_cast<Hit_RESULT_DESC*>(pArg);
	m_fX = Desc->fX;
	m_fY = Desc->fY;
	m_fSizeX = Desc->fSizeX;
	m_fSizeY = Desc->fSizeY;
	m_iTextureNumber = Desc->iTextureNum;
	m_fLifeTime = Desc->fTimer;
	m_fTimer = m_fLifeTime;

	m_fDefaultY = m_fY;         // �ʱ� Y ��ġ
	m_fTargetY = m_fY - 30.0f; // ��ǥ Y ��ġ (��: 100.0f �߰��Ͽ� ���� ����)

	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 0.9f, 1.f));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	SetActive(true);

	return S_OK;
}

void CQTE_Hit_UI_Result::Camera_Update(_float fTimeDelta)
{

}

void CQTE_Hit_UI_Result::Update(_float fTimeDelta)
{
	if (!m_bIsActive)
		return;

	// ���� �ð��� ���ҽ�Ű�� ��� �ð��� ����
	m_fTimer -= fTimeDelta;
	m_fElapsedTime += fTimeDelta;

	// Ratio ��� (0���� 1����)
	_float ratio = Clamp(m_fElapsedTime / m_fLifeTime, 0.f, 1.f);

	// �α� �Լ��� ���� ���� ���� (ó������ �ް��� ����, ���� �ϸ�����)
	_float growthFactor = log(1.0f + 9.0f * ratio) / log(10.0f); // 0���� 1�� ������ ����

	// Y ��ġ ��� (�⺻ Y���� ��ǥ Y����)
	_float currentY = m_fDefaultY + (m_fTargetY - m_fDefaultY) * growthFactor;

	// ��ȯ ������Ʈ�� Y ��ġ ����
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -currentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));

	// Ÿ�̸Ӱ� ������ ����Ʈ�� ��Ȱ��ȭ
	if (m_fTimer <= 0)
	{
		m_fTimer = 0;
		SetActive(false);
	}
}

void CQTE_Hit_UI_Result::Late_Update(_float fTimeDelta)
{
	if (!m_bIsActive)
		return;

	RENDER_OBJECT tDesc{};
	tDesc.tGlowDesc.iPassIndex = 2;
	tDesc.tGlowDesc.fGlowFactor = 5.f;

	m_pRenderInstance->Add_RenderObject(CRenderer::RG_MULTY_GLOW, this, &tDesc);
}

HRESULT CQTE_Hit_UI_Result::Render(_float fTimeDelta)
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(0)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}


HRESULT CQTE_Hit_UI_Result::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_UI_VtxRect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_QTE_Result"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CQTE_Hit_UI_Result::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureNumber)))
		return E_FAIL;

	return S_OK;
}

CQTE_Hit_UI_Result* CQTE_Hit_UI_Result::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CQTE_Hit_UI_Result* pInstance = new CQTE_Hit_UI_Result(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CQTE_Hit_UI_Result"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CQTE_Hit_UI_Result::Clone(void* pArg)
{
	CQTE_Hit_UI_Result* pInstance = new CQTE_Hit_UI_Result(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CQTE_Hit_UI_Result"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQTE_Hit_UI_Result::Free()
{
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	__super::Free();
}

#include "stdafx.h"
#include "..\Public\QTE_Continuous_Attack_Space.h"

#include "RenderInstance.h"
#include "GameInstance.h"

CQTE_Continuous_Attack_Space::CQTE_Continuous_Attack_Space(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CQTE_Continuous_Attack_Space::CQTE_Continuous_Attack_Space(const CQTE_Continuous_Attack_Space& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CQTE_Continuous_Attack_Space::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CQTE_Continuous_Attack_Space::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	CONTINUOUS_ATTACK_DESC* Desc = static_cast<CONTINUOUS_ATTACK_DESC*>(pArg);

	m_fX = Desc->fX;
	m_fY = Desc->fY;
	m_fSizeX = Desc->fSizeX;
	m_fSizeY = Desc->fSizeY;

	// �⺻ ��ġ ����
	m_fDefaultY = m_fY;
	// Ÿ�� ��ġ ���� (���÷� Y ��ǥ�� 100��ŭ �Ʒ��� �̵�)
	m_fTargetY = m_fDefaultY + 30.f;
	// �ʱ� ��ġ�� �⺻ ��ġ�� ����

	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 0.9f, 1.f));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	SetActive(true);

	return S_OK;
}

void CQTE_Continuous_Attack_Space::Camera_Update(_float fTimeDelta)
{

}

void CQTE_Continuous_Attack_Space::Update(_float fTimeDelta)
{
   	if (!m_bIsActive)
		return;

	Update_Animation(fTimeDelta);
}

void CQTE_Continuous_Attack_Space::Late_Update(_float fTimeDelta)
{
	if (!m_bIsActive)
		return;

	m_pRenderInstance->Add_RenderObject(CRenderer::RG_UI, this);
}

void CQTE_Continuous_Attack_Space::Update_Animation(_float fTimeDelta)
{
	// ���� ���� �ð� ������Ʈ
	m_fCurrentTime += fTimeDelta;

	if (m_bIsMovingDown)
	{
		// �������� �ִϸ��̼� ����
		_float fProgress = m_fCurrentTime / m_fMoveDownTime;

		if (fProgress >= 1.0f)
		{
			// �������� �Ϸ�, �ö󰡱� ����
			fProgress = 1.0f;
			m_bIsMovingDown = false;
			m_fCurrentTime = 0.0f;
		}

		// ��¡ �Լ� ���� (Ease-In-Out)
		_float fEaseProgress = EaseInOut(fProgress);

		// ��ġ ���
		m_fY = Lerp(m_fDefaultY, m_fTargetY, fEaseProgress);
	}
	else
	{
		// �ö󰡴� �ִϸ��̼� ����
		_float fProgress = m_fCurrentTime / m_fMoveUpTime;

		if (fProgress >= 1.0f)
		{
			// �ִϸ��̼� ����
			fProgress = 1.0f;
			m_bIsMoving = false;
		}

		// ��¡ �Լ� ���� (Ease-In-Out)
		_float fEaseProgress = EaseInOut(fProgress);

		// ��ġ ���
		m_fY = Lerp(m_fTargetY, m_fDefaultY, fEaseProgress);
	}

	// ���� Transform�� ��ġ ����
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 0.9f, 1.f));
}

void CQTE_Continuous_Attack_Space::Process_Command()
{
	// �ִϸ��̼� ���� �ʱ�ȭ
	m_bIsMoving = true;
	m_bIsMovingDown = true;   // ���� �������� ����
	m_fCurrentTime = 0.0f;    // ���� �ð� �ʱ�ȭ

	// ��ġ�� �⺻ ��ġ�� �ʱ�ȭ
	m_fY = m_fDefaultY;

	// Transform�� ��ġ ����
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fY + g_iWinSizeY * 0.5f, 0.9f, 1.f));
}

HRESULT CQTE_Continuous_Attack_Space::Render(_float fTimeDelta)
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


HRESULT CQTE_Continuous_Attack_Space::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_UI_VtxRect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_QTE_Space"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CQTE_Continuous_Attack_Space::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	return S_OK;
}



CQTE_Continuous_Attack_Space* CQTE_Continuous_Attack_Space::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CQTE_Continuous_Attack_Space* pInstance = new CQTE_Continuous_Attack_Space(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CQTE_Continuous_Attack_Space"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CQTE_Continuous_Attack_Space::Clone(void* pArg)
{
	CQTE_Continuous_Attack_Space* pInstance = new CQTE_Continuous_Attack_Space(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CQTE_Continuous_Attack_Space"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQTE_Continuous_Attack_Space::Free()
{
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	__super::Free();
}

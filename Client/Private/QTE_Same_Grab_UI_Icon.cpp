#include "stdafx.h"
#include "..\Public\QTE_Same_Grab_UI_Icon.h"

#include "RenderInstance.h"
#include "GameInstance.h"
#include "QTE_Same_Grab.h"
#include "QTE_1P_Same_Grab.h"

CQTE_Same_Grab_UI_Icon::CQTE_Same_Grab_UI_Icon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CQTE_Same_Grab_UI_Icon::CQTE_Same_Grab_UI_Icon(const CQTE_Same_Grab_UI_Icon& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CQTE_Same_Grab_UI_Icon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CQTE_Same_Grab_UI_Icon::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	CQTE_Same_Grab_UI_Icon::QTE_Same_Grab_UI_ICON_DESC* desc = static_cast<CQTE_Same_Grab_UI_Icon::QTE_Same_Grab_UI_ICON_DESC*>(pArg);
	m_fSizeX = desc->fSizeX;
	m_fSizeY = desc->fSizeY;
	m_fX = desc->fX;
	m_fY = desc->fY;
	m_fAlpha = desc->fAlpha;
	m_iTextureNumber = desc->iTextureNumber;
	m_fFallDelay = desc->fFallDelay; // �߰�: ��ٿ� ���� �ð� ����
	m_bIsFirst = desc->isFirst;
	m_bIsLast = desc->isLast;
	m_pSameGrab = desc->SameGrab;

	// �ʱ� Y ��ġ ����
	m_fCurrentY = m_fStartY;    // �ʱ� ��ġ�� -100
	m_fTargetY = m_fDefault_Y;  // ���� ��ġ�� m_fDefault_Y

	m_pTransformCom->Set_Scaled(m_fSizeX, m_fSizeY, 1.f);
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fCurrentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));

	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_ProjMatrix, XMMatrixOrthographicLH(g_iWinSizeX, g_iWinSizeY, 0.f, 1.f));

	// �������� �ִϸ��̼� ����
	Set_State(FALLING);

	return S_OK;
}

void CQTE_Same_Grab_UI_Icon::Camera_Update(_float fTimeDelta)
{

}

void CQTE_Same_Grab_UI_Icon::Update(_float fTimeDelta)
{
	switch (m_State)
	{
	case FALLING:
		Update_Falling(fTimeDelta);
		break;

	case SELECTED:
		Update_Selected(fTimeDelta);
		break;

	case ALREADY_PRESSED:
		Update_AlreadyPressed(fTimeDelta);
		break;

	case WRONG_PRESSED:
		Update_WrongPressed(fTimeDelta);
		break;

	case ASCEND:
		Update_Ascend(fTimeDelta);
		break;

	case NOT_SELECTED:
	default:
		Update_NotSelected(fTimeDelta);
		break;
	}
}

void CQTE_Same_Grab_UI_Icon::Late_Update(_float fTimeDelta)
{
	m_pRenderInstance->Add_RenderObject(CRenderer::RG_UI, this);
}

HRESULT CQTE_Same_Grab_UI_Icon::Render(_float fTimeDelta)
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(1)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CQTE_Same_Grab_UI_Icon::Set_State(IconState state)
{
	m_State = state;

	switch (m_State)
	{
	case NOT_SELECTED:
		m_fTargetY = m_fDefault_Y;
		m_bIsShaking = false;
		break;

	case FALLING:
		//Falling�϶�, ù��° �༮�� TargetY�� �޶����
		m_fTargetY = m_bIsFirst == true ? m_fSelected_Y : m_fDefault_Y;
		m_fElapsedTime = 0.f; // �ִϸ��̼� �ð� �ʱ�ȭ
		m_bIsShaking = false;
		break;

	case SELECTED:
		m_fTargetY = m_fSelected_Y;
		m_fElapsedTime = 0.f; // �ִϸ��̼� �ð� �ʱ�ȭ
		m_bIsShaking = false;
		break;

	case ALREADY_PRESSED:
		m_fTargetY = m_fDefault_Y;
		m_fElapsedTime = 0.f; // �ִϸ��̼� �ð� �ʱ�ȭ
		m_bIsShaking = false;
		break;

	case WRONG_PRESSED:
		m_fTargetY = m_fSelected_Y;
		m_fShakeTime = 0.0f;        // �ʱ� ��鸲 �ð�
		m_bIsShaking = true;         // ��鸲 ����
		break;

	case ASCEND:
		m_fTargetY = m_fStartY;      // �⺻ Y ��ġ�� ����
		m_fAscendDelay = m_fElapsedTime; // ASCEND ���� ���� �� ���� �ð� ���� (�ܺο��� ����)
		m_fElapsedTime = 0.f; // �ִϸ��̼� �ð� �ʱ�ȭ
		m_bIsAscending = false; // �ö󰡱� �ִϸ��̼� ���� ��
		break;

	default:
		break;
	}
}

HRESULT CQTE_Same_Grab_UI_Icon::Ready_Components()
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_QTE_VtxRect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_UI_ActionInput"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;


	/* Com_VIBuffer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CQTE_Same_Grab_UI_Icon::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", m_iTextureNumber)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_IconState", &m_State, sizeof(_int))))
		return E_FAIL;

	return S_OK;
}

void CQTE_Same_Grab_UI_Icon::Update_Falling(_float fTimeDelta)
{
	if (m_bIsFalling)
	{
		m_fElapsedTime += fTimeDelta;
		float t = Clamp(m_fElapsedTime / m_fAnimationDuration, 0.f, 1.f);
		// ���� ���� (linear interpolation)
		m_fCurrentY = Lerp(m_fStartY, m_fTargetY, t);

		// �ִϸ��̼� �Ϸ� �� ���� ����
		if (t >= 1.0f)
		{
			m_bIsFalling = false;

			//Same_Grab���� ������ ��ü�� Final_UI�� �������ٰ� �˷�����
			if (m_bIsLast)
			{
				CQTE_Same_Grab* IsSameGrab = dynamic_cast<CQTE_Same_Grab*>(m_pSameGrab);
				if (IsSameGrab)
					IsSameGrab->Notify_Last_UI_Final_Complete();
				else
				{
					CQTE_1P_Same_Grab* is1pSameGrab = dynamic_cast<CQTE_1P_Same_Grab*>(m_pSameGrab);
					is1pSameGrab->Notify_Last_UI_Final_Complete();
				}
			}

			Set_State(NOT_SELECTED); // �ִϸ��̼� �Ϸ� �� �⺻ ���·� ��ȯ
		}

		// ��ġ ������Ʈ
		m_pTransformCom->Set_State(CTransform::STATE_POSITION,
			XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fCurrentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));
	}
	else
	{
		// �������� ���� �� ���� �ð� ó��
		m_fElapsedTime += fTimeDelta;
		if (m_fElapsedTime >= m_fFallDelay)
		{
			m_bIsFalling = true;
			m_fElapsedTime = 0.f; // �ִϸ��̼� �ð� �ʱ�ȭ
		}
	}
}

void CQTE_Same_Grab_UI_Icon::Update_Selected(_float fTimeDelta)
{
	// �ε巯�� �̵��� ���� �ִϸ��̼� ó��
	m_fElapsedTime += fTimeDelta;
	float t = Clamp(m_fElapsedTime / m_fAnimationDuration, 0.f, 1.f);

	// ���� �ִϸ��̼� Ŀ�� ���� (ease-out)
	t = 1.0f - powf(1.0f - t, 3);

	// ���� ���� ��� ���� Ŀ�긦 ������ t�� ����Ͽ� Y ��ġ ������Ʈ
	m_fCurrentY = Lerp(m_fCurrentY, m_fTargetY, t);

	// ��ġ ������Ʈ
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fCurrentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));
}

void CQTE_Same_Grab_UI_Icon::Update_AlreadyPressed(_float fTimeDelta)
{
	// �ִϸ��̼� ���� �ð� ����
	m_fElapsedTime += fTimeDelta;

	// �ִϸ��̼� ���� ���� ��� (0.0f ~ 1.0f)
	float t = Clamp(m_fElapsedTime / m_fAnimationDuration, 0.f, 1.f);

	// ���� �ִϸ��̼� Ŀ�� ���� (ease-out)
	t = 1.0f - powf(1.0f - t, 3);

	// Y ��ġ �ε巴�� �̵� (LERP ���)
	m_fCurrentY = Lerp(m_fCurrentY, m_fTargetY, t);

	// ��ġ ������Ʈ
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fCurrentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));
}

void CQTE_Same_Grab_UI_Icon::Update_WrongPressed(_float fTimeDelta)
{
	// ��鸲 �ִϸ��̼� ó��
	if (m_bIsShaking)
	{
		m_fShakeTime += fTimeDelta;

		if (m_fShakeTime >= m_fShakeDuration)
		{
			// ��鸲 ���� �� ���¸� SELECTED�� ����
			m_bIsShaking = false;
			Set_State(SELECTED);
		}
		else
		{
			// ��鸲 ���
			float shakeOffsetX = m_fShakeAmplitude * sinf(2.0f * XM_PI * m_fShakeFrequency * m_fShakeTime);
			// ���� ����
			float damping = 1.0f - (m_fShakeTime / m_fShakeDuration);
			shakeOffsetX *= damping;

			// ��鸲�� ����� X ��ġ ���
			float shakenX = m_fX + shakeOffsetX;

			// ��鸲�� ����� ��ġ ����
			m_pTransformCom->Set_State(CTransform::STATE_POSITION,
				XMVectorSet(shakenX - g_iWinSizeX * 0.5f, -m_fCurrentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));
		}
	}
}

void CQTE_Same_Grab_UI_Icon::Update_Ascend(_float fTimeDelta)
{
	// ��� ���� �� ���� �ð� ó��
	if (!m_bIsAscending)
	{
		m_fElapsedTime += fTimeDelta;
		if (m_fElapsedTime >= m_fAscendDelay)
		{
			m_bIsAscending = true;
			m_fElapsedTime = 0.f; // �ִϸ��̼� �ð� �ʱ�ȭ
		}
		return;
	}

	// ��� �ִϸ��̼� ó��
	m_fElapsedTime += fTimeDelta;
	float t = Clamp(m_fElapsedTime / m_fAnimationDuration, 0.f, 1.f);

	// ���� �ִϸ��̼� Ŀ�� ���� (ease-out)
	t = 1.0f - powf(1.0f - t, 3);

	// Y ��ġ �ε巴�� �̵� (LERP ���)
	m_fCurrentY = Lerp(m_fCurrentY, m_fTargetY, t);

	// ��ġ ������Ʈ
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fCurrentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));

}

void CQTE_Same_Grab_UI_Icon::Update_NotSelected(_float fTimeDelta)
{
	// ��ġ ������Ʈ
	m_pTransformCom->Set_State(CTransform::STATE_POSITION,
		XMVectorSet(m_fX - g_iWinSizeX * 0.5f, -m_fCurrentY + g_iWinSizeY * 0.5f, 0.9f, 1.f));
}

CQTE_Same_Grab_UI_Icon* CQTE_Same_Grab_UI_Icon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CQTE_Same_Grab_UI_Icon* pInstance = new CQTE_Same_Grab_UI_Icon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CQTE_Same_Grab_UI_Icon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CQTE_Same_Grab_UI_Icon::Clone(void* pArg)
{
	CQTE_Same_Grab_UI_Icon* pInstance = new CQTE_Same_Grab_UI_Icon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CQTE_Same_Grab_UI_Icon"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CQTE_Same_Grab_UI_Icon::Free()
{
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pVIBufferCom);

	__super::Free();
}

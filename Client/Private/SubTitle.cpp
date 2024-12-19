#include "stdafx.h"
#include "..\Public\SubTitle.h"

#include "RenderInstance.h"
#include "GameInstance.h"

CSubTitle::CSubTitle(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CSubTitle::CSubTitle(const CSubTitle& Prototype)
	: CGameObject{ Prototype }
{

}

HRESULT CSubTitle::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CSubTitle::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(nullptr)))
		return E_FAIL;

	SetActive(false);

	return S_OK;
}

HRESULT CSubTitle::Initialize_SubTitle(const _wstring& strText, _float fDuration)
{
	m_strText = strText;
	m_fDuration = fDuration;

	return S_OK;
}


void CSubTitle::Camera_Update(_float fTimeDelta)
{

}

void CSubTitle::Update(_float fTimeDelta)
{
	if (!m_bIsActive)
		return;

	m_fDuration -= fTimeDelta;
	if (m_fDuration <= 0.f)
	{
		SetActive(false);
	}
}

void CSubTitle::Late_Update(_float fTimeDelta)
{
	if (!m_bIsActive)
		return;

	m_pRenderInstance->Add_RenderObject(CRenderer::RG_UI, this);

}

HRESULT CSubTitle::Render(_float fTimeDelta)
{
	// ���� ǥ�� ���� �ڸ� �ؽ�Ʈ
	_wstring wsSubTitle = m_strText;

	// �⺻ �ڸ� ��ġ (����: �߾�, ����: �ϴܿ��� �ణ ��)
	_float2 basePos(1920.f * 0.5f, 1080.f * 0.85f); // �߽ɰ� ȭ�� �ϴ� ����
	_vector textColor = XMVectorSet(1.f, 1.f, 1.f, 1.f); // ���
	_vector shadowTextColor = XMVectorSet(0.f, 0.f, 0.f, 1.f); // 

	// ��Ʈ ũ��
	_float adjustedFontSize = 0.6f; // ��Ʈ ũ��
	// ������ ũ��
	 _float shadowFontSize = 4.f; // ��Ʈ ũ��
	 
	// �ؽ�Ʈ ���� ��� (��Ʈ ��ü�� ���� ���̸� ��´ٰ� ����)
	_float textWidth = wsSubTitle.length();

	// �߽� ����: �ؽ�Ʈ�� ���� ���̸�ŭ �������� �̵�
	_float2 adjustedPos = basePos;
	adjustedPos.x -= textWidth * 10.f;

	// �ڸ� ������
	//m_pGameInstance->Draw_Font(TEXT("Font_Nexon"), wsSubTitle.c_str(), adjustedPos, textColor, 0.f, { 0.f, 0.f }, adjustedFontSize);
	m_pGameInstance->Draw_FontShadow(TEXT("Font_Nexon"), wsSubTitle.c_str(), adjustedPos, textColor, shadowTextColor, shadowFontSize, 0.f, { 0.f,0.f }, adjustedFontSize);

	return S_OK;
}

CSubTitle* CSubTitle::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSubTitle* pInstance = new CSubTitle(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX(TEXT("Failed to Created : CSubTitle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSubTitle::Clone(void* pArg)
{
	CSubTitle* pInstance = new CSubTitle(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX(TEXT("Failed to Cloned : CSubTitle"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSubTitle::Free()
{
	__super::Free();
}

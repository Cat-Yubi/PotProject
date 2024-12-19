#include "..\Public\Light.h"
#include "Shader.h"
#include "VIBuffer_Rect.h"

CLight::CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC& LightDesc, _float4 vChaseColor, _bool* pisChaseLight)
{
	m_LightDesc = LightDesc;

	if (pisChaseLight != nullptr)
	{
		m_ChaseLightDesc = LightDesc;
		//m_ChaseLightDesc.vDiffuse = { vChaseColor.x * 1.5f,vChaseColor.y * 1.5f,vChaseColor.z * 1.5f,1.f };
		m_ChaseLightDesc.vDiffuse = { vChaseColor.x ,vChaseColor.y ,vChaseColor.z ,1.f };
		
		m_ChaseLightDesc.vAmbient = _float4(0.01f, 0.01f, 0.01f, 1.f);
		m_ChaseLightDesc.strName = "Chase";

		m_pIsChaseLight = pisChaseLight;
	}
	return S_OK;
}

HRESULT CLight::Render_Map(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	_uint		iPassIndex = { 0 };
	 
	if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
	{
		iPassIndex = 1;

		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;
	}
	else
	{
		iPassIndex = 2;
		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;
	}

	if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;
	
	pShader->Begin(iPassIndex);

	pVIBuffer->Render();

	return S_OK;
}

HRESULT CLight::Render_Player(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	_uint		iPassIndex = { 0 };

	if (m_pIsChaseLight != nullptr && *m_pIsChaseLight == false)
	{

		if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
		{
			iPassIndex = 5;

			if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
				return E_FAIL;

			if (FAILED(pShader->Bind_RawValue("g_iPlayerDirection", m_LightDesc.pPlayerDirection, sizeof(_int))))
				return E_FAIL;
		}

		if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_iPlayerDirection", m_LightDesc.pPlayerDirection, sizeof(_int))))
			return E_FAIL;
	}
	else if (m_pIsChaseLight != nullptr && *m_pIsChaseLight == true)
	{
		if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
		{
			iPassIndex = 5;

			if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_ChaseLightDesc.vDirection, sizeof(_float4))))
				return E_FAIL;

			if (FAILED(pShader->Bind_RawValue("g_iPlayerDirection", m_ChaseLightDesc.pPlayerDirection, sizeof(_int))))
				return E_FAIL;
		}

		if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_ChaseLightDesc.vDiffuse, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &m_ChaseLightDesc.vAmbient, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &m_ChaseLightDesc.vSpecular, sizeof(_float4))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_iPlayerDirection", m_ChaseLightDesc.pPlayerDirection, sizeof(_int))))
			return E_FAIL;

		if (FAILED(pShader->Bind_RawValue("g_isChase", m_pIsChaseLight, sizeof(_bool))))
			return E_FAIL;
	}

	pShader->Begin(iPassIndex);

	pVIBuffer->Render();

	return S_OK;
}

HRESULT CLight::Render_Effect(CShader* pShader, CVIBuffer_Rect* pVIBuffer, LIGHT_DESC* pLightDesc)
{
	_uint		iPassIndex = { 0 };
	iPassIndex = 6;

	//if (FAILED(pShader->Bind_RawValue("g_iPlayerDirection", m_LightDesc.pPlayerDirection, sizeof(_int))))
	//	return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_iPlayerDirection", pLightDesc->pPlayerDirection, sizeof(_int))))
		return E_FAIL;


	if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_vLightAmbient", &m_LightDesc.vAmbient, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
		return E_FAIL;



	if (FAILED(pShader->Bind_RawValue("g_fLightAccTime", &m_LightDesc.fAccTime, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fLightLifeTime", &m_LightDesc.fLifeTime, sizeof(_float))))
		return E_FAIL;

	/*
	* 
	float g_fLightAccTime;
	float g_fLightLifeTime;
	�̹� ���� m_fTimeDelta�� ������ ���� ���� �ð������� �Ӱ����� �������� �Ǵ��غ��ߵ�
	���� ������ �ִٸ� ���ĸ� ����ð� m_fAccTime �� m_fLifeTime ���̰�
	
	*/

	pShader->Begin(iPassIndex);

	pVIBuffer->Render();

	return S_OK;
}

HRESULT CLight::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer, _int iPassIndex, LIGHT_DESC* pLightDesc)
{
	switch (iPassIndex)
	{
	case 1: /* Map Obj */
	if (FAILED(Render_Map(pShader, pVIBuffer)))
		return E_FAIL;
	break;
	case 5: /* Player */
	if (FAILED(Render_Player(pShader, pVIBuffer)))
		return E_FAIL;
	break;
	case 6: /* Effect */
	if (FAILED(Render_Effect(pShader, pVIBuffer, pLightDesc)))
		return E_FAIL;
	break;
	}

	return S_OK;
}

CLight * CLight::Create(const LIGHT_DESC & LightDesc, _float4 vChaseColor, _bool* pisChaseLight)
{
	CLight*		pInstance = new CLight();

	if (FAILED(pInstance->Initialize(LightDesc, vChaseColor, pisChaseLight)))
	{
		MSG_BOX(TEXT("Failed to Created : CLight"));
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CLight::Free()
{
	__super::Free();

}
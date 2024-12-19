#include "..\Public\VIBuffer_Instancing.h"
#include <random>

CVIBuffer_Instancing::CVIBuffer_Instancing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CVIBuffer{ pDevice, pContext }
{
}

CVIBuffer_Instancing::CVIBuffer_Instancing(const CVIBuffer_Instancing& Prototype)
	: CVIBuffer{ Prototype }
	, m_pVBInstance{ Prototype.m_pVBInstance }
	, m_iNumInstance{ Prototype.m_iNumInstance }
	, m_iInstanceVertexStride{ Prototype.m_iInstanceVertexStride }
	, m_iNumIndexPerInstance{ Prototype.m_iNumIndexPerInstance }
	, m_pInstanceVertices{ Prototype.m_pInstanceVertices }
	, m_pSpeeds{ Prototype.m_pSpeeds }
	, m_vPivotPos{ Prototype.m_vPivotPos }
	, m_isLoop{ Prototype.m_isLoop }
	, m_fMaxTime{ Prototype.m_fMaxTime }
{
	Safe_AddRef(m_pVBInstance);
}

HRESULT CVIBuffer_Instancing::Initialize_Prototype(const VIBUFFER_INSTANCE_DESC* pInitialDesc)
{
	m_iNumInstance = pInitialDesc->iNumInstance;

	m_pSpeeds = new float[m_iNumInstance];

	m_vPivotPos = pInitialDesc->vPivot;

	m_isLoop = pInitialDesc->isLoop;

	m_fMaxTime = pInitialDesc->vLifeTime.y;

	/* �ΰ��ӳ����� ���ǵ尡 �ٲ��� �ʵ��� ��ƼŬ ������ü�� �����Ҷ� �� ���ں� ���ǵ带 ����Ͽ� �����صд�. */
	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		float speed = Get_Random(pInitialDesc->vSpeed.x, pInitialDesc->vSpeed.y);
		m_pSpeeds[i] = speed;
	}

	return S_OK;
}

HRESULT CVIBuffer_Instancing::Initialize(void* pArg)
{

	return S_OK;
}

HRESULT CVIBuffer_Instancing::Bind_Buffers()
{

	ID3D11Buffer* pVertexBufffers[] = {
		m_pVB,
		m_pVBInstance
	};

	_uint				iVertexStrides[] = {
		m_iVertexStride,
		m_iInstanceVertexStride
	};

	_uint				iOffsets[] = {
		0,
		0
	};

	m_pContext->IASetVertexBuffers(0, m_iNumVertexBuffers, pVertexBufffers, iVertexStrides, iOffsets);
	m_pContext->IASetIndexBuffer(m_pIB, m_eIndexFormat, 0);
	m_pContext->IASetPrimitiveTopology(m_ePrimitive_Topology);

	return S_OK;
}

HRESULT CVIBuffer_Instancing::Render()
{
	m_pContext->DrawIndexedInstanced(m_iNumIndexPerInstance, m_iNumInstance, 0, 0, 0);

	return S_OK;
}


HRESULT CVIBuffer_Instancing::Create_InstanceBuffer()
{
	ZeroMemory(&m_BufferDesc, sizeof(m_BufferDesc));

	m_BufferDesc.ByteWidth = m_iNumInstance * m_iInstanceVertexStride;
	m_BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	m_BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	m_BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	m_BufferDesc.MiscFlags = 0;
	m_BufferDesc.StructureByteStride = m_iInstanceVertexStride;

	ZeroMemory(&m_InitialData, sizeof m_InitialData);
	m_InitialData.pSysMem = m_pInstanceVertices;

	if (FAILED(__super::Create_Buffer(&m_pVBInstance)))
		return E_FAIL;

	return S_OK;
}

_bool CVIBuffer_Instancing::Spread(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ������Ÿ�� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		if (pMatrices[i].vLifeTime.y >= 0.f)
		{
			_vector		vMoveDir = XMVector3Normalize(XMLoadFloat4(&m_pInstanceVertices[i].vTranslation) - XMVectorSetW(XMLoadFloat3(&m_vPivotPos), 1.f));

			// Store moveDir in the instance buffer
			XMStoreFloat3(&pMatrices[i].vMoveDir, vMoveDir);

			XMStoreFloat4(&pMatrices[i].vTranslation,
				XMLoadFloat4(&pMatrices[i].vTranslation) + vMoveDir * m_pSpeeds[i] * fTimeDelta);
		}

		if (pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{

			if (m_isLoop == true)
			{
				// ��ƼŬ�� �ʱ� ��ġ�� ����
				pMatrices[i].vTranslation = m_pInstanceVertices[i].vTranslation;

				// lifeTime.y�� -lifeTime.x�� 0 ������ ������ ������ �缳��
				pMatrices[i].vLifeTime.y = -RandomBetween(0.f, pMatrices[i].vLifeTime.x);
			}
			else
			{
				// ��ƼŬ ���� �ֱⰡ ������ �� ó��
				m_pContext->Unmap(m_pVBInstance, 0);
				return true;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}


_bool CVIBuffer_Instancing::FocusPoint(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ������Ÿ�� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		if (pMatrices[i].vLifeTime.y >= 0.f)
		{
			_vector vMoveDir = XMVector3Normalize(XMVectorSetW(XMLoadFloat3(&m_vPivotPos), 1.f) - XMLoadFloat4(&m_pInstanceVertices[i].vTranslation));

			// �̵� ������ �ν��Ͻ� ���ۿ� ����
			XMStoreFloat3(&pMatrices[i].vMoveDir, vMoveDir);

			// ��ƼŬ ��ġ ������Ʈ
			XMStoreFloat4(&pMatrices[i].vTranslation,
				XMLoadFloat4(&pMatrices[i].vTranslation) + vMoveDir * m_pSpeeds[i] * fTimeDelta);
		}

		if (pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			if (m_isLoop == true)
			{
				// ��ƼŬ�� �ʱ� ��ġ�� ����
				pMatrices[i].vTranslation = m_pInstanceVertices[i].vTranslation;

				// lifeTime.y�� -lifeTime.x�� 0 ������ ������ ������ �缳��
				pMatrices[i].vLifeTime.y = -RandomBetween(0.f, pMatrices[i].vLifeTime.x);
			}
			else
			{
				// ��ƼŬ ���� �ֱⰡ ������ �� ó��
				m_pContext->Unmap(m_pVBInstance, 0);
				return true;
			}
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

void CVIBuffer_Instancing::Particle_Initialize()
{
	D3D11_MAPPED_SUBRESOURCE		MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// �� �ν��Ͻ����� ��� �ʵ带 �ùٸ��� �����մϴ�.
		pMatrices[i].vRight = m_pInstanceVertices[i].vRight;
		pMatrices[i].vUp = m_pInstanceVertices[i].vUp;
		pMatrices[i].vLook = m_pInstanceVertices[i].vLook;
		pMatrices[i].vMoveDir = m_pInstanceVertices[i].vMoveDir;
		pMatrices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
		pMatrices[i].vLifeTime = m_pInstanceVertices[i].vLifeTime;
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}



_bool CVIBuffer_Instancing::Spread_2D(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ���� ��ƼŬ�� ��ġ�� ���������� �ε�
		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		// �Ǻ� ��ġ�� ���������� �ε�
		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		// ���� ��ġ�� �Ǻ� ��ġ�� �̿��� �̵� ���� ���
		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			// �ʱ� ��ġ�� ����
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
		// ������ �ȵ����� ������Ÿ�� �ð��� �Ѿ�� ��
		else if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			m_pContext->Unmap(m_pVBInstance, 0);
			return true;
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Spread_FireCracker_2D(_float fTimeDelta)
{
	//MaxTime�� �Ѿ�� ��� ��ƼŬ ���ڰ� �� �ð��� �����ٰ� �Ǵ�
	if (m_fElapsedTime >= m_fMaxTime)
		return true;

	D3D11_MAPPED_SUBRESOURCE		MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	// ���� ����
	const float decelerationFactor = 0.8f; // �ӵ� ���� ����
	const float gravityStrength = -0.8f;  // �߷� ���ӵ�

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
			continue;

		// ���� �ð� ���� ���
		float ratio = pMatrices[i].vLifeTime.y / pMatrices[i].vLifeTime.x;

		// ���� ��ƼŬ�� ��ġ�� ���������� �ε�
		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		// �Ǻ� ��ġ�� ���������� �ε�
		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		// ���� ��ġ�� �Ǻ� ��ġ�� �̿��� �̵� ���� ���
		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// �ʱ� �ӵ� ��������
		float initialSpeed = m_pSpeeds[i];

		float vx = 0.f;
		float vy = 0.f;

		if (ratio < 0.7f)
		{
			// �ӵ� ���� ���
			float speedDamping = 1.0f - (ratio / 0.7f) * decelerationFactor; // ���� ���� ����

			// ���� �ӵ� ���
			vx = dirX * initialSpeed * speedDamping;
			vy = dirY * initialSpeed * speedDamping;
		}
		else
		{
			// ���ӵ� �ӵ��� ����
			float speedDamping = 1.0f - decelerationFactor; // ���� �� ���� �ӵ�

			// ���� �ӵ��� ���ӵ� ���� ����
			vx = dirX * initialSpeed * speedDamping;

			// �߷� ���ӵ� ���� (�ӵ��� ����)
			vy = dirY * initialSpeed * speedDamping + gravityStrength * (pMatrices[i].vLifeTime.y - pMatrices[i].vLifeTime.x * 0.7f);
		}

		// ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += vx * fTimeDelta;
		pMatrices[i].vTranslation.y += vy * fTimeDelta;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			// �ʱ� ��ġ�� ����
			pMatrices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_fElapsedTime += fTimeDelta;

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Half_Spread_X_2D(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ���� ��ƼŬ�� ��ġ�� ���������� �ε�
		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		// �Ǻ� ��ġ�� ���������� �ε�
		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		// ���� ��ġ�� �Ǻ� ��ġ�� �̿��� �̵� ���� ���
		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// �Ʒ��� ���ϴ� ��� dirY�� ����� �����Ͽ� ���� ���ϵ��� ����
		if (dirX < 0.f)
		{
			dirX = -dirX;
			dirY = -dirY;
		}


		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			// �ʱ� ��ġ�� ����
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
		// ������ �ȵ����� ������Ÿ�� �ð��� �Ѿ�� ��
		else if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			m_pContext->Unmap(m_pVBInstance, 0);
			return true;
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Half_Spread_Y_2D(_float fTimeDelta)
{
	//MaxTime�� �Ѿ�� ��� ��ƼŬ ���ڰ� �� �ð��� �����ٰ� �Ǵ�
	if (m_fElapsedTime >= m_fMaxTime)
		return true;

	D3D11_MAPPED_SUBRESOURCE		MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ������ �ȵ����� ������Ÿ�� �ð��� �Ѿ�� ��
		if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
			continue;

		// ���� ��ƼŬ�� ��ġ�� ���������� �ε�
		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		// �Ǻ� ��ġ�� ���������� �ε�
		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		// ���� ��ġ�� �Ǻ� ��ġ�� �̿��� �̵� ���� ���
		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// �Ʒ��� ���ϴ� ��� dirY�� ����� �����Ͽ� ���� ���ϵ��� ����
		if (dirY < 0.f)
		{
			dirX = -dirX;
			dirY = -dirY;
		}


		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			// �ʱ� ��ġ�� ����
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_fElapsedTime += fTimeDelta;

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Spiral_Spread_2D(_float fTimeDelta)
{
	//MaxTime�� �Ѿ�� ��� ��ƼŬ ���ڰ� �� �ð��� �����ٰ� �Ǵ�
	if (m_fElapsedTime >= m_fMaxTime)
		return true;

	D3D11_MAPPED_SUBRESOURCE MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	const float spiralSpeed = XM_PI; // ȸ�� �ӵ� (���� ����)

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ������ �ȵ����� ������Ÿ�� �ð��� �Ѿ�� ��
		if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
			continue;

		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// ���� ���
		float angle = atan2(dirY, dirX);
		angle += spiralSpeed * fTimeDelta; // ȸ��

		// ���ο� ���� ���� ���
		dirX = cosf(angle);
		dirY = sinf(angle);

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_fElapsedTime += fTimeDelta;

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Circular_Spread_2D(_float fTimeDelta)
{
	//MaxTime�� �Ѿ�� ��� ��ƼŬ ���ڰ� �� �ð��� �����ٰ� �Ǵ�
	if (m_fElapsedTime >= m_fMaxTime)
		return true;

	D3D11_MAPPED_SUBRESOURCE MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ������ �ȵ����� ������Ÿ�� �ð��� �Ѿ�� ��
		if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
			continue;

		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// ���� ����ȭ (0�� ~ 360��)
		float angle = Get_Random(0.f, 360.f) * (XM_PI / 180.f);
		dirX = cosf(angle);
		dirY = sinf(angle);

		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_fElapsedTime += fTimeDelta;

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Elliptical_Spread_2D(_float fTimeDelta)
{
	//MaxTime�� �Ѿ�� ��� ��ƼŬ ���ڰ� �� �ð��� �����ٰ� �Ǵ�
	if (m_fElapsedTime >= m_fMaxTime)
		return true;

	D3D11_MAPPED_SUBRESOURCE MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ������ �ȵ����� ������Ÿ�� �ð��� �Ѿ�� ��
		if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
			continue;

		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// Ÿ���� �������带 ���� Y�� �ӵ� ����
		float ellipseFactor = 0.5f; // Y�� �ӵ� ����
		dirY *= ellipseFactor;

		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_fElapsedTime += fTimeDelta;
	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Random_Wiggle_Spread_2D(_float fTimeDelta)
{
	//MaxTime�� �Ѿ�� ��� ��ƼŬ ���ڰ� �� �ð��� �����ٰ� �Ǵ�
	if (m_fElapsedTime >= m_fMaxTime)
		return true;

	D3D11_MAPPED_SUBRESOURCE MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	const float scaleFactor = 50.f; // ���� ���� ������ ����

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// ������ �ȵ����� ������Ÿ�� �ð��� �Ѿ�� ��
		if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
			continue;

		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// ���� ������ �ӵ��� ����
		float wiggleIntensity = m_pSpeeds[i] * scaleFactor;

		// ���� ���� �߰�
		float wiggleX = ((rand() % 100) / 100.f - 0.5f) * wiggleIntensity;
		float wiggleY = ((rand() % 100) / 100.f - 0.5f) * wiggleIntensity;

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta + wiggleX * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta + wiggleY * fTimeDelta;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_fElapsedTime += fTimeDelta;
	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

_bool CVIBuffer_Instancing::Random_Wiggle_Half_Spread_2D(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	const float scaleFactor = 50.f; // ���� ���� ������ ����

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		float currentX = m_pInstanceVertices[i].vTranslation.x;
		float currentY = m_pInstanceVertices[i].vTranslation.y;

		float pivotX = m_vPivotPos.x;
		float pivotY = m_vPivotPos.y;

		float dirX = currentX - pivotX;
		float dirY = currentY - pivotY;

		// ���� ����ȭ
		float length = sqrt(dirX * dirX + dirY * dirY);
		if (length != 0.f)
		{
			dirX /= length;
			dirY /= length;
		}

		// ���� ������ �ӵ��� ����
		float wiggleIntensity = m_pSpeeds[i] * scaleFactor;

		// ���� ���� �߰�
		float wiggleX = ((rand() % 100) / 100.f - 0.5f) * wiggleIntensity;
		float wiggleY = ((rand() % 100) / 100.f - 0.5f) * wiggleIntensity;

		// �̵� �ӵ� ���
		float moveX = dirX * m_pSpeeds[i] * fTimeDelta + wiggleX * fTimeDelta;
		float moveY = dirY * m_pSpeeds[i] * fTimeDelta + wiggleY * fTimeDelta;

		if (moveX < 0.f)
			moveX = -moveX;

		// ��ƼŬ�� ���� ��ġ ������Ʈ
		pMatrices[i].vTranslation.x += moveX;
		pMatrices[i].vTranslation.y += moveY;

		// ���� �ð� ������Ʈ
		pMatrices[i].vLifeTime.y += fTimeDelta;

		// ���� ó��
		if (m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			pMatrices[i].vTranslation.x = m_pInstanceVertices[i].vTranslation.x;
			pMatrices[i].vTranslation.y = m_pInstanceVertices[i].vTranslation.y;
			pMatrices[i].vLifeTime.y = 0.f;
		}
		else if (!m_isLoop && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			m_pContext->Unmap(m_pVBInstance, 0);
			return true;
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
	return false;
}

void CVIBuffer_Instancing::Drop(_float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE		MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		// _vector		vMoveDir = XMVectorSet(Get_Random(0.f, 2.f) - 1.f, -1.f, Get_Random(0.f, 2.f) - 1.f, 0.f);
		_vector		vMoveDir = XMVectorSet(0.0f, -1.f, 0.0f, 0.f);

		XMStoreFloat4(&pMatrices[i].vTranslation,
			XMLoadFloat4(&pMatrices[i].vTranslation) + vMoveDir * m_pSpeeds[i] * fTimeDelta);

		pMatrices[i].vLifeTime.y += fTimeDelta;
		if (m_isLoop == true && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			pMatrices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);

}

void CVIBuffer_Instancing::MoveDir(_vector vDir, _float fTimeDelta)
{
	D3D11_MAPPED_SUBRESOURCE        MappedSubResource{};

	m_pContext->Map(m_pVBInstance, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &MappedSubResource);

	VTXINSTANCE* pMatrices = static_cast<VTXINSTANCE*>(MappedSubResource.pData);

	for (size_t i = 0; i < m_iNumInstance; i++)
	{
		_vector        vMoveDir = vDir;

		XMStoreFloat4(&pMatrices[i].vTranslation,
			XMLoadFloat4(&pMatrices[i].vTranslation) + vMoveDir * m_pSpeeds[i] * fTimeDelta);

		pMatrices[i].vLifeTime.y += fTimeDelta;
		if (m_isLoop == true && pMatrices[i].vLifeTime.y >= pMatrices[i].vLifeTime.x)
		{
			pMatrices[i].vTranslation = m_pInstanceVertices[i].vTranslation;
			pMatrices[i].vLifeTime.y = 0.f;
		}
	}

	m_pContext->Unmap(m_pVBInstance, 0);
}

_float CVIBuffer_Instancing::Get_RandomNormalize()
{
	return (_float)rand() / RAND_MAX;
}

_float CVIBuffer_Instancing::Get_Random(_float fMin, _float fMax)
{
	return fMin + (fMax - fMin) * Get_RandomNormalize();
}

void CVIBuffer_Instancing::Free()
{
	__super::Free();

	Safe_Release(m_pVBInstance);

	if (false == m_isCloned)
	{
		Safe_Delete_Array(m_pInstanceVertices);
		Safe_Delete_Array(m_pSpeeds);
	}
}

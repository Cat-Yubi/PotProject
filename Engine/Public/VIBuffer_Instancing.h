#pragma once
#pragma once

/* �ν��Ͻ��� ���� �׷����ϴ� �𵨵��� ��ӹ޾ƾ��ϴ� Ŭ���� .*/
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instancing abstract : public CVIBuffer
{
public:
	struct VIBUFFER_INSTANCE_DESC
	{
		_uint		iNumInstance = { 0 };
		_float3		vRange = {};
		_float3		vCenter = {};
		_float3		vPivot = {};
		_float2		vSpeed = {};
		_float2		vScale = {};

		_float2		vLifeTime = {};
		_bool		isLoop = {};

	};

protected:
	CVIBuffer_Instancing(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Instancing(const CVIBuffer_Instancing& Prototype);
	virtual ~CVIBuffer_Instancing() = default;

public:
	virtual HRESULT Initialize_Prototype(const VIBUFFER_INSTANCE_DESC* pInitialDesc);
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Bind_Buffers() override;
	virtual HRESULT Render() override;

public:
	_bool Spread(_float fTimeDelta);
	_bool Spread_2D(_float fTimeDelta);
	_bool Spread_FireCracker_2D(_float fTimeDelta);
	_bool Half_Spread_X_2D(_float fTimeDelta);
	_bool Half_Spread_Y_2D(_float fTimeDelta);
	_bool Spiral_Spread_2D(_float fTimeDelta);
	_bool Circular_Spread_2D(_float fTimeDelta);
	_bool Elliptical_Spread_2D(_float fTimeDelta);
	_bool Random_Wiggle_Spread_2D(_float fTimeDelta);
	_bool Random_Wiggle_Half_Spread_2D(_float fTimeDelta);
	void Drop(_float fTimeDelta);
	void MoveDir(_vector vDir, _float fTimeDelta);

	_bool FocusPoint(_float fTimeDelta);
	void Particle_Initialize();
protected:
	/* ��(Instance�ϳ�)�� ������� ���� ��������� ������ ���� ����.  */
	ID3D11Buffer*			m_pVBInstance = { nullptr };

	/* ���� �������ۿ� �߰��� ���� �ϳ��� ũ��. */
	_uint					m_iInstanceVertexStride = { 0 };

	/* �ν��Ͻ��� ���� ��� ��(�ν��Ͻ�)�� �׷������̴�. */
	_uint					m_iNumInstance = { 0 };
	_float3					m_vPivotPos = {};
	_float*					m_pSpeeds = { nullptr };
	_bool					m_isLoop = {  };
	_float					m_fMaxTime = {};
	_float					m_fElapsedTime = {};
	_uint					m_iNumIndexPerInstance = { 0 };

	VTXINSTANCE*			m_pInstanceVertices = { nullptr };

protected:
	HRESULT Create_InstanceBuffer();

protected:
	_float Get_RandomNormalize();
	_float Get_Random(_float fMin, _float fMax);


public:
	virtual CComponent* Clone(void* pArg) =0 ;
	virtual void Free() override;
};

END
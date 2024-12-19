#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CLobby_Goku final : public CGameObject
{
private:
	CLobby_Goku(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLobby_Goku(const CLobby_Goku& Prototype);
	virtual ~CLobby_Goku() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	void RotateTowardsTarget(const _float3& vTargetDir, _float fTimeDelta);
	void MoveForward(_float fTimeDelta);
	void Entry_Level();

	void Set_CharacterInfo();

	//�ش� ���̾� ������Ʈ���� �Ÿ� üũ
	_float ObjectDistance(_wstring strLayerTag ,_uint iLayerIndex = 0);

	//�������� ��ȣ�ۿ�
	void Talk_Frieza(_float fEnableDistance);
	_bool m_bOnMessageBox = { FALSE };

	//�ɾ��� �� ���� ����Ʈ
	void CreateRunDustEffect(_bool bOnInput, _float fCreateDuration,_float fTimeDelta);

	_float CreateDustTimer = { 0.f };

	// �ִϸ��̼� ���� ����
	_bool m_bDashTriggered = false; // �뽬 �ִϸ��̼��� Ʈ���ŵǾ����� ����

	// ���� Ű ���� ����
	_bool m_bPrevKeyUp = false;
	_bool m_bPrevKeyDown = false;
	_bool m_bPrevKeyLeft = false;
	_bool m_bPrevKeyRight = false;

	// �߼Ҹ� Ÿ�̸�
	_float m_fFootstepTimer = 0.f;
	// �߼Ҹ� ��� ���� (��)
	_float m_fFootstepInterval = 0.3f; // ��: 0.5�ʸ��� �߼Ҹ� ���

private:
	//�׸���
	_bool					m_isShadow = { true };
	_uint					m_iPassIndex = { 0 };

public:
	static CLobby_Goku* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
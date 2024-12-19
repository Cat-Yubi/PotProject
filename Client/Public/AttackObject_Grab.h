#pragma once

//#include "GameObject.h"
//#include "Client_Defines.h"
//#include "AnimationEvent_Defines.h"
#include "AttackObject.h"



BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)



class CAttackObject_Grab final : public CAttackObject
{
public:
	//enum HitMotion { HIT_LIGHT, HIT_MEDIUM, HIT_HEAVY, HIT_CROUCH_MEDIUM, HIT_KNOCK_AWAY_LEFT, HIT_KNOCK_AWAY_UP };

	typedef struct ATTACK_Grab_DESC : public  CAttackObject::ATTACK_DESC
	{
		//_uint m_iOnwerNextAnimationIndex   �� �̹� ����  m_bOwnerNextAnimation �� ����

		//�ǰ����� �ִϸ��̼�? 
	
		
		//������ ���� �Ÿ� ����
		_float2 fDistance = {};

		//��� �ִϸ��̼� index, position
		//_ushort iGrabAnimationIndex = {};    == //m_iOnwerNextAnimationIndex

		_float fGrabAnimationPosition={};


		//Set_Animation���� Set_NextAnimation���� bool��?

	};
private:
	CAttackObject_Grab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAttackObject_Grab(const CAttackObject_Grab& Prototype);
	virtual ~CAttackObject_Grab() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;


public:
	//void Set_RemoteDestory();

public:
	virtual void OnCollisionEnter(class CCollider* other, _float fTimeDelta) override;
	virtual void OnCollisionStay(class CCollider* other, _float fTimeDelta) override;
	virtual void OnCollisionExit(class CCollider* other) override;


private:
	void CollisingAttack();
	void CollisingPlayer();

	//ī�޶�
	void Camera_Grab(class CCharacter* pOwner, class CCharacter* pHitOwner);
	void Camera_Same_Grab(class CCharacter* pOwner, class CCharacter* pHitOwner);

private:

	_float2 m_fDistance = {};
	//_ushort m_iGrabAnimationIndex = {};
	_float m_fGrabAnimationPosition = {};


	
	//ĳ���Ͱ� �������־���ϴ°�?
	//_ushort m_iLoofCycle = 3;


public:
	static CAttackObject_Grab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
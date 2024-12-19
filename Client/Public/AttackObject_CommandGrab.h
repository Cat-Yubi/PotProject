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



class CAttackObject_CommandGrab final : public CAttackObject
{
public:
	//enum HitMotion { HIT_LIGHT, HIT_MEDIUM, HIT_HEAVY, HIT_CROUCH_MEDIUM, HIT_KNOCK_AWAY_LEFT, HIT_KNOCK_AWAY_UP };

	typedef struct ATTACK_COMMANDGRAB_DESC : public  CAttackObject::ATTACK_DESC
	{
		_float2 fDistance = {100.f,100.f};


		_float fGrabAnimationPosition={};

		//�ִϸ��̼��� �������� ���дϱ� üũ��
		_short iGrabAnimationIndex = {};

		_bool bForcedHit = false;
		//_short iOnwerDirection = {231};  //���� ���� ������ �ݴ밡 �ǵ��� ����������  0���� ������ ����� ������ �������� ����. �⺻��(231)�̸� �ڵ�ó���ϱ�


		

		//_float fCameraShakeDuration = {200};
		//_float fCameraShakeMagnitude = {200};

		_bool bGrabedGravity = { false };

	};
private:
	CAttackObject_CommandGrab(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAttackObject_CommandGrab(const CAttackObject_CommandGrab& Prototype);
	virtual ~CAttackObject_CommandGrab() = default;

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

private:

	_float2 m_fDistance = {};
	_float m_fGrabAnimationPosition = {};
	_ushort m_iGrabAnimationIndex = {};

	_bool m_bForcedHit = { false };

	

	_bool m_bGrabedGravity = { false };

	//ĳ���Ͱ� �������־���ϴ°�?
	//_ushort m_iLoofCycle = 3;


public:
	static CAttackObject_CommandGrab* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
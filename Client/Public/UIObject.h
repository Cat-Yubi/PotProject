#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

#include "GameInstance.h"
#include "UI_Manager.h"

BEGIN(Engine)

class CVIBuffer_Rect;
class CTexture;
class CShader;

END

BEGIN(Client)

class CUIObject abstract : public CGameObject
{
public:
	enum UI_LRPOS  { LEFT , RIGHT  , POS_END };
	enum UI_TYPE { UI_ANIM, UI_NONANIM, TYPE_END };

public:
	typedef struct :public CGameObject::GAMEOBJECT_DESC
	{
		UI_LRPOS eLRPos = {};
		
		UI_TYPE	eType = {};
		_uint iNumUI = {};
		_uint iTotalNum = {};
	}UI_DESC;

protected:
	CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject(const CUIObject& Prototype);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Camera_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render(_float fTimeDelta);

protected:
	virtual HRESULT Bind_ShaderResources();
	virtual HRESULT Ready_Components();

	//UI �ʱ� ���� ��
	virtual void Set_UI_Setting(_float fSizeX, _float fSizeY, _float fPosX, _float fPosY, _float fDepth = 0.9f);
	
//UI ũ��� ��ġ�� ������ؼ� ã��
	void DebugTesting(_float fSizeOffset,  _float fDepth = 1.f);

//Ŭ���� ��ġ�� �ش� UI ��ġ��� ����
	_bool ClickRange(_float fPickPosX, _float fPickPosY);

public:
	//UI ���� / ������
	void IsShow(_bool IsShiw) {m_bIsActive = IsShiw;}

private:
	//�� �����Ӹ��� MainPlayer�� Sub�÷��̾� ������ �� ���� 
	void InitPlayer();

private:
	void Add_UIList(CUI_Define::UI_LISTPOS ePos);
	void UI_PosArea(_float fAreaPosY);

protected:
	//���� ��ġ���� ���� X,Y������ ��ȯ
	_vector GetOffsetPostion(_vector vPosition);

	_bool MoveAnimUI(_vector vTargetPos, _float fSpeed,  _float fDepth , _float fTimeDelta , _float fEndDistance = 15.f);
	_bool Animation(_vector vStartPos ,_vector vTargetPos, _float fSpeed , _float fDepth, _float fTimeDelta,_float fEndDistance = 15.f);

protected:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

protected:
	_bool m_bCharaStun = { FALSE };
	_bool m_bHit = { FALSE };

	UI_LRPOS m_eLRPos = { POS_END };
	UI_DESC* pDesc = {};


//UI ������ , ũ�� üŷ 
	_float m_fSizeX = { 100.f };
	_float m_fSizeY = { 100.f };
	_float m_fPosX = { 0.f };
	_float m_fPosY = { 0.f };

	_float4x4 m_ViewMatrix = {};
	_float4x4 m_ProjMatrix = {};

	class CCharacter* m_pMainPawn = { nullptr };
	class CCharacter* m_pSubPawn = { nullptr };

	CUI_Manager* m_pUI_Manager = { nullptr };

protected:
//Animation
	_bool m_bStart = { FALSE };
	_bool m_bAnimEnd = { FALSE };
	_vector m_vAnimStartPos = {};
	_vector m_vAnimTargetPos = {};
	_float m_fAnimDelayTiemr = { 0.f };

	_bool m_bCheck = { FALSE };

protected:
	//�ػ� ����
	_float2 m_vOffSetWinSize = { };
	_float2 m_vPrevWinSize = {};

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

END


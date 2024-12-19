#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
END

BEGIN(Client)

class CQTE_Same_Grab_UI_Icon final : public CGameObject
{
public:
	struct QTE_Same_Grab_UI_ICON_DESC
	{
		_float	fSizeX{}, fSizeY{}, fX{}, fY{}, fAlpha{}, iTextureNumber{}, fFallDelay{};
		_bool isFirst = false;
		_bool isLast = false;
		CGameObject* SameGrab = { nullptr };
	};

	enum IconState
	{
		NOT_SELECTED,        // �⺻ ����
		FALLING,             // �������� ���� ���� (���� �߰�)
		SELECTED,            // ���� ���õ� ����
		ALREADY_PRESSED,     // �ùٸ��� ���� ����
		WRONG_PRESSED,        // �߸� ���� ����
		ASCEND,				 // �ö󰡱�
	};

private:
	CQTE_Same_Grab_UI_Icon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CQTE_Same_Grab_UI_Icon(const CQTE_Same_Grab_UI_Icon& Prototype);
	virtual ~CQTE_Same_Grab_UI_Icon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;

	void Set_State(IconState state);
	void Set_AscendDelay(_float delay) { m_fAscendDelay = delay; };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	void Update_Falling(_float fTimeDelta);
	void Update_Selected(_float fTimeDelta);
	void Update_AlreadyPressed(_float fTimeDelta);
	void Update_WrongPressed(_float fTimeDelta);
	void Update_Ascend(_float fTimeDelta);
	void Update_NotSelected(_float fTimeDelta);

public:
	_float m_fX{}, m_fY{};
private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	_float					m_fSizeX{}, m_fSizeY{},  m_fAlpha{};
	_float4x4				m_ViewMatrix{}, m_ProjMatrix{};
	_int m_iTextureNumber = {};

	_bool m_bIsFirst = { false };
	_bool m_bIsLast = { false };
	IconState m_State = NOT_SELECTED; // ������ ����

	// �ִϸ��̼� ���� ����
	_float m_fTargetY = {};          // ��ǥ Y ��ġ
	_float m_fCurrentY = {};         // ���� Y ��ġ
	_float m_fVelocityY = {};        // Y �ӵ� (�ε巯�� �̵��� ���� ����)
	const _float m_fDefault_Y = 300.f;   // �⺻ Y ��ġ
	const _float m_fSelected_Y = 250.f;  // ���õ� ������ Y ��ġ
	const _float m_fSmoothing_Time = 0.05f; // �ִϸ��̼� �ӵ� ���� (�ʿ信 ���� ����)

	// ��鸲 �ִϸ��̼� ����
	_float m_fShakeDuration = { 0.5f };    // ��鸲 ���� �ð�
	_float m_fShakeTime = {};        // ���� ��鸲 �ð�
	_float m_fShakeAmplitude = { 20.f };   // ��鸲 ����
	_float m_fShakeFrequency = { 25.f };   // ��鸲 ���ļ�
	_bool m_bIsShaking = { false };         // ��鸲 ����

	// ���ο� �ִϸ��̼� ����
	_float m_fStartY = -100.f;          // �ʱ� Y ��ġ
	_float m_fAnimationDuration = 0.7f; // �ִϸ��̼� ���� �ð�
	_float m_fElapsedTime = 0.f;        // ��� �ð�
	_bool m_bIsFalling = false;         // �������� ������ ����
	_float m_fFallDelay = 0.f;          // �������� �����ϴ� ���� �ð�

	// ASCEND ���� ���� ����
	_bool m_bIsAscending;       // �ö󰡴� ������ ����
	_float m_fAscendDelay;      // �ö󰡱� �����ϴ� ���� �ð�

	class CGameObject* m_pSameGrab = { nullptr };

public:
	static CQTE_Same_Grab_UI_Icon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
#pragma once

#include "Client_Defines.h"
#include "AnimationEvent_Defines.h"
#include "FrameEvent_Manager.h"
#include "Character.h"

#include "Frieza_MeleeAttack.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CPlay_Frieza final : public CCharacter
{
public:
	typedef struct : CGameObject::GAMEOBJECT_DESC
	{
		_wstring strModelName;
	}Play_Frieza_DESC;
	enum AnimationIndex
	{
		//Light - �����
		//Meaduim - �߰���
		//Heavy - ������
		//Special - ��ź (���Ÿ� ����)
		//Air - 1,2,3 ���߰���
		//Crouch - ���� ���� ����
		//236 ���� ���ִ°� ��Ű�� ����Ű�� ������ ����
		//Final - �����ʻ��
		ANIME_ATTACK_LIGHT1 = 41, ANIME_ATTACK_LIGHT2 = 42, ANIME_ATTACK_LIGHT3 = 43,
		ANIME_ATTACK_MEDIUM = 44, ANIME_ATTACK_HEAVY = 45,
		ANIME_ATTACK_SPECIAL = 46,

		ANIME_ATTACK_AIR1 = 51, ANIME_ATTACK_AIR2 = 52, ANIME_ATTACK_AIR3 = 53, ANIME_ATTACK_SPECIAL_AIR = 54, ANIME_ATTACK_SPECIAL_AIR2 = 55,

		ANIME_ATTACK_CROUCH_LIGHT = 47, ANIME_ATTACK_CROUCH_MEDUIM = 48, ANIME_ATTACK_CROUCH_HEAVY = 49, ANIME_ATTACK_CROUCH_SPECIAL = 50,
		ANIME_ATTACK_CROUCH_CROUCH_SPECIAL = 56,


		ANIME_SPARKING = 58,
		ANIME_REFLECT = 57,


		//�ó׸�ƽ
		ANIME_GOKU_CINEMATIC_01 = 86, //FRN801cs_01  //������ ����
		ANIME_GOKU_CINEMATIC_02 = 87, //FRN801cs_02 //���� ���� 
		ANIME_GOKU_CINEMATIC_03 = 88,  //�¸� ��


		ANIME_GRAB = 59, //304 60 �巡�﷯��    duration 40~130�� ����  23���� 275�� �̵���  5~20�뿡�� ���������ϰ� ������ 40���� �̵��ϱ�


		ANIME_ATTACK_236 = 62,
		ANIME_ATTACK_236_SPECIAL = 63,
		ANIME_ATTACK_214 = 64,  //214 Special�� �̰ɷ� ����. �������
		ANIME_ULTIMATE = 65, //1��


		ANIME_214_FINAL = 66,
		ANIME_214_FINAL_2 = 67,


		ANIME_WAKEUP_FINAL = 68,
		ANIME_TRANSFORM_FINAL = 69,
		ANIME_TRANSFORM_FINAL_CUTSCENE = 70,






		ANIME_IDLE = 0,
		ANIME_JUMP_UP = 5, ANIME_JUMP_DOWN = 6,
		ANIME_FORWARD_WALK = 8, ANIME_BACK_WALK = 9,
		ANIME_FORWARD_DASH = 10, ANIME_BACK_DASH = 11,
		ANIME_FORWARD_DASH_END = 13, ANIME_BACK_DASH_END = 14,
		ANIME_CROUCH_START = 2, ANIME_CROUCHING = 3,
		ANIME_CHASE = 12,


		//����
		ANIME_GUARD_GROUND = 17,	//040
		ANIME_GUARD_CROUCH = 18,	//041
		ANIME_GUARD_AIR = 19,		//042



		//��
		ANIME_HIT_LIGHT = 20, ANIME_HIT_LIGHT_FRONT = 21, ANIME_HIT_CROUCH = 22,			//050   051 052




		//����
		ANIE_HIT_AIR = 23, ANIME_HIT_FALL = 25,									//060 061


		//���� ��� ����
		ANIME_HIT_BOUND_DOWN = 24,   // WakeUp�ʻ��� ������ �߰��ܰ�


		ANIME_LAYUP = 26,
		ANIME_DIE_STAND = 27,


		//�ǰ�
		ANIME_HIT_HEAVY_AWAY_SPIN_UP = 28,
		ANIME_HIT_HEAVY_AWAY_SPIN_LEFTUP = 29,
		ANIME_HIT_HEAVY_AWAY_SPIN_LEFT = 30,

		ANIME_HIT_HEAVY_AWAY_LEFT = 31,
		ANIME_HIT_HEAVY_AWAY_UP = 33, //077      ���� 061(26��) ���� ����� 
		ANIME_HIT_HEAVY_AWAY_LEFTDOWN = 37, //081


		ANIME_HIT_WALLBOUNCE = 32,				//076.  �� ���� 062(26�� ANIME_HIT_FALL_UPPER)�� �����

		ANIME_BREAK_FALL_GROUND = 39, // 100
		ANIME_BREAK_FALL_AIR = 40, // 101

		ANIME_GRAB_READY = 16,  //038 �⺻��� �غ��ڼ�.  (����)   �������� ������� �ʰ� ��� �غ�θ� ���,  NextPosition 5



		//���� 600cs
		ANIME_START_DEFAULT = 71,

		//�¸� 610cs
		ANIME_WIN_DEFAULT = 74,

		//���庯��
		ANIME_NEWROUND_RIGHTHAND_APEEAR_CUTSCENE = 76, //620 cs.   ->630�� ����
		ANIME_NEWROUND_LEFTHAND_CUTSCENE = 77, //621 cs ->631���� ���� Durtaion 122

		ANIME_NEWROUND_RIGHTHAND = 78,  //630.   
		ANIME_NEWROUND_LEFTHAND = 79,  //631.    Durtaion 24


	};
	enum AnimationCount
	{
		COUNT_ATTACK_MEDIUM = 0,
		COUNT_ATTACK_CROUCH_MEDUIM,
		COUNT_ATTACK_SPECIAL,
		COUNT_ATTACK_GRAB,
		COUNT_ATTACK_BENISHING,
		COUNT_ATTACK_CROUCH_SPECIAL,
		COUNT_END
	};

private:
	CPlay_Frieza(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlay_Frieza(const CPlay_Frieza& Prototype);
	virtual ~CPlay_Frieza() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Player_Update(_float fTimeDelta) override;
	virtual void Camera_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render(_float fTimeDelta) override;
	virtual HRESULT Shadow_Render(_float fTimeDelta) override;
	
	//virtual void NextMoveCheck() override;
	//virtual void AttackNextMoveCheck() override;
	//virtual void AnimeEndNextMoveCheck() override;

	//virtual void Test_InputCommand();
	//virtual void Set_Animation(_uint iAnimationIndex) override;
	//virtual void Set_Animation(_uint iAnimationIndex, _bool bloof = false);

	void KeyTest();


	virtual _bool Check_bCurAnimationisGroundMove(_uint iAnimation = 1000) override;
	virtual _bool Check_bCurAnimationisAttack(_uint iAnimation = 1000) override;
	virtual _bool Check_bCurAnimationisAirAttack(_uint iAnimation = 1000)override;
	virtual _bool Check_bCurAnimationisHalfGravityStop(_uint iAnimation = 1000) override;


	virtual _short Check_bCurAnimationisCanChase() override;


	_bool* Get_pbAttackCount() { return m_bAttackCount; };
	_ushort* Get_piSpecialCount() { return &m_iCountGroundSpecial; };
	_ushort* Get_piSAirpecialCount() { return &m_iCountAirSpecial; };

	virtual void Reset_AttackCount() override;

	virtual void Gravity(_float fTimeDelta) override;

	virtual void AttackEvent(_int iAttackEvent, _int AddEvent = 0)override;
	virtual void Play_Sound(_uint SoundName, _bool bisLoof, _float fvolume)override;
	virtual void Play_Group_Sound(_uint groupKey, _bool loop, _float volume)override;
	virtual void Play_Sound_Stop(_uint SoundName)override;


	virtual _float Get_DamageScale(_bool bUltimate = false) override;
	virtual void Set_BreakFall_Ground() override;

	void Add_YellowLight();
	void Add_BlueLight();


	void Update214ReturnEvent(_float fTimeDelta);

	_bool Get_bGoldenFrieza();

	virtual void Character_CinematicEnd() override;

private:
	//CModel* m_pModelCom_Opening = { nullptr };
	CModel* m_pModelCom_Skill = { nullptr };
	CTexture* m_pGold_BaseTexture = { nullptr };
	CTexture* m_pGold_OutLineTexture = { nullptr };
	CTexture* m_pMetalTexture = { nullptr };

	CFrieza_MeleeAttack m_tAttackMap;

	_bool m_bAttackCount[COUNT_END] = { true };
	_ushort m_iCountGroundSpecial = 0;
	_ushort m_iCountAirSpecial = 0;


	_float m_fAcc214AttackTime = {};
	_bool m_bAcc214Attack = false;
	_short m_i214AttackPreviousDirection = { 0 };


	_bool m_bGoldFrieza = false;

	list<CEffect_Layer*>m_DeleteLayer;

	CEffect_Layer* m_p236SpecialEffect = { nullptr };
	CEffect_Layer* m_p236UltimateEffect = { nullptr };

	CEffect_Layer* m_p214FinalEffect = { nullptr };
	CEffect_Layer* m_pAttackHeavyEffect = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	_bool m_bSoloFinalEndCount = false;

public:
	static CPlay_Frieza* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
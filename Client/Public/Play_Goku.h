#pragma once

#include "Client_Defines.h"
#include "AnimationEvent_Defines.h"
#include "FrameEvent_Manager.h"
#include "Character.h"

#include "Goku_MeleeAttack.h"

BEGIN(Engine)
class CShader;
class CModel;
END

BEGIN(Client)

class CPlay_Goku final : public CCharacter
{
public:
	typedef struct : CGameObject::GAMEOBJECT_DESC
	{
		_wstring strModelName;
	}Play_Goku_DESC;
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
		ANIME_ATTACK_LIGHT1 = 43, ANIME_ATTACK_LIGHT2 = 44, ANIME_ATTACK_LIGHT3 = 47,
		ANIME_ATTACK_MEDIUM = 46, ANIME_ATTACK_HEAVY = 45,
		ANIME_ATTACK_SPECIAL = 48,

		ANIME_ATTACK_AIR1 = 52, ANIME_ATTACK_AIR2 = 53, ANIME_ATTACK_AIR3 = 54, ANIME_ATTACK_SPECIAL_AIR = 57,

		ANIME_ATTACK_CROUCH_LIGHT = 49, ANIME_ATTACK_CROUCH_MEDUIM = 50, ANIME_ATTACK_CROUCH_HEAVY = 51, ANIME_ATTACK_UPPER_AIR = 55,


		ANIME_ATTACK_236 = 66, ANIME_ATTACK_236_AIR = 67,
		ANIME_ATTACK_214 = 68,

		ANIME_ATTACK_236_SPECIAL = 62, ANIME_ATTACK_236_SPECIAL_UPSIDE = 63,
		ANIME_ATTACK_236_SPECIAL_AIR = 64, ANIME_ATTACK_236_SPECIAL_AIR_DOWNSIDE = 65,

		ANIME_FINAL_START = 69,
		ANIME_FINAL_ELBO = 72,
		ANIME_FINAL_UPPER = 74,
		ANIME_FINAL_SMASH = 73,
		ANIME_FINAL_KAMEHAMEHA = 70,


		ANIME_IDLE = 0,
		ANIME_FORWARD_WALK = 9, ANIME_BACK_WALK = 10,
		ANIME_FORWARD_DASH = 11, ANIME_BACK_DASH = 12,
		ANIME_FORWARD_DASH_END = 14, ANIME_BACK_DASH_END = 15,


		ANIME_CROUCH_START = 3, ANIME_CROUCHING = 4,
		ANIME_JUMP_UP = 6, ANIME_JUMP_DOWN = 7,

		ANIME_CHASE = 13,


		//��
		ANIME_HIT_LIGHT = 21, ANIME_HIT_LIGHT_FRONT = 22, ANIME_HIT_CROUCH = 23,			//050   051 052


		//����
		ANIE_HIT_AIR = 24, ANIME_HIT_FALL = 26,									//060 061


		//���� ��� ����
		ANIME_HIT_BOUND_DOWN = 25,

		//�ǰ�
		ANIME_HIT_HEAVY_AWAY_LEFT = 33,
		ANIME_HIT_HEAVY_AWAY_UP = 35, //077      ���� 061(26��) ���� ����� 
		ANIME_HIT_HEAVY_AWAY_LEFTDOWN = 39, //081
		ANIME_HIT_HEAVY_AWAY_SPIN_UP = 30,
		ANIME_HIT_HEAVY_AWAY_SPIN_LEFTUP = 31,
		ANIME_HIT_HEAVY_AWAY_SPIN_LEFT = 32,

		ANIME_HIT_WALLBOUNCE = 34,				//076.  �� ���� 062(26�� ANIME_HIT_FALL_UPPER)�� �����


		ANIME_LAYUP = 27,
		ANIME_BREAK_FALL_GROUND = 41, // 100
		ANIME_BREAK_FALL_AIR = 42, // 101


		//28 ??	 MEDIUM ������ �߰��� �¾Ƶ� �̰� �ȳ���
		//078 : HIT_AWAY_LEFTUP

		ANIME_DIE_STAND = 29,


		//����
		ANIME_GUARD_GROUND = 18,	//040
		ANIME_GUARD_CROUCH = 19,	//041
		ANIME_GUARD_AIR = 20,		//042



		ANIME_GRAB_READY = 17,  //038 �⺻��� �غ��ڼ�.  (����)   �������� ������� �ʰ� ��� �غ�θ� ���,  NextPosition 5
		ANIME_GRAB = 60, //304 60 �巡�﷯��    duration 40~130�� ����  23���� 275�� �̵���  5~20�뿡�� ���������ϰ� ������ 40���� �̵��ϱ�
		//ANIME_GRAB_RUSH = 61 // 306cs


		//301 58 �и�
		//306cs 61  ���� �巡�﷯�� ���� �� ����

		ANIME_SPARKING = 59,
		ANIME_REFLECT = 58,


		//���� 600cs
		ANIME_START_DEFAULT = 75,

		//�¸� 610cs
		ANIME_WIN_DEFAULT = 78,

		//���庯��
		ANIME_NEWROUND_RIGHTHAND_APEEAR_CUTSCENE = 79 , //620 cs.   ->630�� ����
		ANIME_NEWROUND_LEFTHAND_CUTSCENE = 80 , //621 cs ->631���� ���� Durtaion 122

		ANIME_NEWROUND_RIGHTHAND = 81,  //630.   
		ANIME_NEWROUND_LEFTHAND = 82,  //631.    Durtaion 24


		//�ó׸�ƽ
		ANIME_GOKU_CINEMATIC_01 = 86, //FRN801cs_01  //������ ����
		ANIME_GOKU_CINEMATIC_02 = 87, //FRN801cs_02 //���� ���� 
		ANIME_GOKU_CINEMATIC_03 = 88,  //�¸� ��

	};
	enum AnimationCount
	{
		COUNT_ATTACK_MEDIUM = 0,
		COUNT_ATTACK_CROUCH_MEDUIM,
		COUNT_ATTACK_SPECIAL,
		COUNT_ATTACK_GRAB,
		COUNT_ATTACK_BENISHING,
		COUNT_END
	};

private:
	CPlay_Goku(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlay_Goku(const CPlay_Goku& Prototype);
	virtual ~CPlay_Goku() = default;

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
	virtual void Reset_AttackCount() override;

	virtual void Gravity(_float fTimeDelta) override;

	virtual void AttackEvent(_int iAttackEvent, _int AddEvent = 0)override;
	virtual void Play_Sound(_uint SoundName, _bool bisLoof, _float fvolume)override;
	virtual void Play_Group_Sound(_uint groupKey, _bool loop, _float volume)override;
	virtual void Play_Sound_Stop(_uint SoundName)override;

	void Set_UltimateKamehameha(_bool bUltimate);

	void Add_YellowLight();
	void Add_BlueLight();
	_float Get_DamageScale(_bool bUltimate = false) override;

	virtual void Character_CinematicEnd() override;


	virtual void HitStopEffect() override;

private:
	CModel* m_pModelCom_Opening = { nullptr };
	CModel* m_pModelCom_Skill = { nullptr };
	//CModel* m_pBlackGokuModelCom = { nullptr };
	
	CTexture* m_pOpeningTextureCom = { nullptr };
	CTexture* m_pOpeningDecalTextureCom = { nullptr };
	CTexture* m_pOpeningOutLineTextureCom = { nullptr };
	CGoku_MeleeAttack m_tAttackMap;

	_bool m_bAttackCount[COUNT_END] = { true };
	_ushort m_iCountGroundSpecial = 0;

	_bool m_bUltimateKamehameha = false;

	_bool m_bFinalSkillss3 = { false };
	_bool m_bAlwaysss3Test = { false };


	_bool m_bNormalGoku = { true }; //true

	CEffect_Layer* m_pFinalAura = { nullptr };
	CEffect_Layer* m_p236ChaseAura = { nullptr };

	
private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();


public:
	static CPlay_Goku* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END
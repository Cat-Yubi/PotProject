#include "..\Public\Sound_Manager.h"
#include "Layer.h"
#include "GameObject.h"
#include <locale>
#include <codecvt>

CSound_Manager::CSound_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{
}

CSound_Manager::CSound_Manager(const CSound_Manager& Prototype)
	: CGameObject(Prototype)
{
}


HRESULT CSound_Manager::Initialize_Prototype()
{
	// FMOD �ý��� �ʱ�ȭ
	FMOD_System_Create(&m_pSoundSystem, FMOD_VERSION);
	FMOD_System_Init(m_pSoundSystem, MAX_CHANNELS, FMOD_INIT_NORMAL | FMOD_INIT_STREAM_FROM_UPDATE, nullptr);

	// ä�� �׷� ����
	FMOD_System_CreateChannelGroup(m_pSoundSystem, nullptr, &m_pChannelGroup);

	// ī�װ��� �ʱ� ���� ���� (�⺻��: 1.0f)
	m_categoryVolumes[SOUND_CATEGORY::BGM] = 1.0f;
	m_categoryVolumes[SOUND_CATEGORY::VOICE] = 1.0f;
	m_categoryVolumes[SOUND_CATEGORY::SFX] = 1.0f;


	return S_OK;
}

HRESULT CSound_Manager::Initialize(void* pArg)
{
	return S_OK;
}

void CSound_Manager::Camera_Update(_float fTimeDelta)
{

}

void CSound_Manager::Update(_float fTimeDelta)
{
	FMOD_System_Update(m_pSoundSystem);

	// ��� �Ϸ�� ä�� ����
	int playingChannels = 0;
	FMOD_ChannelGroup_GetNumChannels(m_pChannelGroup, &playingChannels);

	for (int i = 0; i < playingChannels; ++i)
	{
		FMOD_CHANNEL* tempChannel = nullptr;
		if (FMOD_ChannelGroup_GetChannel(m_pChannelGroup, i, &tempChannel) == FMOD_OK)
		{
			FMOD_BOOL isPlaying = false;
			if (FMOD_Channel_IsPlaying(tempChannel, &isPlaying) == FMOD_OK && !isPlaying)
			{
				FMOD_Channel_Stop(tempChannel);
			}
		}
	}
}

void CSound_Manager::Late_Update(_float fTimeDelta)
{

}

HRESULT CSound_Manager::Render(_float fTimeDelta)
{
	return S_OK;
}

void CSound_Manager::Register_Sound(const std::wstring& filePath, SOUND_KEY_NAME alias, SOUND_CATEGORY category, _bool loop, _bool isNonOverlapping)
{
	// �̹� ��ϵ� �������� Ȯ��
	if (m_soundMap.find(alias) != m_soundMap.end())
	{
		return; // �̹� �����ϹǷ� ������� �ʰ� ��ȯ
	}

	FMOD_SOUND* sound;

	// wstring�� string���� ��ȯ
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::string filePathStr = converter.to_bytes(filePath);

	// ��带 �����Ͽ� ���� ����
	FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL | FMOD_CREATESAMPLE : FMOD_LOOP_OFF | FMOD_CREATESAMPLE;
	if (FMOD_System_CreateSound(m_pSoundSystem, filePathStr.c_str(), FMOD_DEFAULT | mode, nullptr, &sound) != FMOD_OK)
	{
		return;
	}

	// ���� ���
	m_soundMap[alias] = sound;
	// ī�װ� �ʿ� �߰�
	m_soundCategoryMap[alias] = category;

	// �ߺ� ��� ���� ����� ���
	if (isNonOverlapping)
	{
		m_nonOverlappingSounds.insert(alias);
	}
}

void CSound_Manager::Register_Sound_Group(SOUND_GROUP_KEY groupKey, const std::wstring& filePath, SOUND_GROUP_KEY_NAME alias, SOUND_CATEGORY category, _bool loop)
{
	// �׷��� �������� ������ ����
	if (m_soundGroupMap.find(groupKey) == m_soundGroupMap.end())
	{
		m_soundGroupMap[groupKey] = std::vector<SOUND_GROUP_KEY_NAME>();
	}

	// �̹� ��ϵ� �������� Ȯ��
	if (m_groupSoundMap.find(alias) == m_groupSoundMap.end())
	{
		// �׷� ���� ���
		FMOD_SOUND* sound;

		// wstring�� string���� ��ȯ
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		std::string filePathStr = converter.to_bytes(filePath);

		// ��带 �����Ͽ� ���� ����
		FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
		if (FMOD_System_CreateSound(m_pSoundSystem, filePathStr.c_str(), FMOD_DEFAULT | mode, nullptr, &sound) != FMOD_OK)
		{
			return;
		}

		// �׷� ���� �ʿ� ���
		m_groupSoundMap[alias] = sound;
		m_groupSoundCategoryMap[alias] = category;
	}

	// �׷쿡 �ش� alias�� �߰�
	m_soundGroupMap[groupKey].push_back(alias);
}

void CSound_Manager::Play_Sound(SOUND_KEY_NAME alias, _bool loop, _float volume)
{
	if (!m_isImguiPlay)
		return;

	auto it = m_soundMap.find(alias);
	if (it == m_soundMap.end()) return;

	// �ߺ� ��� ���� �������� Ȯ��
	_bool isNonOverlapping = (m_nonOverlappingSounds.find(alias) != m_nonOverlappingSounds.end());

	// �ߺ� ��� ���� ������ ���, �̹� ��� ������ Ȯ��
	if (isNonOverlapping)
	{
		auto channelIt = m_channelMap.find(alias);
		if (channelIt != m_channelMap.end())
		{
			FMOD_BOOL isPlaying = false;
			FMOD_Channel_IsPlaying(channelIt->second.channel, &isPlaying);
			if (isPlaying)
			{
				// �̹� ��� ���̹Ƿ� ���ο� ��� ��û ����
				return;
			}
		}
	}


	FMOD_CHANNEL* channel = nullptr;

	// ä�� �� Ȯ��
	int playingChannels = 0;
	FMOD_ChannelGroup_GetNumChannels(m_pChannelGroup, &playingChannels);

	if (playingChannels >= MAX_CHANNELS)
	{
		// ������ ä���� �����ϰ� ����
		FMOD_CHANNEL* oldestChannel = nullptr;
		FMOD_ChannelGroup_GetChannel(m_pChannelGroup, 0, &oldestChannel); // ���÷� ù ��° ä�� ���
		if (oldestChannel)
		{
			FMOD_Channel_Stop(oldestChannel);
		}
	}

	// ī�װ� ���� ��������
	auto categoryIt = m_soundCategoryMap.find(alias);
	float categoryVolume = 1.0f;
	if (categoryIt != m_soundCategoryMap.end())
	{
		categoryVolume = m_categoryVolumes[categoryIt->second];
	}

	float actualVolume = volume * categoryVolume;

	// ���� ��� �� ���� ����
	if (FMOD_System_PlaySound(m_pSoundSystem, it->second, m_pChannelGroup, false, &channel) == FMOD_OK)
	{
		FMOD_Channel_SetVolume(channel, actualVolume);
		m_channelMap[alias] = { channel, volume }; // �ʱ� ���� ����
	}
}

void CSound_Manager::Play_Group_Sound(SOUND_GROUP_KEY groupKey, _bool loop, _float volume)
{
	if (!m_isImguiPlay)
		return;

	// �׷��� �����ϴ��� Ȯ��
	auto groupIt = m_soundGroupMap.find(groupKey);
	if (groupIt == m_soundGroupMap.end()) return;

	auto& soundList = groupIt->second;

	// ���� ����Ʈ�� ����ִ��� Ȯ��
	if (soundList.empty()) return;

	// ���������� ����� ���带 ������ ���� ��� ����
	std::vector<SOUND_GROUP_KEY_NAME> availableSounds;
	for (const auto& soundAlias : soundList)
	{
		if (m_lastPlayedSound[groupKey] != soundAlias)
		{
			availableSounds.push_back(soundAlias);
		}
	}

	// ��� ������ ���尡 ������ ��� ���带 �ٽ� ���
	if (availableSounds.empty())
	{
		availableSounds = soundList;
	}

	// �������� �ϳ��� ���� ����
	int randomIndex = rand() % availableSounds.size();
	SOUND_GROUP_KEY_NAME selectedSoundKey = availableSounds[randomIndex];

	// ������ ���带 �׷� ���� �ʿ��� ã��
	auto soundIt = m_groupSoundMap.find(selectedSoundKey);
	if (soundIt == m_groupSoundMap.end()) return;

	FMOD_SOUND* pSound = soundIt->second;

	FMOD_CHANNEL* channel = nullptr;

	// ä�� �� Ȯ��
	int playingChannels = 0;
	FMOD_ChannelGroup_GetNumChannels(m_pChannelGroup, &playingChannels);

	if (playingChannels >= MAX_CHANNELS)
	{
		// ������ ä���� �����ϰ� ����
		FMOD_CHANNEL* oldestChannel = nullptr;
		FMOD_ChannelGroup_GetChannel(m_pChannelGroup, 0, &oldestChannel);
		if (oldestChannel)
		{
			FMOD_Channel_Stop(oldestChannel);
		}
	}

	// ī�װ� ���� ��������
	auto categoryIt = m_groupSoundCategoryMap.find(selectedSoundKey);
	float categoryVolume = 1.0f;
	if (categoryIt != m_groupSoundCategoryMap.end())
	{
		categoryVolume = m_categoryVolumes[categoryIt->second];
	}

	float actualVolume = volume * categoryVolume;

	// ���� ��� �� ���� ����
	if (FMOD_System_PlaySound(m_pSoundSystem, pSound, m_pChannelGroup, false, &channel) == FMOD_OK)
	{
		FMOD_Channel_SetVolume(channel, actualVolume);
		m_groupChannelMap[selectedSoundKey] = { channel, volume }; // �ʱ� ���� ����
	}

	// ���������� ����� ���� ����
	m_lastPlayedSound[groupKey] = selectedSoundKey;
}

void CSound_Manager::Stop_Sound(SOUND_KEY_NAME alias)
{
	auto it = m_channelMap.find(alias);
	if (it == m_channelMap.end()) return;

	FMOD_Channel_Stop(it->second.channel);
}

void CSound_Manager::Stop_Group_Sound(SOUND_GROUP_KEY groupKey)
{
	// �׷� ã��
	auto groupIt = m_soundGroupMap.find(groupKey);
	if (groupIt == m_soundGroupMap.end()) return;

	// �׷� �� ��� ������ ä���� ����
	const auto& soundList = groupIt->second;
	for (const auto& alias : soundList)
	{
		auto channelIt = m_groupChannelMap.find(alias);
		if (channelIt != m_groupChannelMap.end())
		{
			FMOD_Channel_Stop(channelIt->second.channel);
		}
	}
}

void CSound_Manager::Set_Volume(SOUND_KEY_NAME alias, float volume)
{
	auto it = m_channelMap.find(alias);
	if (it == m_channelMap.end()) return;

	// �ʱ� ���� ������Ʈ
	it->second.baseVolume = volume;

	// ���� ī�װ� ���� ��������
	auto categoryIt = m_soundCategoryMap.find(alias);
	float categoryVolume = 1.0f;
	if (categoryIt != m_soundCategoryMap.end())
	{
		categoryVolume = m_categoryVolumes[categoryIt->second];
	}

	float actualVolume = volume * categoryVolume;
	FMOD_Channel_SetVolume(it->second.channel, actualVolume);
}

void CSound_Manager::Set_Group_Volume(SOUND_GROUP_KEY groupKey, float volume)
{
	// �׷� ã��
	auto groupIt = m_soundGroupMap.find(groupKey);
	if (groupIt == m_soundGroupMap.end()) return;

	// �׷� �� ��� ������ ������ ������Ʈ
	const auto& soundList = groupIt->second;
	for (const auto& alias : soundList)
	{
		auto channelIt = m_groupChannelMap.find(alias);
		if (channelIt != m_groupChannelMap.end())
		{
			// �ʱ� ���� ������Ʈ
			channelIt->second.baseVolume = volume;

			// ���� ī�װ� ���� ��������
			auto categoryIt = m_groupSoundCategoryMap.find(alias);
			float categoryVolume = 1.0f;
			if (categoryIt != m_groupSoundCategoryMap.end())
			{
				categoryVolume = m_categoryVolumes[categoryIt->second];
			}

			float actualVolume = volume * categoryVolume;
			FMOD_Channel_SetVolume(channelIt->second.channel, actualVolume);
		}
	}
}

void CSound_Manager::Set_Category_Volume(SOUND_CATEGORY category, float volume)
{
	// ī�װ� ���� ������Ʈ
	m_categoryVolumes[category] = volume;

	// ���� ���� ���� ����
	for (const auto& pair : m_soundCategoryMap)
	{
		if (pair.second == category)
		{
			auto channelIt = m_channelMap.find(pair.first);
			if (channelIt != m_channelMap.end())
			{
				float newVolume = channelIt->second.baseVolume * volume;
				FMOD_Channel_SetVolume(channelIt->second.channel, newVolume);
			}
		}
	}

	// �׷� ���� ���� ����
	for (const auto& pair : m_groupSoundCategoryMap)
	{
		if (pair.second == category)
		{
			auto channelIt = m_groupChannelMap.find(pair.first);
			if (channelIt != m_groupChannelMap.end())
			{
				float newVolume = channelIt->second.baseVolume * volume;
				FMOD_Channel_SetVolume(channelIt->second.channel, newVolume);
			}
		}
	}
}

_float CSound_Manager::Get_Category_Volume(SOUND_CATEGORY category)
{
	auto it = m_categoryVolumes.find(category);
	if (it != m_categoryVolumes.end())
	{
		return it->second;
	}
	return 1.0f; // �⺻ ������ 1.0
}

void CSound_Manager::Set_ImguiPlay(_bool isPlay)
{
	m_isImguiPlay = isPlay;

	// m_isImguiPlay�� false��� ��� ���带 ����
	if (!m_isImguiPlay)
	{
		// ���� ���� ä�� ����
		for (auto& pair : m_channelMap)
		{
			if (pair.second.channel)
			{
				FMOD_Channel_Stop(pair.second.channel);
			}
		}

		// �׷� ���� ä�� ����
		for (auto& pair : m_groupChannelMap)
		{
			if (pair.second.channel)
			{
				FMOD_Channel_Stop(pair.second.channel);
			}
		}
	}
	else
	{
		// ���÷� BGM ���
		Play_Sound(SOUND_KEY_NAME::SPACE_BGM, true, 0.2f);
	}
}


CSound_Manager* CSound_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSound_Manager* pInstance = new CSound_Manager(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype())) {
		MessageBox(0, L"CSound_Manager Created Failed", L"System Error", MB_OK);
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSound_Manager::Clone(void* pArg)
{
	return this;
}

void CSound_Manager::Free()
{
	// ���� ���� ����
	for (auto& pair : m_soundMap)
	{
		FMOD_Sound_Release(pair.second);
	}
	m_soundMap.clear();
	m_channelMap.clear();

	// �׷� ���� ����
	for (auto& pair : m_groupSoundMap)
	{
		FMOD_Sound_Release(pair.second);
	}
	m_groupSoundMap.clear();
	m_groupChannelMap.clear();

	if (m_pSoundSystem)
	{
		FMOD_System_Close(m_pSoundSystem);
		FMOD_System_Release(m_pSoundSystem);
		m_pSoundSystem = nullptr;
	}

	__super::Free();
}

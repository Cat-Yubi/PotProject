#include "stdafx.h"
#include <string>

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "Imgui_Manager.h"
#include "GameInstance.h"
#include "RenderInstance.h"

#include "UI_Manager.h"
#include "UIObject.h"

#include "IMGUI_Shader_Tab.h"
#include "IMGUI_Animation_Tab.h"
#include "IMGUI_Effect_Tab.h"
#include "IMGUI_UI_Tab.h"
#include "IMGUI_Object_Tab.h"
#include "IMGUI_Level_Tab.h"
#include "IMGUI_Camera_Tab.h"

#include "Character.h"

#include "imnodes.h"
#include <iostream>

_bool bShowImGuiWindows = true;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����
_bool bShowImGuiRenderTarget = false;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����
_bool bShowImGuiDebug_Component = false;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����
_bool bShowImGuiDebug_COut = true;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����
_bool bShowImGuiLayerView = false;
_bool bShowImGuiPlayerInput = true;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����

_bool bShowImGuiUI_TopShow = false;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����
_bool bShowImGuiUI_MidShow = false;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����
_bool bShowImGuiUI_BotShow = false;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����

_bool bShowImGuiSoundIsActive = true;  // IMGUI â ǥ�� ���θ� �����ϴ� ���� ����

IMPLEMENT_SINGLETON(CImgui_Manager)

// IMGUI â ǥ�� ���θ� �����ϴ� ���� ����
CImgui_Manager::CImgui_Manager()
	: m_pGameInstance{ CGameInstance::Get_Instance() }, m_pRenderInstance{ CRenderInstance::Get_Instance() }

{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pRenderInstance);
}

HRESULT CImgui_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_pDevice = pDevice;
	m_pContext = pContext;
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImNodes::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	ImGui::StyleColorsLight();

	ImGui_ImplWin32_Init(g_hWnd);
	ImGui_ImplDX11_Init(m_pDevice, m_pContext);
	ImGui_ImplWin32_EnableDpiAwareness();

	//IMGUI �� ��ü ����
	//m_vecTabs.push_back(CIMGUI_Camera_Tab::Create(m_pDevice, m_pContext));
	//m_vecTabs.push_back(CIMGUI_Level_Tab::Create(m_pDevice, m_pContext));
	//m_vecTabs.push_back(CIMGUI_Animation_Tab::Create(m_pDevice, m_pContext));
	//m_vecTabs.push_back(CIMGUI_UI_Tab::Create(m_pDevice, m_pContext));
	m_vecTabs.push_back(CIMGUI_Effect_Tab::Create(m_pDevice, m_pContext));
	//m_vecTabs.push_back(CIMGUI_Object_Tab::Create(m_pDevice, m_pContext));

	m_pBackBufferSRV = m_pRenderInstance->Get_ViewPortSRV();
	Safe_AddRef(m_pBackBufferSRV);

	return S_OK;
}

void CImgui_Manager::Camera_Update(_float fTimeDelta)
{

}


void CImgui_Manager::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Mouse_Down(DIMK_LBUTTON))
	{
		_int iMeshIndex = Pick_Effect_Mesh();

		if (iMeshIndex != -1)
		{
			_int iCount = 0;
			for (auto& tab : m_vecShader_Tabs)
			{
				if (tab.second->m_iNumberId == iMeshIndex)
				{
					tab.second->m_TabPick = true;
					tab.second->TabPos_Init();
				}
				else
					tab.second->m_TabPick = false;

				iCount++;
			}
		}
	}
}

void CImgui_Manager::Late_Update(_float fTimeDelta)
{

}

HRESULT CImgui_Manager::Render(_float fTimeDelta)
{
	m_fTimeAcc += fTimeDelta;

	++m_iNumCount;

	if (m_fTimeAcc >= 1.f)
	{
		m_fTimeAcc = 0.f;
		m_iNumRender = m_iNumCount;
		m_iNumCount = 0;
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	
	//// Render IMGUI UI elements
	//Render_IMGUI(fTimeDelta);
	
	//Render_EffectAnimationTabs(fTimeDelta);

	ImGui::SetNextWindowSize(ImVec2(1920, 20));

	if (ImGui::BeginMainMenuBar()) {
		ImGui::Text("FPS : %d", m_iNumRender);

		if (ImGui::BeginMenu("Render_Target")) {
			if (ImGui::MenuItem("Render_Target", NULL, &bShowImGuiRenderTarget)) {
				m_pRenderInstance->SetActive_RenderTarget(bShowImGuiRenderTarget);
			}
			ImGui::EndMenu();
		}

		if (ImGui::Checkbox("Test_View", &bShowImGuiLayerView)) {
			m_pRenderInstance->Show_Layer_View();
		}

		if (ImGui::BeginMenu("InputActive")) {
			if (ImGui::MenuItem("InputActive", NULL, &bShowImGuiPlayerInput)) {
				CGameObject* player_1P = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"), 0);
				CGameObject* player_2P = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"), 1);

				CCharacter* character_1P = static_cast<CCharacter*>(player_1P);
				CCharacter* character_2P = static_cast<CCharacter*>(player_2P);

				character_1P->Set_InputActive(!bShowImGuiPlayerInput);
				character_2P->Set_InputActive(!bShowImGuiPlayerInput);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	ImGui::SetNextWindowPos(ImVec2(0, 20));
	ImGui::SetNextWindowSize(ImVec2(1920, 1080));

	ImGui::Begin("Window", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |  
		ImGuiWindowFlags_NoMove |    
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoScrollbar | 
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoBringToFrontOnFocus);
	
	
	/*�޴��ٴ� �������� �׳� �����ư ���� ���ߵɵ�*/
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	//ImGui::BeginChild("Menu", ImVec2(1920, 30), true);
	////
	//ImGui::EndChild();
	//ImGui::PopStyleVar();

	/**
	*	�޴��ٴ� �������� �׳� �����ư ���� ���ߵɵ�
	*	���� âũ�� �����ϴ� ����� �������� �ϱ�
	*	�� �ִϸ��̼�â�� ����Ʈ�� �״�� �̽� �� ������ ������ ���̴��Ǳ���
	*	�����ʹؿ� ����Ʈ������ �Ļ��� ����� ������â ������ �����ϴ�â����
	*	�����ʻ�ܿ� ����Ʈ�� �����ִ� ������Ʈ ���� ( �ƿ����� �߰�, ��ŷ �������� )
	*/
	ImGui::BeginChild("Viewport", ImVec2(1280, 720), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::Image((ImTextureID)m_pBackBufferSRV, ImVec2(1280, 720));
	ImGui::EndChild();

	ImGui::SameLine();


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

	ImGui::BeginChild("RightPanel", ImVec2(1920 - 1280, 720), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	ImGui::BeginChild("Outliner", ImVec2(1920 - 1280, 360), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::Text("Outliner");
	if (ImGui::TreeNode("Scene Objects"))
	{
		/* ������Ʈ ������ ���� ��� */
		ImGui::TreePop();
	}
	ImGui::EndChild();

	ImGui::BeginChild("Details", ImVec2(1920 - 1280, 360), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::Text("Ex");
	ImGui::Text("Name: Object 1");
	ImGui::Text("Position: (0, 0, 0)");
	ImGui::Text("Rotation: (0, 0, 0)");
	ImGui::Text("Scale: (1, 1, 1)");
	ImGui::EndChild();
	ImGui::EndChild();

	ImGui::PopStyleVar();

	ImGui::BeginChild("Effect Tool", ImVec2(1920, 360), true);
	(*m_vecTabs.begin())->Render(fTimeDelta);
	ImGui::EndChild();

	ImGui::End();

	if (m_pGameInstance->Key_Down(DIK_TAB))
		m_bisSwitchShaderTab = true;
	
	Render_ShaderTabs(fTimeDelta);

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return S_OK;
}

void CImgui_Manager::Show_Debug_COut(_bool bShow)
{
	if (bShow) // true�� �� �ܼ��� �ѱ�
	{
		if (::AllocConsole() == TRUE)
		{
			FILE* fp;
			if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
			{
				MessageBox(NULL, L"stdout ���𷺼� ����", L"Error", MB_OK);
			}
			if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
			{
				MessageBox(NULL, L"stderr ���𷺼� ����", L"Error", MB_OK);
			}
			// ����ȭ Ȱ��ȭ
			std::ios::sync_with_stdio(true);
			// ��Ʈ�� ���� �ʱ�ȭ
			std::cout.clear();
			std::cerr.clear();
			std::cout << "�ܼ� �ʱ�ȭ �Ϸ�" << std::endl;
			std::cerr << "�ܼ� �ʱ�ȭ �Ϸ�" << std::endl;
		}
	}
	else // false�� �� �ܼ��� ����
	{
		HWND hwndConsole = GetConsoleWindow(); // �ܼ� â�� �ڵ��� ������
		if (hwndConsole)
		{
			::FreeConsole(); // �ְܼ��� ���� ����
			PostMessage(hwndConsole, WM_CLOSE, 0, 0); // �ܼ� â�� �ݱ� �޽��� ������
		}
	}
}

void CImgui_Manager::Push_Shader_Tab(CTexture* pTexture, CEffect* pEffect)
{
	m_vecShader_Tabs[to_string(m_iShaderCount)] = (CIMGUI_Shader_Tab::Create(m_pDevice, m_pContext, pTexture, pEffect));
	m_vecShader_Tabs[to_string(m_iShaderCount)]->m_iNumberId = m_iShaderCount;

	m_iShaderCount++;
}

void CImgui_Manager::Save_Shader_Tab(_int iIndex, string fileName)
{
	m_vecShader_Tabs[to_string(iIndex)]->Click_Save_Shader_Tab(fileName);
}

void CImgui_Manager::Load_Shader_Tab(CTexture* pTexture, string strFilename, _int iIndex, CEffect* pEffect)
{
	m_vecShader_Tabs[to_string(iIndex)] = (CIMGUI_Shader_Tab::Create_Load(m_pDevice, m_pContext, pTexture, strFilename, pEffect));
	m_vecShader_Tabs[to_string(iIndex)]->m_iNumberId = iIndex;
	m_vecShader_Tabs[to_string(iIndex)]->Click_Load_Shader_Tab(strFilename.c_str());

	m_iShaderCount++;
}

void CImgui_Manager::Delete_Shader_Tab(_int iIndex)
{
	Safe_Release(m_vecShader_Tabs[to_string(iIndex)]);
	m_vecShader_Tabs.erase(to_string(iIndex));
}

_int CImgui_Manager::Pick_Effect_Mesh()
{
	_int isPick = { -1 };

	POINT ptMouse{};

	GetCursorPos(&ptMouse);
	ScreenToClient(g_hWnd, &ptMouse);

	_bool isOverMainImGui = (ptMouse.x >= m_ImGuiScreen.MainImGuiPos.x && ptMouse.x <= m_ImGuiScreen.MainImGuiPos.x + m_ImGuiScreen.MainImGuiSize.x &&
		ptMouse.y >= m_ImGuiScreen.MainImGuiPos.y && ptMouse.y <= m_ImGuiScreen.MainImGuiPos.y + m_ImGuiScreen.MainImGuiSize.y);

	_bool isOverShaderImGui = (ptMouse.x >= m_ImGuiScreen.ShaderImGuiPos.x && ptMouse.x <= m_ImGuiScreen.ShaderImGuiPos.x + m_ImGuiScreen.ShaderImGuiSize.x &&
		ptMouse.y >= m_ImGuiScreen.ShaderImGuiPos.y && ptMouse.y <= m_ImGuiScreen.ShaderImGuiPos.y + m_ImGuiScreen.ShaderImGuiSize.y);

	if (!isOverMainImGui && !isOverShaderImGui)
	{
		isPick = m_pRenderInstance->Picked_Effect_Index();
	}


	return isPick;
}

void CImgui_Manager::Click_EffectToShaderTab(_uint iMeshIndex)
{
	if (iMeshIndex != -1)
	{
		_int iCount = 0;
		for (auto& tab : m_vecShader_Tabs)
		{
			if (tab.second->m_iNumberId == iMeshIndex)
			{
				tab.second->m_TabPick = true;
				tab.second->TabPos_Init();
			}
			else
				tab.second->m_TabPick = false;

			iCount++;
		}
	}
}

void CImgui_Manager::Render_IMGUI(_float fTimeDelta)
{
	ImGui::Begin("EffecASDASDSADASDt Color Edit");

	// RGBA ���� ���� ���ι� �����̴�
	bool valueChanged = false;  // ���� ����Ǿ����� Ȯ��
	
	ImGui::Dummy(ImVec2(5.0f, 1.0f));
	ImGui::SameLine();
	valueChanged |= ImGui::VSliderFloat("R", ImVec2(20, 160), &color.x, 0.0f, 300.0f, "");
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(5.0f, 1.0f));
	ImGui::SameLine();
	valueChanged |= ImGui::VSliderFloat("G", ImVec2(20, 160), &color.y, 0.0f, 300.0f, "");
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(5.0f, 1.0f));
	ImGui::SameLine();
	valueChanged |= ImGui::VSliderFloat("B", ImVec2(20, 160), &color.z, 0.0f, 300.0f, "");
	ImGui::SameLine();
	ImGui::Dummy(ImVec2(5.0f, 1.0f));
	ImGui::SameLine();
	valueChanged |= ImGui::VSliderFloat("A", ImVec2(20, 160), &color.w, 0.0f, 300.0f, "");

	// ���� �Է��� ���� �ʵ�
	valueChanged |= ImGui::InputFloat4("", reinterpret_cast<float*>(&color));

	// 0.5f�� ����/�����ϴ� ��ư (RGB)
	if (ImGui::Button("+0.1 R")) { color.x = min(255.0f, max(0.0f, color.x + 0.1f)); valueChanged = true; }
	ImGui::SameLine();
	if (ImGui::Button("+0.1 G")) { color.y = min(255.0f, max(0.0f, color.y + 0.1f)); valueChanged = true; }
	ImGui::SameLine();
	if (ImGui::Button("+0.1 B")) { color.z = min(255.0f, max(0.0f, color.z + 0.1f)); valueChanged = true; }
	ImGui::SameLine();
	if (ImGui::Button("+0.05 A")) { color.w = min(30.0f, max(0.0f, color.w + 0.05f)); valueChanged = true; }

	if (ImGui::Button("-0.1 R")) { color.x = min(255.0f, max(0.0f, color.x - 0.1f)); valueChanged = true; }
	ImGui::SameLine();
	if (ImGui::Button("-0.1 G")) { color.y = min(255.0f, max(0.0f, color.y - 0.1f)); valueChanged = true; }
	ImGui::SameLine();
	if (ImGui::Button("-0.1 B")) { color.z = min(255.0f, max(0.0f, color.z - 0.1f)); valueChanged = true; }
	ImGui::SameLine();
	if (ImGui::Button("-0.05 A")) { color.w = min(30.0f, max(0.0f, color.w - 0.05f)); valueChanged = true; }

	ImGui::End();

	//m_pRenderInstance->Set_AuraColor(color);

	// ��� ��ư
	if (ImGui::BeginMainMenuBar()) {
		ImGui::Text("FPS : %d", m_iNumRender);

		if (ImGui::BeginMenu("Option")) {
			if (ImGui::MenuItem("IMGUI ON", NULL, &bShowImGuiWindows)) {
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Render_Target")) {
			if (ImGui::MenuItem("Render_Target", NULL, &bShowImGuiRenderTarget)) {
				m_pRenderInstance->SetActive_RenderTarget(bShowImGuiRenderTarget);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug_Component")) {
			if (ImGui::MenuItem("Debug_Component", NULL, &bShowImGuiDebug_Component)) {
				m_pRenderInstance->SetActive_Debug_Component(bShowImGuiDebug_Component);
			}
			ImGui::EndMenu();
		}

		if (ImGui::Checkbox("Test_View", &bShowImGuiLayerView)) {
			m_pRenderInstance->Show_Layer_View();
		}

		if (ImGui::BeginMenu("Debug_COut")) {
			if (ImGui::MenuItem("Debug_COut", NULL, &bShowImGuiDebug_COut)) {
				Show_Debug_COut(bShowImGuiDebug_COut);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("InputActive")) {
			if (ImGui::MenuItem("InputActive", NULL, &bShowImGuiPlayerInput)) {
				CGameObject* player_1P = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"), 0);
				CGameObject* player_2P = m_pGameInstance->Get_GameObject(LEVEL_GAMEPLAY, TEXT("Layer_Character"), 1);

				CCharacter* character_1P = static_cast<CCharacter*>(player_1P);
				CCharacter* character_2P = static_cast<CCharacter*>(player_2P);

				character_1P->Set_InputActive(!bShowImGuiPlayerInput);
				character_2P->Set_InputActive(!bShowImGuiPlayerInput);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("UI_Active")) {

			if (ImGui::MenuItem("TopUI_Active", NULL, &bShowImGuiUI_TopShow)) {
				
				for (auto& TopIter : CUI_Manager::Get_Instance()->m_ListTopUI)
				{
					TopIter->SetActive(bShowImGuiUI_TopShow);
				}
			}

			if (ImGui::MenuItem("MinUI_Active", NULL, &bShowImGuiUI_MidShow)) {
				for (auto& MidIter : CUI_Manager::Get_Instance()->m_ListMidUI)
				{
					MidIter->SetActive(bShowImGuiUI_MidShow);
				}
			}

			if (ImGui::MenuItem("BotUI_Active", NULL, &bShowImGuiUI_BotShow)) {
				for (auto& BotIter : CUI_Manager::Get_Instance()->m_ListBotUI)
				{
					BotIter->SetActive(bShowImGuiUI_BotShow);
				}
			}


			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("SoundActive")) {
			if (ImGui::MenuItem("SoundActive", NULL, &bShowImGuiSoundIsActive)) {
				m_pGameInstance->Set_ImguiPlay(bShowImGuiSoundIsActive);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (bShowImGuiWindows) {  // �� ������ ���� ��� ImGui â�� ǥ�� ���θ� ����

		ImGui::Begin("Main Tab", &bShowImGuiWindows); // ���� â ����
		if (ImGui::BeginTabBar("DragonBall_Tool")) { // �� �� ����

			for (auto& tab : m_vecTabs)
			{
				if (ImGui::BeginTabItem(tab->GetTabName()))
				{
					tab->Render(fTimeDelta);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar(); // �� �� ����
		}
		m_ImGuiScreen.MainImGuiPos = ImGui::GetWindowPos();
		m_ImGuiScreen.MainImGuiSize = ImGui::GetWindowSize();
		ImGui::End();  // â�� ����
	}
}

void CImgui_Manager::Render_ShaderTabs(_float fTimeDelta)
{
	//if (m_bisSwitchShaderTab == true)
	//{
	//	ImGui::SetNextWindowFocus();
	//	m_bisSwitchShaderTab = false;
	//	ImGui::Begin("Shader Tab", nullptr,
	//		ImGuiWindowFlags_NoBringToFrontOnFocus |
	//		ImGuiWindowFlags_NoFocusOnAppearing |
	//		ImGuiWindowFlags_NoNavFocus);
	//}
	//else
	//	ImGui::Begin("Shader Tab", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus);

	ImGui::Begin("Shader Tab");

	for (auto& tab : m_vecShader_Tabs)
	{
		

		if (/*ImGui::BeginTabItem(to_string(tab->m_iNumberId).c_str(), &tab->m_TabPick) || */tab.second->m_TabPick == true)
		{
			ImGui::Text("Mesh Index : %d", tab.second->m_iNumberId);
			m_iCurShaderTabId = tab.second->m_iNumberId;

			//ImGui::SameLine();
			//if (ImGui::Button("Out_Line"))
			//	m_pRenderInstance->Show_OutLine();
			if (tab.second->m_TabPick == false)
			{
				tab.second->TabPos_Init();
				tab.second->m_TabPick = true;
			}

			m_iCurShaderTabIndex = tab.second->m_iNumberId;
			tab.second->Render(fTimeDelta);
			//ImGui::EndTabItem();
		}
		else
		{
			tab.second->m_TabPick = false;

			if (m_pCurEffectLayer == nullptr)
			{
				//tab.second->Update(fTimeDelta);
			}
			else
			{
				for (auto& iter : m_pCurEffectLayer->m_MixtureEffects)
				{
					if (tab.first == to_string(iter->m_iUnique_Index))
						tab.second->Update(fTimeDelta);
				}
			}
		}


	}


	m_ImGuiScreen.ShaderImGuiPos = ImGui::GetWindowPos();
	m_ImGuiScreen.ShaderImGuiSize = ImGui::GetWindowSize();
	ImGui::End(); // ���� â ����
}

void CImgui_Manager::Render_EffectAnimationTabs(_float fTimeDelta)
{
}

void CImgui_Manager::ToolViewRender()
{

}

void CImgui_Manager::Free()
{
	for (auto& iter : m_vecTabs)
		Safe_Release(iter);

	m_vecTabs.clear();

	for (auto& iter : m_vecShader_Tabs)
		Safe_Release(iter.second);
	m_vecShader_Tabs.clear();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pRenderInstance);
	Safe_Release(m_pBackBufferSRV);
	__super::Free();
}

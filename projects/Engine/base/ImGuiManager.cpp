#include "ImGuiManager.h"

void ImGuiManager::Initialize(DirectXCommon* dxCommon, WinApp* winApp, SrvHeapManager* srvHeapManager)
{

	uint32_t index = srvHeapManager->Allocate();

	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon->GetDevice(),
		//swapChainDesc.BufferCount,
		2,
		//rtvDesc.Format,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvHeapManager->GetDescriptorHeap(),
		srvHeapManager->GetCPUDescriptorHandle(index),
		srvHeapManager->GetGPUDescriptorHandle(index));

}

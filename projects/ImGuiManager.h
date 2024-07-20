#pragma once
#include "SrvHeapManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

class ImGuiManager
{
public:

	//初期化
	void Initialize(DirectXCommon* dxCommon, WinApp* winApp, SrvHeapManager* srvHeapManager);

private:

};


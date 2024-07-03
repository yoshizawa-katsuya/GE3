#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "Model.h"
#include "PrimitiveDrawer.h"
#include "GameScene.h"
#include "Input.h"
#include <string>
#include <format>
#include <cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

void Log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}








/*
class ResourceObject {
public:
	ResourceObject(ID3D12Resource* resource)
		:resource_(resource)
	{}
	~ResourceObject() {
		if (resource_) {
			resource_->Release();
		}
	}
	ID3D12Resource* Get() { return resource_; }

private:
	ID3D12Resource* resource_;
};
*/
struct D3DResourceLeakChecker
{
	~D3DResourceLeakChecker()
	{
		//リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			debug->Release();
		}
	}
};

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheck;

	//ゲームウィンドウの作成
	WinApp* win = new WinApp();
	win->CreateGameWindow();

	// DirectX初期化
	DirectXCommon* dxCommon = new DirectXCommon();
	dxCommon->Initialize(win, win->GetKClientWidth(), win->GetKClientHeight());

	//入力の初期化
	Input* input = new Input();
	input->Initialize(win->GetHinstance(), win->GetHwnd());

	//TextureManager初期化
	TextureManager* textureManager = new TextureManager();
	textureManager->Initialize(dxCommon->GetDevice());

	//PSOの設定
	PrimitiveDrawer* primitiveDrawer = new PrimitiveDrawer();
	primitiveDrawer->Initialize(dxCommon->GetDevice());

	//DescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV = dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	//ゲームシーンの初期化
	GameScene* gameScene = new GameScene();
	gameScene->Initialize(dxCommon->GetDevice(), textureManager, win->GetKClientWidth(), win->GetKClientHeight());

	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(win->GetHwnd());
	ImGui_ImplDX12_Init(dxCommon->GetDevice(),
		//swapChainDesc.BufferCount,
		2,
		//rtvDesc.Format,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		textureManager->GetSrvDescriptorHeap(),
		dxCommon->GetCPUDescriptorHandle(textureManager->GetSrvDescriptorHeap(), descriptorSizeSRV, 0),
		textureManager->GetGPUDescriptorHandle(textureManager->GetSrvDescriptorHeap(), descriptorSizeSRV, 0));

	
	//ウィンドウのxボタンが押されるまでループ
	while (true) {
		if (win->ProcessMessage()) {
			break;
		}
		

		//imGuiに、フレームが始まる旨を伝える
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//入力の更新
		input->Update();


		//ゲームの処理
		gameScene->Update();

		

		
		//描画開始
		dxCommon->PreDraw();

		
		//ImGuiの内部コマンドを生成する
		ImGui::Render();

		
		
		
		
		//RootSignatureを設定。PSOに設定しているけど別途設定が必要
		primitiveDrawer->SetPipelineSet(dxCommon->GetCommandList(), BlendMode::kBlendModeNone);
		/*
		dxCommon->GetCommandList()->SetGraphicsRootSignature(primitiveDrawer->GetRootSignature());
		dxCommon->GetCommandList()->SetPipelineState(primitiveDrawer->GetGrahicsPipelineState());	//PSOを設定
		*/
		//commandList_->IASetIndexBuffer(&indexBufferView);	//IBVを設定
		//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
		dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		gameScene->Draw(dxCommon->GetCommandList(), primitiveDrawer);

		
		
		//実際のcommandListのImGuiの描画コマンドを積む
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommandList());

		dxCommon->PostDraw();

	}

	win->TerminateGameWindow();

	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	

#ifdef DEBUG
	//debugController->Relese();
#endif // DEBUG
	CloseWindow(win->GetHwnd());

	delete gameScene;
	delete primitiveDrawer;
	delete textureManager;
	delete input;
	delete dxCommon;
	delete win;

	//リソースリークチェック
	//IDXGIDebug1* debug;
	

	return 0;
}
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

	

	//球の分割数
	//const uint32_t kSubdivision = 16;
	//円周率
	//const float pi = float(M_PI);

	
	/*
	//経度分割1つ分の角度 φ
	const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//緯度分割1つ分の角度 θ
	const float kLatEvery = pi / float(kSubdivision);
	//緯度の方向に分割
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -pi / 2.0f + kLatEvery * latIndex;	//θ
		//経度の方向に分割しながら線を描く
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			uint32_t start = (latIndex * kSubdivision + lonIndex) * 4;
			float lon = lonIndex * kLonEvery;	//φ
			//頂点にデータを入力する。基準点a
			vertexData[start].position.x = cos(lat) * cos(lon);
			vertexData[start].position.y = sin(lat);
			vertexData[start].position.z = cos(lat) * sin(lon);
			vertexData[start].position.w = 1.0f;
			vertexData[start].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[start].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexData[start].normal.x = vertexData[start].position.x;
			vertexData[start].normal.y = vertexData[start].position.y;
			vertexData[start].normal.z = vertexData[start].position.z;
			//b
			uint32_t i = 1;
			vertexData[start + i].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData[start + i].position.y = sin(lat + kLatEvery);
			vertexData[start + i].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData[start + i].position.w = 1.0f;
			vertexData[start + i].texcoord.x = float(lonIndex) / float(kSubdivision);
			vertexData[start + i].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexData[start + i].normal.x = vertexData[start + i].position.x;
			vertexData[start + i].normal.y = vertexData[start + i].position.y;
			vertexData[start + i].normal.z = vertexData[start + i].position.z;
			//c
			i++;
			vertexData[start + i].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData[start + i].position.y = sin(lat);
			vertexData[start + i].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData[start + i].position.w = 1.0f;
			vertexData[start + i].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + i].texcoord.y = 1.0f - float(latIndex) / float(kSubdivision);
			vertexData[start + i].normal.x = vertexData[start + i].position.x;
			vertexData[start + i].normal.y = vertexData[start + i].position.y;
			vertexData[start + i].normal.z = vertexData[start + i].position.z;
			//d
			i++;
			vertexData[start + i].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexData[start + i].position.y = sin(lat + kLatEvery);
			vertexData[start + i].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexData[start + i].position.w = 1.0f;
			vertexData[start + i].texcoord.x = float(lonIndex + 1) / float(kSubdivision);
			vertexData[start + i].texcoord.y = 1.0f - float(latIndex + 1) / float(kSubdivision);
			vertexData[start + i].normal.x = vertexData[start + i].position.x;
			vertexData[start + i].normal.y = vertexData[start + i].position.y;
			vertexData[start + i].normal.z = vertexData[start + i].position.z;
			

		}
	}
	

	//ResourceIndex
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource = CreateBufferResource(device, sizeof(uint32_t) * (kSubdivision * kSubdivision * 6));

	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	//リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	
	indexBufferView.SizeInBytes = sizeof(uint32_t) * (kSubdivision * kSubdivision * 6);
	//インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {

			uint32_t index = (latIndex * kSubdivision + lonIndex) * 6;
			uint32_t index2 = (latIndex * kSubdivision + lonIndex) * 4;

			indexData[index] = index2;
			uint32_t i = 1;
			indexData[index + i] = index2 + 1;
			i++;
			indexData[index + i] = index2 + 2;
			i++;
			indexData[index + i] = index2 + 1;
			i++;
			indexData[index + i] = index2 + 3;
			i++;
			indexData[index + i] = index2 + 2;
		}
	}
	*/

	
	
	//テクスチャ切り替え用の変数
	//bool useMonaterBall = true;

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

		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n");
		}

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
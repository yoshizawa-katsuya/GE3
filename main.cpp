#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "Model.h"
#include "PrimitiveDrawer.h"
#include "GameScene.h"
#include "Input.h"
#include "dx12.h"
#include <format>
#include <dxgidebug.h>
#include <dxcapi.h>
#define _USE_MATH_DEFINES
#include <math.h>


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
	WinApp* winApp = new WinApp();
	winApp->CreateGameWindow();

	// DirectX初期化
	DirectXCommon* dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);

	//入力の初期化
	Input* input = new Input();
	input->Initialize(winApp);

	//TextureManager初期化
	TextureManager* textureManager = new TextureManager();
	textureManager->Initialize(dxCommon);

	//PSOの設定
	PrimitiveDrawer* primitiveDrawer = new PrimitiveDrawer();
	primitiveDrawer->Initialize(dxCommon);

	//DescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV = dxCommon->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
	//ゲームシーンの初期化
	GameScene* gameScene = new GameScene();
	gameScene->Initialize(dxCommon->GetDevice(), textureManager, WinApp::kClientWidth, WinApp::kClientHeight);

	
	
	//ウィンドウのxボタンが押されるまでループ
	while (true) {
		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
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

	
	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	

#ifdef DEBUG
	//debugController->Relese();
#endif // DEBUG
	
	winApp->TerminateGameWindow();

	//解放処理
	delete gameScene;
	gameScene = nullptr;

	delete primitiveDrawer;
	primitiveDrawer = nullptr;

	delete textureManager;
	textureManager = nullptr;

	//入力開放
	delete input;
	input = nullptr;

	delete dxCommon;
	dxCommon = nullptr;

	//WindowsAPI解放
	delete winApp;
	winApp = nullptr;

	//リソースリークチェック
	//IDXGIDebug1* debug;
	

	return 0;
}
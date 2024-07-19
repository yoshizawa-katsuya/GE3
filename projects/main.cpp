#include "WinApp.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "SpritePlatform.h"
#include "Model.h"
#include "ModelPlatform.h"
#include "PrimitiveDrawer.h"
#include "GameScene.h"
#include "Input.h"
#include "dx12.h"
#include "D3DResourceLeakChecker.h"
#include <format>
#include <dxgidebug.h>
#include <dxcapi.h>
#define _USE_MATH_DEFINES
#include <math.h>



//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResourceLeakChecker leakCheck;

	//ゲームウィンドウの作成
	WinApp* winApp = new WinApp;
	winApp->Initialize();

	// DirectX初期化
	DirectXCommon* dxCommon = new DirectXCommon;
	dxCommon->Initialize(winApp);

	//入力の初期化
	Input* input = new Input;
	input->Initialize(winApp);

	//TextureManager初期化
	TextureManager::GetInstance()->Initialize(dxCommon);

	//PSOの設定
	PrimitiveDrawer* primitiveDrawer = new PrimitiveDrawer;
	primitiveDrawer->Initialize(dxCommon);

	//スプライト共通部の初期化
	SpritePlatform* spritePlatform = new SpritePlatform;
	spritePlatform->Initialize(dxCommon, primitiveDrawer);

	//3Dオブジェクト共通部の初期化
	ModelPlatform* modelPlatform = new ModelPlatform;
	modelPlatform->Initialize(dxCommon, primitiveDrawer);

	//ゲームシーンの初期化
	GameScene* gameScene = new GameScene;
	gameScene->Initialize(dxCommon, spritePlatform, modelPlatform);

	
	
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


		gameScene->Draw(primitiveDrawer);

		
		
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

	delete modelPlatform;
	modelPlatform = nullptr;

	delete spritePlatform;
	spritePlatform = nullptr;

	delete primitiveDrawer;
	primitiveDrawer = nullptr;

	TextureManager::GetInstance()->Finalize();

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
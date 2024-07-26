#include "WinApp.h"
#include "DirectXCommon.h"
#include "Audio.h"
#include "SrvHeapManager.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "Sprite.h"
#include "SpritePlatform.h"
#include "Model.h"
#include "ModelPlatform.h"
#include "PrimitiveDrawer.h"
#include "ParticleManager.h"
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

	//Audio初期化
	Audio* audio = new Audio;
	audio->Initialize();

	//SrvHeapManager初期化
	SrvHeapManager* srvHeapManager = new SrvHeapManager;
	srvHeapManager->Initialize(dxCommon);

	ImGuiManager* imGuiManager = new ImGuiManager;
	imGuiManager->Initialize(dxCommon, winApp, srvHeapManager);

	//入力の初期化
	Input* input = new Input;
	input->Initialize(winApp);

	//TextureManager初期化
	TextureManager::GetInstance()->Initialize(dxCommon, srvHeapManager);

	//PSOの設定
	PrimitiveDrawer* primitiveDrawer = new PrimitiveDrawer;
	primitiveDrawer->Initialize(dxCommon);

	//スプライト共通部の初期化
	SpritePlatform* spritePlatform = new SpritePlatform;
	spritePlatform->Initialize(dxCommon, primitiveDrawer);

	//ParticleManagerの初期化
	ParticleManager::GetInstance()->Initialize(dxCommon, srvHeapManager, primitiveDrawer);

	//3Dオブジェクト共通部の初期化
	ModelPlatform* modelPlatform = new ModelPlatform;
	modelPlatform->Initialize(dxCommon, primitiveDrawer);

	//ゲームシーンの初期化
	GameScene* gameScene = new GameScene;
	gameScene->Initialize(dxCommon, spritePlatform, modelPlatform, audio);

	
	
	//ウィンドウのxボタンが押されるまでループ
	while (true) {
		//Windowsのメッセージ処理
		if (winApp->ProcessMessage()) {
			//ゲームループを抜ける
			break;
		}
		

		//imGuiに、フレームが始まる旨を伝える
		imGuiManager->Begin();
		
		//入力の更新
		input->Update();


		//ゲームの処理
		gameScene->Update();

		//ImGuiの内部コマンドを生成する
		imGuiManager->End();
		
		//描画開始
		dxCommon->PreDraw();

		srvHeapManager->PreDraw();
		
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
		imGuiManager->Draw();
		
		dxCommon->PostDraw();

	}

	
	//ImGuiの終了処理
	imGuiManager->Finalize();
	

	//出力ウィンドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	OutputDebugStringW(L"文字列リテラルを出力するよ\n");

	std::wstring a(L"stringに埋め込んだ文字列を出力するよ\n");
	OutputDebugStringW(a.c_str());


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

	ParticleManager::GetInstance()->Finalize();

	delete primitiveDrawer;
	primitiveDrawer = nullptr;

	TextureManager::GetInstance()->Finalize();

	//入力開放
	delete input;
	input = nullptr;

	delete imGuiManager;
	imGuiManager = nullptr;

	delete srvHeapManager;
	srvHeapManager = nullptr;

	delete audio;
	audio = nullptr;

	delete dxCommon;
	dxCommon = nullptr;

	//WindowsAPI解放
	delete winApp;
	winApp = nullptr;

	//リソースリークチェック
	//IDXGIDebug1* debug;
	

	return 0;
}
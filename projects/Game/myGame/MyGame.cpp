#include "MyGame.h"

void MyGame::Initialize()
{

	

	//ゲームウィンドウの作成
	winApp_ = new WinApp;
	winApp_->Initialize();

	// DirectX初期化
	dxCommon_ = new DirectXCommon;
	dxCommon_->Initialize(winApp_);

	//Audio初期化
	audio_ = new Audio;
	audio_->Initialize();

	//SrvHeapManager初期化
	srvHeapManager_ = new SrvHeapManager;
	srvHeapManager_->Initialize(dxCommon_);

	imGuiManager_ = new ImGuiManager;
	imGuiManager_->Initialize(dxCommon_, winApp_, srvHeapManager_);

	//入力の初期化
	input_ = new Input;
	input_->Initialize(winApp_);

	//TextureManager初期化
	TextureManager::GetInstance()->Initialize(dxCommon_, srvHeapManager_);

	//PSOの設定
	primitiveDrawer_ = new PrimitiveDrawer;
	primitiveDrawer_->Initialize(dxCommon_);

	//スプライト共通部の初期化
	spritePlatform_ = new SpritePlatform;
	spritePlatform_->Initialize(dxCommon_, primitiveDrawer_);

	//ParticleManagerの初期化
	ParticleManager::GetInstance()->Initialize(dxCommon_, srvHeapManager_, primitiveDrawer_);

	//3Dオブジェクト共通部の初期化
	modelPlatform_ = new ModelPlatform;
	modelPlatform_->Initialize(dxCommon_, primitiveDrawer_);

	//ゲームシーンの初期化
	gameScene_ = new GameScene;
	gameScene_->Initialize(dxCommon_, spritePlatform_, modelPlatform_, audio_, input_);


}

void MyGame::Finalize()
{

	//ImGuiの終了処理
	imGuiManager_->Finalize();

	winApp_->TerminateGameWindow();

	//解放処理
	delete gameScene_;
	gameScene_ = nullptr;

	delete modelPlatform_;
	modelPlatform_ = nullptr;

	delete spritePlatform_;
	spritePlatform_ = nullptr;

	ParticleManager::GetInstance()->Finalize();

	delete primitiveDrawer_;
	primitiveDrawer_ = nullptr;

	TextureManager::GetInstance()->Finalize();

	//入力開放
	delete input_;
	input_ = nullptr;

	delete imGuiManager_;
	imGuiManager_ = nullptr;

	delete srvHeapManager_;
	srvHeapManager_ = nullptr;

	delete audio_;
	audio_ = nullptr;

	delete dxCommon_;
	dxCommon_ = nullptr;

	//WindowsAPI解放
	delete winApp_;
	winApp_ = nullptr;


}

void MyGame::Update()
{

	//Windowsのメッセージ処理
	if (winApp_->ProcessMessage()) {
		//ゲームループを抜ける
		isEndReqest_ = true;
	}


	//imGuiに、フレームが始まる旨を伝える
	imGuiManager_->Begin();

	//入力の更新
	input_->Update();


	//ゲームの処理
	gameScene_->Update();

	//ImGuiの内部コマンドを生成する
	imGuiManager_->End();

}

void MyGame::Draw()
{

	//描画開始
	dxCommon_->PreDraw();

	srvHeapManager_->PreDraw();

	//RootSignatureを設定。PSOに設定しているけど別途設定が必要
	primitiveDrawer_->SetPipelineSet(dxCommon_->GetCommandList(), BlendMode::kBlendModeNone);
	/*
	dxCommon->GetCommandList()->SetGraphicsRootSignature(primitiveDrawer->GetRootSignature());
	dxCommon->GetCommandList()->SetPipelineState(primitiveDrawer->GetGrahicsPipelineState());	//PSOを設定
	*/
	//commandList_->IASetIndexBuffer(&indexBufferView);	//IBVを設定
	//形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	gameScene_->Draw(primitiveDrawer_);

	//実際のcommandListのImGuiの描画コマンドを積む
	imGuiManager_->Draw();

	dxCommon_->PostDraw();


}

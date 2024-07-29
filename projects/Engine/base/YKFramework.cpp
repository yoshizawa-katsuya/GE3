#include "YKFramework.h"

void YKFramework::Initialize()
{
	//ゲームウィンドウの作成
	winApp_ = new WinApp;
	winApp_->Initialize();

	// DirectX初期化
	dxCommon_ = DirectXCommon::GetInstance();
	dxCommon_->Initialize(winApp_);

	//Audio初期化
	audio_ = Audio::GetInstance();;
	audio_->Initialize();

	//SrvHeapManager初期化
	srvHeapManager_ = new SrvHeapManager;
	srvHeapManager_->Initialize(dxCommon_);

	imGuiManager_ = new ImGuiManager;
	imGuiManager_->Initialize(dxCommon_, winApp_, srvHeapManager_);

	//入力の初期化
	input_ = Input::GetInstance();
	input_->Initialize(winApp_);

	//TextureManager初期化
	TextureManager::GetInstance()->Initialize(dxCommon_, srvHeapManager_);

	//PSOの設定
	primitiveDrawer_ = new PrimitiveDrawer;
	primitiveDrawer_->Initialize(dxCommon_);

	//スプライト共通部の初期化
	spritePlatform_ = SpritePlatform::GetInstance();
	spritePlatform_->Initialize(dxCommon_, primitiveDrawer_);

	//ParticleManagerの初期化
	ParticleManager::GetInstance()->Initialize(dxCommon_, srvHeapManager_, primitiveDrawer_);

	//3Dオブジェクト共通部の初期化
	modelPlatform_ = ModelPlatform::GetInstance();
	modelPlatform_->Initialize(dxCommon_, primitiveDrawer_);

}

void YKFramework::Finalize()
{
	//ImGuiの終了処理
	imGuiManager_->Finalize();

	winApp_->TerminateGameWindow();

	modelPlatform_->Finalize();

	spritePlatform_->Finalize();

	ParticleManager::GetInstance()->Finalize();

	delete primitiveDrawer_;
	primitiveDrawer_ = nullptr;

	TextureManager::GetInstance()->Finalize();

	//入力開放
	input_->Finalize();

	delete imGuiManager_;
	imGuiManager_ = nullptr;

	delete srvHeapManager_;
	srvHeapManager_ = nullptr;

	audio_->Finalize();

	dxCommon_->Finalize();

	//WindowsAPI解放
	delete winApp_;
	winApp_ = nullptr;
}

void YKFramework::Update()
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

	
}

void YKFramework::EndFrame()
{

	//ImGuiの内部コマンドを生成する
	imGuiManager_->End();

}

void YKFramework::Run()
{
	//初期化
	Initialize();

	while (true)	//ゲームループ
	{
		//毎フレーム更新
		Update();
		//終了リクエストが来たら抜ける
		if (GetIsEndReqest()) {
			break;
		}
		//描画
		Draw();
	}
	//ゲームの終了
	Finalize();
}

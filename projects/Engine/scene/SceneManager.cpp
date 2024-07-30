#include "SceneManager.h"

SceneManager* SceneManager::instance_ = nullptr;

SceneManager* SceneManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new SceneManager;
	}
	return instance_;
}

void SceneManager::Finalize()
{

	//最後のシーンの終了と解放
	scene_->Finalize();
	delete scene_;

	delete instance_;
	instance_ = nullptr;
}

void SceneManager::Update()
{

	//次シーンの予約があるなら
	if (nextScene_) {
		//旧シーンの終了
		if (scene_) {
			scene_->Finalize();
			delete scene_;
		}

		//シーン切り替え
		scene_ = nextScene_;
		nextScene_ = nullptr;

		//シーンマネージャをセット
		scene_->SetSceneManager(this);

		//次シーンを初期化する
		scene_->Initialize();
	}

	//実行中シーンを更新する
	scene_->Update();

}

void SceneManager::Draw()
{

	scene_->Draw();

}

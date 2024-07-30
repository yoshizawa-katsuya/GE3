#pragma once
#include "BaseScene.h"

//シーン管理
class SceneManager
{
public:

	//シングルトンインスタンスの取得
	static SceneManager* GetInstance();

	//終了
	void Finalize();

	//更新
	void Update();

	//描画
	void Draw();

	//次シーン予約
	void SetNextScene(BaseScene* nextScene) { nextScene_ = nextScene; }

private:

	static SceneManager* instance_;

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = default;
	SceneManager& operator=(SceneManager&) = default;

	//今のシーン(実行中シーン)
	BaseScene* scene_ = nullptr;

	//次のシーン
	BaseScene* nextScene_ = nullptr;

};
#pragma once
#include "GameScene.h"
#include "TitleScene.h"
#include "YKFramework.h"

//ゲーム全体
class MyGame : public YKFramework
{
public:

	//初期化
	void Initialize() override;

	//終了
	void Finalize() override;

	//更新
	void Update() override;

	//描画
	void Draw() override;

	
private:

	
	GameScene* gameScene_ = nullptr;
	TitleScene* titleScene_ = nullptr;
	
};

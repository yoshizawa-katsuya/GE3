#pragma once
#include "Model.h"

class Player
{
public:

	void Initialize(Model* model, Transforms* cameratransform);

	void Update();

	void Draw();


private:

	//Transform変数を作る
	Transforms transform_{ {1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f } };

	//カメラの変数
	Transforms* cameratransform_;

	Model* model_ = nullptr;

};


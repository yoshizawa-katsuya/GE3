#pragma once
#include "Struct.h"
class Camera;
class Input;


class DebugCamera
{
public:

	//初期化
	void Initialize(Camera* camera, Input* input);

	//更新
	void Update();

private:

	Camera* camera_;
	Input* input_;

	Vector3 target_ = { 0.0f, 0.0f, 10.0f };

};


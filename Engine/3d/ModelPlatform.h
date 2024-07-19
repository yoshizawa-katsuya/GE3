#pragma once
#include "DirectXCommon.h"
#include "PrimitiveDrawer.h"
#include "DirectionalLight.h"

class ModelPlatform
{
public:
	//初期化
	void Initialize(DirectXCommon* dxCommon, PrimitiveDrawer* primitiveDrawer);

	//共通描画設定
	void PreDraw();

	DirectXCommon* GetDxCommon() const { return dxCommon_; }

	void SetDirectionalLight(DirectionalLight* directionalLight) { directionalLight_ = directionalLight; }

private:

	DirectXCommon* dxCommon_;

	PrimitiveDrawer* primitiveDrawer_;

	DirectionalLight* directionalLight_;
};


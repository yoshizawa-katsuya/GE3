#pragma once
#include "DirectXCommon.h"
#include "PrimitiveDrawer.h"

class SpritePlatform
{
public:

	//初期化
	void Initialize(DirectXCommon* dxCommon, PrimitiveDrawer* primitiveDrawer);

	//共通描画設定
	void PreDraw();

	DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:

	DirectXCommon* dxCommon_;

	PrimitiveDrawer* primitiveDrawer_;

};


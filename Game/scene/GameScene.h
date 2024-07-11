#pragma once
#include <cstdlib>
#include <memory>
#include "Struct.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "SpritePlatform.h"
#include "Model.h"
#include "Sprite.h"
#include "Player.h"

class GameScene
{
public:

	~GameScene();

	void Initialize(DirectXCommon* dxCommon, SpritePlatform* spritePlatform);

	void Update();

	void Draw(PrimitiveDrawer* primitiveDrawer);

private:

	

	//デバイス
	DirectXCommon* dxCommon_;

	SpritePlatform* spritePlatform_;

	int blendMode = static_cast<int>(BlendMode::kBlendModeNone);

	//カメラの変数
	Transforms cameratransform;

	uint32_t textureHandle_[2];

	//平行光源用のResourceを作成
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	//データを書き込む
	DirectionalLight* directionalLightData_;

	std::unique_ptr<Model> model_;

	//std::vector<std::unique_ptr<Sprite>> sprites_;
	std::unique_ptr<Sprite> sprite_;

	//プレイヤー
	std::unique_ptr<Player> player_;

};


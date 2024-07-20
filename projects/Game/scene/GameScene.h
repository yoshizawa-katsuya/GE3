#pragma once
#include <cstdlib>
#include <memory>
#include "Struct.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "ModelPlatform.h"
#include "SpritePlatform.h"
#include "Model.h"
#include "DirectionalLight.h"
#include "Sprite.h"
#include "Player.h"
#include "Object3d.h"
#include "Camera.h"

class GameScene
{
public:

	~GameScene();

	void Initialize(DirectXCommon* dxCommon, SpritePlatform* spritePlatform, ModelPlatform* modelPlatform);

	void Update();

	void Draw(PrimitiveDrawer* primitiveDrawer);

private:

	

	//デバイス
	DirectXCommon* dxCommon_;

	SpritePlatform* spritePlatform_;
	ModelPlatform* modelPlatform_;

	int blendMode = static_cast<int>(BlendMode::kBlendModeNone);

	std::unique_ptr<Camera> camera_;

	uint32_t textureHandle_[2];

	//平行光源
	std::unique_ptr<DirectionalLight> directionalLight_;

	std::unique_ptr<Model> model_;
	std::unique_ptr<Model> modelAxis_;

	//std::vector<std::unique_ptr<Sprite>> sprites_;
	std::unique_ptr<Sprite> sprite_;

	//プレイヤー
	std::unique_ptr<Player> player_;

	//3Dオブジェクト
	std::unique_ptr<Object3d> object3d_;

};


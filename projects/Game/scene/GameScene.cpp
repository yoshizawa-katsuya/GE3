#include "GameScene.h"
#include "Vector.h"
#include "dx12.h"
#include "imgui/imgui.h"

GameScene::~GameScene() {



}

void GameScene::Initialize(DirectXCommon* dxCommon, SpritePlatform* spritePlatform, ModelPlatform* modelPlatform) {

	dxCommon_ = dxCommon;
	spritePlatform_ = spritePlatform;
	modelPlatform_ = modelPlatform;

	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Initialize(dxCommon_);

	modelPlatform_->SetDirectionalLight(directionalLight_.get());

	camera_ = std::make_unique<Camera>();
	camera_->SetRotate({ 0.3f, 0.0f, 0.0f });
	camera_->SetTransform({ 0.0f, 3.0f, -10.0f });

	textureHandle_[0] = TextureManager::GetInstance()->Load("./resources/uvChecker.png");
	textureHandle_[1] = TextureManager::GetInstance()->Load("./resources/monsterBall.png");

	model_ = std::make_unique<Model>();
	model_->Initialize(modelPlatform_);
	model_->CreateFromOBJ("./resources", "plane.obj");

	modelAxis_ = std::make_unique<Model>();
	modelAxis_->Initialize(modelPlatform_);
	modelAxis_->CreateFromOBJ("./resources", "axis.obj");
	
	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(textureHandle_[0], spritePlatform_);
	
	/*
	for (uint32_t i = 0; i < 5; ++i) {
		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		if (i < 2) {
			sprite->Initialize(textureHandle_[i], Vector2{ 100.0f, 100.0f }, spritePlatform_);
		}
		else {
			sprite->Initialize(textureHandle_[i % 2], Vector2{ 100.0f, 100.0f }, spritePlatform_);
		}
		Vector2 position = { 50.0f + 200.0f * i, 100.0f };
		sprite->SetPosition(position);
		sprites_.push_back(std::move(sprite));
	}
	*/
	//プレイヤーの初期化
	player_ = std::make_unique<Player>();
	player_->Initialize(model_.get(), camera_.get());

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(modelAxis_.get(), camera_.get());

}

void GameScene::Update() {

	camera_->Update();

	//プレイヤーの更新
	player_->Update();

	//3dオブジェクトの更新
	object3d_->Update();

	ImGui::Begin("Window");
	
	if (ImGui::TreeNode("Sprite")) {
		ImGui::DragFloat2("tranlate", &sprite_->GetPosition().x, 0.01f);
		ImGui::DragFloat2("size", &sprite_->GetSize().x, 0.01f);
		ImGui::SliderAngle("rotate", &sprite_->GetRotation());
		ImGui::ColorEdit4("color", &sprite_->GetColor().x);
		ImGui::DragFloat2("anchprPoint", &sprite_->GetAnchorPoint().x, 0.01f);
		ImGui::Checkbox("IsFlipX", &sprite_->GetIsFlipX());
		ImGui::Checkbox("IsFlipY", &sprite_->GetIsFlipY());
		ImGui::DragFloat2("textureLeftTop", &sprite_->GetTextureLeftTop().x, 0.01f);
		ImGui::DragFloat2("textureSize", &sprite_->GetTextureSize().x, 0.01f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("SpriteUV")) {
		ImGui::DragFloat2("UVTranslate", &sprite_->GetUVTransformAddress().translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &sprite_->GetUVTransformAddress().scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &sprite_->GetUVTransformAddress().rotate.z);

		ImGui::TreePop();
	}
	
	if (ImGui::TreeNode("camera")) {
		ImGui::DragFloat3("translate", &camera_->GetTranslate().x, 0.01f);
		ImGui::DragFloat3("rotate", &camera_->GetRotate().x, 0.01f);
		//ImGui::DragFloat3("scale", &cameratransform.scale.x, 0.01f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("directionalLight")) {
		ImGui::ColorEdit4("color", &directionalLight_->GetColor().x);
		ImGui::DragFloat3("direction", &directionalLight_->GetDirection().x, 0.01f);
		ImGui::DragFloat("intensity", &directionalLight_->GetIntensity(), 0.01f);

		ImGui::TreePop();
	}
	/*
	ImGui::RadioButton("BlendModeNone", &blendMode, static_cast<int>(BlendMode::kBlendModeNone));
	ImGui::RadioButton("BlendModeNormal", &blendMode, static_cast<int>(BlendMode::kBlendModeNormal));
	ImGui::RadioButton("BlendModeAdd", &blendMode, static_cast<int>(BlendMode::kBlendModeAdd));
	ImGui::RadioButton("BlendModeSubtract", &blendMode, static_cast<int>(BlendMode::kBlendModeSubtract));
	ImGui::RadioButton("BlendModeMultiply", &blendMode, static_cast<int>(BlendMode::kBlendModeMultiply));
	ImGui::RadioButton("BlendModeScreen", &blendMode, static_cast<int>(BlendMode::kBlendModeScreen));
	*/
	
	//ImGui::Checkbox("useMonsterBall", &useMonaterBall);
	ImGui::End();

}

void GameScene::Draw(PrimitiveDrawer* primitiveDrawer) {

	//primitiveDrawer->SetPipelineSet(dxCommon_->GetCommandList(), static_cast<BlendMode>(blendMode));


	//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonaterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

	//Modelの描画前処理
	modelPlatform_->PreDraw();
	//プレイヤーの描画
	player_->Draw();
	//3dオブジェクトの描画
	object3d_->Draw();

	//modelの描画
	//model_->Draw(commandList);

	//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spritePlatform_->PreDraw();

	//Spriteの描画。変更が必要なものだけ変更する

	/*
	for (const std::unique_ptr<Sprite>& sprite : sprites_) {
		sprite->Draw();
	}
	*/
	//sprite_->Draw();

}
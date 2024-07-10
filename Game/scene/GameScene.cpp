#include "GameScene.h"
#include "Vector.h"
#include "dx12.h"
#include "imgui/imgui.h"


GameScene::~GameScene() {



}

void GameScene::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, SpritePlatform* spritePlatform) {

	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
	spritePlatform_ = spritePlatform;

	//平行光源用のResourceを作成
	directionalLightResource_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLight));
	//データを書き込む
	//書き込むためのアドレスを取得
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	//デフォルト値
	directionalLightData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData_->direction = { 0.0f, 0.0f, 1.0f };
	directionalLightData_->intensity = 1.0f;


	cameratransform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -10.0f} };

	textureHandle1 = textureManager_->Load("resources/uvChecker.png");

	model_ = std::make_unique<Model>(dxCommon_->GetDevice(), &cameratransform, textureManager_,  WinApp::kClientWidth, WinApp::kClientHeight);
	model_->CreateFromOBJ("./resources", "plane.obj");

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initialize(textureHandle1, Vector2{ 320.0f, 180.0f }, Vector2{ 640.0f, 360.0f }, Vector4{ 1.0f, 1.0f, 1.0f, 1.0f }, spritePlatform_);

	//プレイヤーの初期化
	player_ = std::make_unique<Player>();
	player_->Initialize(model_.get());

}

void GameScene::Update() {

	//プレイヤーの更新
	player_->Update();

	ImGui::Begin("Window");
	ImGui::DragFloat3("tranlateSprite", &sprite_->GetTransformAddress().translate.x, 0.01f);
	if (ImGui::TreeNode("camera")) {
		ImGui::DragFloat3("translate", &cameratransform.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &cameratransform.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &cameratransform.scale.x, 0.01f);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("SpriteUV")) {
		ImGui::DragFloat2("UVTranslate", &sprite_->GetUVTransformAddress().translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &sprite_->GetUVTransformAddress().scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &sprite_->GetUVTransformAddress().rotate.z);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("directionalLight")) {
		ImGui::DragFloat4("color", &directionalLightData_->color.x, 0.01f);
		ImGui::DragFloat3("direction", &directionalLightData_->direction.x, 0.01f);
		directionalLightData_->direction = Normalize(directionalLightData_->direction);
		ImGui::DragFloat("intensity", &directionalLightData_->intensity, 0.01f);

		ImGui::TreePop();
	}

	ImGui::RadioButton("BlendModeNone", &blendMode, static_cast<int>(BlendMode::kBlendModeNone));
	ImGui::RadioButton("BlendModeNormal", &blendMode, static_cast<int>(BlendMode::kBlendModeNormal));
	ImGui::RadioButton("BlendModeAdd", &blendMode, static_cast<int>(BlendMode::kBlendModeAdd));
	ImGui::RadioButton("BlendModeSubtract", &blendMode, static_cast<int>(BlendMode::kBlendModeSubtract));
	ImGui::RadioButton("BlendModeMultiply", &blendMode, static_cast<int>(BlendMode::kBlendModeMultiply));
	ImGui::RadioButton("BlendModeScreen", &blendMode, static_cast<int>(BlendMode::kBlendModeScreen));

	
	//ImGui::Checkbox("useMonsterBall", &useMonaterBall);
	ImGui::End();

}

void GameScene::Draw(PrimitiveDrawer* primitiveDrawer) {

	primitiveDrawer->SetPipelineSet(dxCommon_->GetCommandList(), static_cast<BlendMode>(blendMode));


	//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonaterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
	//DirectionalRight
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());

	//プレイヤーの描画
	player_->Draw(dxCommon_->GetCommandList());

	//modelの描画
	//model_->Draw(commandList);

	//Spriteの描画準備。Spriteの描画に共通のグラフィックスコマンドを積む
	spritePlatform_->PreDraw();

	//Spriteの描画。変更が必要なものだけ変更する
	sprite_->Draw(dxCommon_->GetCommandList(), textureManager_);

}
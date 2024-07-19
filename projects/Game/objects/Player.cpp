#include "Player.h"
#include "imgui/imgui.h"

void Player::Initialize(Model* model, Transforms* cameratransform) {

	model_ = model;
	cameratransform_ = cameratransform;

	//model->SetRotate({ 0.0f, 3.15f, 0.0f });
	transform_.rotate = { 0.0f, 3.15f, 0.0f };

}

void Player::Update() {

	ImGui::Begin("Player");
	if (ImGui::TreeNode("obj")) {
		ImGui::ColorEdit4("color", &model_->GetMaterialDataAddress().color.x);
		ImGui::DragFloat3("translate", &transform_.translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &transform_.rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &transform_.scale.x, 0.01f);

		ImGui::TreePop();
	}
	ImGui::End();

}

void Player::Draw() {

	if (model_) {
		model_->Draw(transform_, cameratransform_);
	}

}
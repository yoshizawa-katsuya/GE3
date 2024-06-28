#include "Player.h"
#include "imgui/imgui.h"

void Player::Initialize(Model* model) {

	model_ = model;

	model->SetRotate({ 0.0f, 3.15f, 0.0f });

}

void Player::Update() {

	ImGui::Begin("Player");
	if (ImGui::TreeNode("obj")) {
		ImGui::ColorEdit4("color", &model_->GetMaterialDataAddress().color.x);
		ImGui::DragFloat3("translate", &model_->GetTransformAddress().translate.x, 0.01f);
		ImGui::DragFloat3("rotate", &model_->GetTransformAddress().rotate.x, 0.01f);
		ImGui::DragFloat3("scale", &model_->GetTransformAddress().scale.x, 0.01f);

		ImGui::TreePop();
	}
	ImGui::End();

}

void Player::Draw(ID3D12GraphicsCommandList* commandList) {

	model_->Draw(commandList);

}
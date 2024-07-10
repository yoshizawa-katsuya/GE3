#include "Sprite.h"
#include "Matrix.h"
#include <cassert>
#include "SpritePlatform.h"

void Sprite::Initialize(uint32_t textureHandle, Vector2 size, SpritePlatform* spritePlatform) {

	//引数を受け取ってメンバ変数に記録する
	spritePlatform_ = spritePlatform;
	textureHandle_ = textureHandle;
	size_ = size;

	CreateVertexData();

	CreateMaterialData();

	CreateTransformData();

}

void Sprite::Draw(TextureManager* textureManager) {

	transform_.scale = { size_.x, size_.y, 1.0f };
	transform_.translate = { position_.x, position_.y, 0.0f };
	transform_.rotate = { 0.0f, 0.0f, rotation_ };

	//Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, WinApp::kClientWidth, WinApp::kClientHeight, 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
	transformationMatrixData_->WVP = worldViewProjectionMatrixSprite;
	transformationMatrixData_->World = worldMatrixSprite;

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;


	//Spriteの描画。変更が必要なものだけ変更する
	//マテリアルのCBufferの場所を設定
	spritePlatform_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	spritePlatform_->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);	//VBVを設定
	spritePlatform_->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);	//IBVを設定
	//TransformationMatrixCBufferの場所を設定
	spritePlatform_->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	//SRVの設定
	textureManager->SetGraphicsRootDescriptorTable(spritePlatform_->GetDxCommon()->GetCommandList(), 2, textureHandle_);
	//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	//描画
	spritePlatform_->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
	//commandList_->DrawInstanced(6, 1, 0, 0);

}



void Sprite::CreateVertexData()
{

	vertexResource_ = spritePlatform_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);


	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースは頂点4つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	//四角形
	vertexData_[0].position = { -(1.0f / 2.0f), (1.0f / 2.0f), 0.0f, 1.0f };//左下
	vertexData_[0].texcoord = { 0.0f, 1.0f };
	vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexData_[1].position = { -(1.0f / 2.0f),-(1.0f / 2.0f), 0.0f, 1.0f };//左上
	vertexData_[1].texcoord = { 0.0f, 0.0f };
	vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexData_[2].position = { (1.0f / 2.0f), (1.0f / 2.0f), 0.0f, 1.0f };//右下
	vertexData_[2].texcoord = { 1.0f, 1.0f };
	vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexData_[3].position = { (1.0f / 2.0f), -(1.0f / 2.0f), 0.0f, 1.0f };//右上
	vertexData_[3].texcoord = { 1.0f, 0.0f };
	vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };

	//Sprite用のIndexResource
	indexResource_ = spritePlatform_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);


	//リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	indexData_[0] = 0; indexData_[1] = 1; indexData_[2] = 2;
	indexData_[3] = 1; indexData_[4] = 3; indexData_[5] = 2;

}

void Sprite::CreateMaterialData()
{

	//Sprite用のマテリアルリソースを作る
	materialResource_ = spritePlatform_->GetDxCommon()->CreateBufferResource(sizeof(Material));

	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//白を書き込む
	materialData_->color = {1.0f, 1.0f, 1.0f, 1.0f};
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();

}

void Sprite::CreateTransformData()
{

	//Sprite用のTransformationMatrix用のリソースを作る。Matix4x4 1つ分のサイズを用意する
	transformationMatrixResource_ = spritePlatform_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));

	//書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	//単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();
	transformationMatrixData_->World = MakeIdentity4x4();

}

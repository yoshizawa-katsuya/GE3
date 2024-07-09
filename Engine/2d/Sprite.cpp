#include "Sprite.h"
#include "Matrix.h"
#include <cassert>

void Sprite::Initialize(ID3D12Device* device, uint32_t textureHandle, Vector2 position, Vector2 size, Vector4 color, const int32_t kClientWidth, const int32_t kClientHeight) {

	device_ = device;
	textureHandle_ = textureHandle;
	kClientWidth_ = kClientWidth;
	kClientHeight_ = kClientHeight;

	vertexResource_ = CreateBufferResource(device_, sizeof(VertexData) * 4);

	
	//リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	//使用するリソースは頂点4つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	//1頂点当たりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//四角形
	vertexDataSprite[0].position = { position.x - (size.x / 2.0f), position.y + (size.y / 2.0f), 0.0f, 1.0f };//左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[1].position = { position.x - (size.x / 2.0f), position.y - (size.y / 2.0f), 0.0f, 1.0f };//左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[2].position = { position.x + (size.x / 2.0f), position.y + (size.y / 2.0f), 0.0f, 1.0f };//右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexDataSprite[3].position = { position.x + (size.x / 2.0f), position.y - (size.y / 2.0f), 0.0f, 1.0f };//右上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[3].normal = { 0.0f, 0.0f, -1.0f };

	//Sprite用のIndexResource
	indexResource_ = CreateBufferResource(device_, sizeof(uint32_t) * 6);

	
	//リソースの先頭のアドレスから使う
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	//使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	//インデックスはuint32_tとする
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0; indexDataSprite[1] = 1; indexDataSprite[2] = 2;
	indexDataSprite[3] = 1; indexDataSprite[4] = 3; indexDataSprite[5] = 2;

	//Sprite用のマテリアルリソースを作る
	materialResource_ = CreateBufferResource(device_, sizeof(Material));
	
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	//白を書き込む
	materialData_->color = color;
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentity4x4();

	//Sprite用のTransformationMatrix用のリソースを作る。Matix4x4 1つ分のサイズを用意する
	transformationMatrixResource_ = CreateBufferResource(device_, sizeof(TransformationMatrix));
	
	//書き込むためのアドレスを取得
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	//単位行列を書き込んでおく
	transformationMatrixData_->WVP = MakeIdentity4x4();

}

void Sprite::Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager) {

	//Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth_), float(kClientHeight_), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
	transformationMatrixData_->WVP = worldViewProjectionMatrixSprite;
	transformationMatrixData_->World = worldMatrixSprite;

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));
	materialData_->uvTransform = uvTransformMatrix;


	//Spriteの描画。変更が必要なものだけ変更する
	//マテリアルのCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);	//VBVを設定
	commandList->IASetIndexBuffer(&indexBufferView_);	//IBVを設定
	//TransformationMatrixCBufferの場所を設定
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	//SRVの設定
	textureManager->SetGraphicsRootDescriptorTable(commandList, 2, textureHandle_);
	//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	//描画
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	//commandList_->DrawInstanced(6, 1, 0, 0);

}

Microsoft::WRL::ComPtr<ID3D12Resource> Sprite::CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes)
{

	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	//頂点リソースの設定
	D3D12_RESOURCE_DESC ResourceDesc{};
	//バッファリソース。テクスチャの場合はまた別の設定をする
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ResourceDesc.Width = sizeInBytes;
	//バッファの場合はこれらは1にする決まり
	ResourceDesc.Height = 1;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&Resource));
	assert(SUCCEEDED(hr));

	return Resource;

	//assert(SUCCEEDED(hr));

}
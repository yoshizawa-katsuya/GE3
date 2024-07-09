#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include "Struct.h"
#include "TextureManager.h"

class Sprite
{
public:

	void Initialize(ID3D12Device* device, uint32_t textureHandle, Vector2 position, Vector2 size, Vector4 color, const int32_t kClientWidth, const int32_t kClientHeight);

	//描画
	void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager);

	//Resource作成の関数化
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

	Transforms GetTransform() { return transform_; }
	Transforms GetUVTransform() { return uvTransform_; }

	Transforms& GetTransformAddress() { return transform_; }
	Transforms& GetUVTransformAddress() { return uvTransform_; }

private:

	int32_t kClientWidth_;
	int32_t kClientHeight_;

	//デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;


	//頂点バッファビューを作成する
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};


	//IBV
	//Sprite用のIndexResource
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	//Sprite用のマテリアルリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;

	//マテリアルにデータを書き込む
	Material* materialData_ = nullptr;

	//Sprite用のTransformationMatrix用のリソースを作る。Matix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;

	//データを書き込む
	TransformationMatrix* transformationMatrixData_ = nullptr;

	//Sprite用のTransform変数
	Transforms transform_{ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f ,0.0f, 0.0f} };
	Transforms uvTransform_{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f},
	};

	uint32_t textureHandle_;

};


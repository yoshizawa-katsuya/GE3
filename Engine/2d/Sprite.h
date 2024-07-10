#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include "Struct.h"
#include "TextureManager.h"
class SpritePlatform;

class Sprite
{
public:

	//初期化
	void Initialize(uint32_t textureHandle, Vector2 position, Vector2 size, Vector4 color, SpritePlatform* spritePlatform);

	//描画
	void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager);

	
	Transforms GetTransform() { return transform_; }
	Transforms GetUVTransform() { return uvTransform_; }

	Transforms& GetTransformAddress() { return transform_; }
	Transforms& GetUVTransformAddress() { return uvTransform_; }

private:

	//頂点データ作成
	void CreateVertexData(Vector2 position, Vector2 size);

	//マテリアルデータ作成
	void CreateMaterialData(Vector4 color);

	//座標行列変換データ作成
	void CreateTransformData();

	

	SpritePlatform* spritePlatform_ = nullptr;

	//頂点バッファビューを作成する
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData* vertexData_ = nullptr;

	//IBV
	//Sprite用のIndexResource
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
	uint32_t* indexData_ = nullptr;

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

	uint32_t textureHandle_ = 1;

};


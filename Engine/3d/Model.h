#pragma once
#include "Struct.h"
#include "TextureManager.h"
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

class Model
{
public:

	Model(ID3D12Device* device, Transforms* camera, const int32_t kClientWidth, const int32_t kClientHeight);

	void Initialize();

	void CreateFromOBJ(const std::string& directoryPath, const std::string& filename);

	void Draw(ID3D12GraphicsCommandList* commandList);

	//objファイルの読み込み
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	//mtlファイルの読み込み
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);

	//Resource作成の関数化
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

	Material& GetMaterialDataAddress() { return *materialData_; }

	Transforms& GetTransformAddress() { return transform_; }

	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }

private:

	int32_t kClientWidth_;
	int32_t kClientHeight_;

	//デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	ModelData modelData_;

	//VertexResourceを生成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	//頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	//マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	//マテリアルにデータを書き込む
	Material* materialData_ = nullptr;

	//TransformationMatrix用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	//データを書き込む
	TransformationMatrix* transformationMatrixData_ = nullptr;

	//Transform変数を作る
	Transforms transform_{ {1.0f, 1.0f, 1.0f}, { 0.0f, 0.0f, 0.0f}, { 0.0f, 0.0f, 0.0f } };

	//カメラの変数
	Transforms* cameratransform_;

	uint32_t textureHandle_;

};


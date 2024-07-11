#pragma once

#include <array>
#include <d3d12.h>
#include <string>
#include <wrl.h>
#include "DirectXCommon.h"
#include "DirectXTex/DirectXTex.h"

class TextureManager
{
public:

	//シングルトンインスタンスの取得
	static TextureManager* GetInstance();

	//終了
	void Finalize();

	//初期化
	void Initialize(DirectXCommon* dxCommon);

	//全テクスチャリセット
	void ResetAll();

	//読み込み
	uint32_t Load(const std::string& fileName);

	
	// デスクリプタテーブルをセット
	void SetGraphicsRootDescriptorTable(ID3D12GraphicsCommandList* commandList, UINT rootParamIndex, uint32_t textureHandle);


	//DiscriptorHeap作成の関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisiblr);

	//Textureデータを読む
	void LoadTexture(const std::string& filePath);
	
	std::wstring ConvertString(const std::string& str);

	std::string ConvertString(const std::wstring& str);

	ID3D12DescriptorHeap* GetSrvDescriptorHeap() const { return srvDescriptorHeap_.Get(); }


private:

	static TextureManager* instance_;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = default;

	DirectXCommon* dxCommon_;

	//デバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device_;

	uint32_t descriptorSizeSRV_;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;

	uint32_t index_;

	// デスクリプターの数
	UINT kNumDescriptors_;


	/// <summary>
	/// テクスチャ
	/// </summary>
	struct Texture {
		std::string filePath;
		DirectX::TexMetadata metadata;
		// テクスチャリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		// シェーダリソースビューのハンドル(CPU)
		D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandleSRV;
		// シェーダリソースビューのハンドル(CPU)
		D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandleSRV;
	};

	// テクスチャコンテナ
	std::array<Texture, DirectXCommon::kMaxSrvDescriptors_> textures_;

};
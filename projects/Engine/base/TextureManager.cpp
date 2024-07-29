#include "TextureManager.h"
#include <dxcapi.h>
#include <cassert>
#include "dx12.h"

TextureManager* TextureManager::instance_ = nullptr;

TextureManager* TextureManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new TextureManager;
	}
	return instance_;
}

void TextureManager::Finalize()
{
	delete instance_;
	instance_ = nullptr;
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvHeapManager* srvHeapManager) {

	assert(dxCommon);

	dxCommon_ = dxCommon;
	srvHeapManager_ = srvHeapManager;
	
	
	//全テクスチャリセット
	//ResetAll();

}

uint32_t TextureManager::Load(const std::string& fileName) {

	//読み込み済みテクスチャを検索
	auto it = std::find_if(
		textures_.begin(),
		textures_.end(),
		[&](Texture& texture) {return texture.filePath == fileName; }
	);
	if (it != textures_.end()) {
		uint32_t textureIndex = static_cast<uint32_t>(std::distance(textures_.begin(), it));
		return textureIndex;
	}

	

	index_ = srvHeapManager_->Allocate();
	
	//テクスチャ枚数上限チェック
	assert(srvHeapManager_->Check());

	//Textureを読んで転送する
	LoadTexture(fileName);

	return index_;
}

void TextureManager::SetGraphicsRootDescriptorTable(uint32_t textureHandle) {

	// シェーダリソースビューをセット
	srvHeapManager_->SetGraphicsRootDescriptorTable(2, textureHandle);
	
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureHandle)
{
	
	//範囲外指定違反チェック
	assert(textureHandle > 0 && textureHandle <= SrvHeapManager::kMaxSrvDescriptors_);

	Texture& texture = textures_[textureHandle];
	return texture.metadata;

}


//DiscriptorHeap作成の関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> TextureManager::CreateDescriptorHeap(
	Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisiblr) {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisiblr ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;

}

void TextureManager::LoadTexture(const std::string& filePath) {

	//テクスチャデータを追加して書き込む
	Texture& texture = textures_[index_];

	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す
	texture.filePath = filePath;
	texture.metadata = mipImages.GetMetadata();
	texture.resource = dxCommon_->CreateTextureResource(texture.metadata);

	//SRVを作成するDescriptorHeapの場所を決める。先頭はImGuiが使っているのでその次を使う
	texture.cpuDescHandleSRV = srvHeapManager_->GetCPUDescriptorHandle(index_);
	texture.gpuDescHandleSRV = srvHeapManager_->GetGPUDescriptorHandle(index_);

	srvHeapManager_->CreateSRVforTexture2D(index_, texture.resource.Get(), texture.metadata.format, UINT(texture.metadata.mipLevels));


	//Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	//全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		//MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		//Textureに転送
		HRESULT hr = texture.resource->WriteToSubresource(
			UINT(mipLevel),
			nullptr,				//全領域へコピー
			img->pixels,			//元データアドレス
			UINT(img->rowPitch),	//1ラインサイズ
			UINT(img->slicePitch)	//1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}

}

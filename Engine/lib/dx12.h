#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>

//Resource作成の関数化
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);


//ログ
namespace Logger
{
	void Log(const std::string& message);
}

//文字コードユーティリティ
namespace StringUtility 
{
	//wstringに変換
	std::wstring ConvertString(const std::string& str);

	//stringに変換
	std::string ConvertString(const std::wstring& str);
}

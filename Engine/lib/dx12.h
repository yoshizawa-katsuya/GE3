#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

//Resource作成の関数化
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes);

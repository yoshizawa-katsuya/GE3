#include "DirectXCommon.h"
#include <cassert>
#include <format>
#include <dx12.h>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dxcompiler.lib")

using namespace Microsoft::WRL;

void DirectXCommon::Initialize(WinApp* winApp) {

	//nullptrチェック
	assert(winApp);

	winApp_ = winApp;

	//DXGIデバイス初期化
	InitializeDXGIDevice();

	//コマンド関連初期化
	InitializeCommand();

	//スワップチェーンの生成
	CreateSwapChain();

	//各種デスクリプタヒープの生成
	CreateDescriptorHeaps();

	//レンダーターゲット生成
	CreateFinalRenderTargets();

	// 深度バッファ生成
	CreateDepthBuffer();

	//フェンス作成
	CreateFence();

	CreateViewport();

	CreateScissorRect();

	//DXCコンパイラ生成
	CreateDXCCompiler();

	ImGuiInitialize();
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(rtvDescriptorHeap_, descriptorSizeRTV_, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(rtvDescriptorHeap_, descriptorSizeRTV_, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDescriptorHeap_, descriptorSizeSRV_, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap_, descriptorSizeSRV_, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVCPUDescriptorHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(dsvDescriptorHeap_, descriptorSizeDSV_, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetDSVGPUDescriptorHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(dsvDescriptorHeap_, descriptorSizeDSV_, index);
}

void DirectXCommon::InitializeDXGIDevice() {

#ifdef  _DEBUG

	ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif //  _DEBUG

	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2", "12.1", "12.0" };

	//DXGIファクトリーの生成
	//HRESUlTはWindows系のエラーコードであり、
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));

	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	// どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; ++i) {

		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		//ソフトウェアアダプタでなければ採用！
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringの方なので注意
			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;	//ソフトウェアアダプタの場合は見なかったことにする

	}

	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		//指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を使ってループを抜ける
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	//デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Logger::Log("Complete create D3D12Device!!!\n");

#ifdef  _DEBUG

	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//Windoes11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			//https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
		
		//解放
		//infoQueue->Release();

	}

#endif //  _DEBUG

}

void DirectXCommon::InitializeCommand() {

	//コマンドアロケータを生成する
	HRESULT hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	//コマンドアロケータの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr,
		IID_PPV_ARGS(&commandList_));
	//コマンドリストの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

	//コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));

	//コマンドキューの生成がうまくいかなかったので起動できない
	assert(SUCCEEDED(hr));

}

void DirectXCommon::CreateSwapChain() {

	
	//スワップチェーンを生成する
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kClientWidth;		//画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height =WinApp::kClientHeight;	//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//色の形式
	swapChainDesc.SampleDesc.Count = 1;		//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;		//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//モニタにうつしたら、中身を破棄

	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(hr));

}

void DirectXCommon::CreateFinalRenderTargets() {

	

	//SwapChianからResourceを引っ張ってくる
	HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));

	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));

	assert(SUCCEEDED(hr));

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//2dテクスチャとして書き込む

	//RTVを2つ作るのでディスクリプタを2つ用意
	//まず一つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles_[0] = GetRTVCPUDescriptorHandle(0);
	device_->CreateRenderTargetView(swapChainResources_[0].Get(), &rtvDesc, rtvHandles_[0]);

	//2つ目のディスクリプタハンドルを得る(自力で)
	rtvHandles_[1] = GetRTVCPUDescriptorHandle(1);
	//2つ目を作る
	device_->CreateRenderTargetView(swapChainResources_[1].Get(), &rtvDesc, rtvHandles_[1]);



}

void DirectXCommon::CreateDepthBuffer() {

	

	//DepthStencilTextureをウィンドウのサイズで作成
	depthStencilResource_ = CreateDepthStencilTextureResource(WinApp::kClientWidth, WinApp::kClientHeight);

	
	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;	//2dTexture
	//DSVHeapの先頭にDSVを作る
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, GetDSVCPUDescriptorHandle(0));


}

void DirectXCommon::CreateDescriptorHeaps()
{

	//DescriptorSizeを取得しておく
	descriptorSizeRTV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeSRV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	//ディスクリプタヒープの生成
	//RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShadrVisibleはfalse
	rtvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	//SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue
	srvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kNumSrvDescriptors_, true);

	//DSV用のヒープでディスクリプタの数は1。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvDescriptorHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

}

void DirectXCommon::CreateFence() {

	//初期値0でFenceを作る
	HRESULT hr = device_->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

}

void DirectXCommon::CreateViewport()
{

	//ビューポート

	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport_.Width = FLOAT(WinApp::kClientWidth);
	viewport_.Height = FLOAT(WinApp::kClientHeight);
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

}

void DirectXCommon::CreateScissorRect()
{

	//シザー矩形

	//基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect_.left = 0;
	scissorRect_.right = WinApp::kClientWidth;
	scissorRect_.top = 0;
	scissorRect_.bottom = WinApp::kClientHeight;

}

void DirectXCommon::CreateDXCCompiler()
{

	//dxcCompilerを初期化
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	//includeに対応するための設定
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));

}

void DirectXCommon::ImGuiInitialize()
{

	//ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp_->GetHwnd());
	ImGui_ImplDX12_Init(device_.Get(),
		//swapChainDesc.BufferCount,
		2,
		//rtvDesc.Format,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		srvDescriptorHeap_.Get(),
		GetSRVCPUDescriptorHandle(0),
		GetSRVGPUDescriptorHandle(0));


}

void DirectXCommon::PreDraw() {

	
	//これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して使う
	barrier.Transition.pResource = swapChainResources_[backBufferIndex].Get();
	//遷移前（現在）のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);

	//描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = GetDSVCPUDescriptorHandle(0);
	commandList_->OMSetRenderTargets(1, &rtvHandles_[backBufferIndex], false, &dsvHandle);

	//指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };	//青っぽい色。RGBAの順
	commandList_->ClearRenderTargetView(rtvHandles_[backBufferIndex], clearColor, 0, nullptr);

	//指定した深度で画面全体をクリアする
	commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//描画用のDescriptorHeapの設定
	commandList_->SetDescriptorHeaps(1, srvDescriptorHeap_.GetAddressOf());

	commandList_->RSSetViewports(1, &viewport_);	//Viewportを設定

	

	commandList_->RSSetScissorRects(1, &scissorRect_);	//Scirssorを設定

}

void DirectXCommon::PostDraw() {

	//画面に描く処理はすべて終わり、画面に映すので、状態を遷移
	//これから書き込むバックバッファのインデックスを取得
	UINT backBufferIndex = swapChain_->GetCurrentBackBufferIndex();

	//TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して使う
	barrier.Transition.pResource = swapChainResources_[backBufferIndex].Get();
	//今回はRenderTargetからPresentにする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);

	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	HRESULT hr = commandList_->Close();
	assert(SUCCEEDED(hr));

	//GPUにコマンドリストの実行を行わせる
	ComPtr<ID3D12CommandList> commandLists[] = { commandList_ };
	commandQueue_->ExecuteCommandLists(1, commandLists->GetAddressOf());

	//GPUとOSに画面の交換を行うよう通知する
	swapChain_->Present(1, 0);

	//Fenceの値を更新
	fenceValue_++;
	//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSiganlを送る
	commandQueue_->Signal(fence_.Get(), fenceValue_);

	//Fenceの値が指定したSignak値にたどり着いているか確認する
		//GetCompletedValueの初期値はFence作成時に渡した初期値
	if (fence_->GetCompletedValue() < fenceValue_)
	{
		//FenceのSignalを待つためのイベントを作成する
		HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		assert(fenceEvent != nullptr);
		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent);
		//イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}

	//次のフレーム用コマンドリストを準備
	hr = commandAllocator_->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(hr));

}

ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(int32_t width, int32_t height)
{

	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;	//Textureの幅
	resourceDesc.Height = height;	//Textureの高さ
	resourceDesc.MipLevels = 1;	//mipmapの数
	resourceDesc.DepthOrArraySize = 1;	//奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//DepthStencliとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;	//サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	//2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	//DepthStencilとして使う通知

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;	//VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;	//1.0f(最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	//フォーマット。Resourceと合わせる

	//Resourceの生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,	//Heapの設定
		D3D12_HEAP_FLAG_NONE,	//Heapの特殊な設定。特になし。
		&resourceDesc,	//Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	//深度値を書き込む状態にしておく
		&depthClearValue,	//Clear最適値
		IID_PPV_ARGS(&resource)	//作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));

	return resource;
}

//DiscriptorHeap作成の関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisiblr) {

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisiblr ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));

	return descriptorHeap;

}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}
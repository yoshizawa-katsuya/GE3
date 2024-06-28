#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp
{
public:

	//ウィンドウプロシージャ
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	//ゲームウィンドウの作成
	void CreateGameWindow();

	//メッセージの処理
	bool ProcessMessage();

	//ゲームウィンドウの破棄
	void TerminateGameWindow();

	HWND GetHwnd() const { return hwnd_; }

	const int32_t GetKClientWidth() { return kClientWidth; }

	const int32_t GetKClientHeight() { return kClientHeight; }

private:

	//クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	//ウィンドウクラス
	WNDCLASS wc_{};

	HWND hwnd_ = nullptr;
	

};


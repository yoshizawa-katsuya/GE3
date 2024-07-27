#include "MyGame.h"



//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	MyGame game;

	game.Initialize();
	
	
	//ウィンドウのxボタンが押されるまでループ
	while (true) {
		
		game.Update();

		if (game.GetIsEndReqest()) {
			break;
		}
		
		game.Draw();
	}

	game.Finalize();

	
	

	//出力ウィンドウへの文字出力
	//OutputDebugStringA("Hello,DirectX!\n");

	


#ifdef DEBUG
	//debugController->Relese();
#endif // DEBUG
	
	
	//リソースリークチェック
	//IDXGIDebug1* debug;
	

	return 0;
}
#include<Windows.h>
#ifdef _DEBUG 
#include<iostream>
#include <tchar.h>
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#endif

using namespace std;
//@brief コンソール画面にフォーマット付き文字列を表示
//@param format(%dとか%fとかの)
//@param 可変長引数
//@remarks この関数はデバッグ用 デバッグ時にしか動かない
void DebugOutputFormatString(const char* format, ...)
{
	#ifdef _DEBUG
		va_list valist;//可変長引数格納するところ
		va_start(valist, format);//可変長引数リストへのアクセス,va_startの第二引数は最後の固定引数
		printf(format, valist);
		va_end(valist);
	#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OSに対してアプリの終了を知らせる
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//基底の処理
}
#ifdef _DEBUG
int main() {
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定
	w.lpszClassName = _T("DX12sample");//アプリケーション名てきとうでいい
	w.hInstance = GetModuleHandle(nullptr);//ハンドルの取得
	RegisterClassEx(&w);//アプリケーションクラス（ウィンドウの指定をOSに伝える）
	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT};//ウィンドウサイズ決定
	//関数でウィンドウサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//ウィンドウの生成
	HWND hwnd = CreateWindow(w.lpszClassName,//クラス名指定
							_T("DX12テスト"),//タイトルバーの文字
							WS_OVERLAPPEDWINDOW,//タイトルバーと境界線があるウィンドウ
							CW_USEDEFAULT,//X座標はOSに任せる
							CW_USEDEFAULT,//Y座標はOSに任せる
							wrc.right-wrc.left,//ウィンドウ幅
							wrc.bottom-wrc.top,//ウィンドウ高
							nullptr,//親ウィンドウタイトル
							nullptr,
							w.hInstance,//呼び出しアプリケーションハンドル
							nullptr);
	ShowWindow(hwnd, SW_SHOW);
	MSG msg = {};
	while (true) {
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		//アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}
	}
	UnregisterClass(w.lpszClassName, w.hInstance);//もうクラスを使わないので登録解除
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	DebugOutputFormatString("Show window test.");
	getchar();
	return 0;
}


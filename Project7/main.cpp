#include<Windows.h>
#ifdef _DEBUG 
#include<iostream>

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

#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	DebugOutputFormatString("Show window test.");
	getchar();
	return 0;
}


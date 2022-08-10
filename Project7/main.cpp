#include<Windows.h>
#ifdef _DEBUG 
#include<iostream>

#endif

using namespace std;
//@brief �R���\�[����ʂɃt�H�[�}�b�g�t���������\��
//@param format(%d�Ƃ�%f�Ƃ���)
//@param �ϒ�����
//@remarks ���̊֐��̓f�o�b�O�p �f�o�b�O���ɂ��������Ȃ�
void DebugOutputFormatString(const char* format, ...)
{
	#ifdef _DEBUG
		va_list valist;//�ϒ������i�[����Ƃ���
		va_start(valist, format);//�ϒ��������X�g�ւ̃A�N�Z�X,va_start�̑������͍Ō�̌Œ����
		printf(format, valist);
		va_end(valist);
	#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//�E�B���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OS�ɑ΂��ăA�v���̏I����m�点��
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//���̏���
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


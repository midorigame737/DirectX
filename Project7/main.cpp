#include<Windows.h>
#include<vector>
#ifdef _DEBUG 
#include<iostream>
#include <tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
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
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	//Direct3D��{�I�u�W�F�N�g����
	ID3D12Device* _dev = nullptr;
	IDXGIFactory6* _dxgiFactory = nullptr;
	IDXGISwapChain4* _swapchain = nullptr;
	ID3D12CommandAllocator* _cmdAllocator = nullptr;//GPU�R�}���h�p�̃X�g���[�W���蓖�ĂƂ������ւ̃C���^�[�t�F�[�X
	ID3D12GraphicsCommandList* _cmdList = nullptr;//�����_�����O�p�̃O���t�B�b�N�X�R�}���h�̖��߃I�u�W�F�N�g
	ID3D12CommandQueue* cmdQueue = nullptr;//�R�}���h���X�g�ł��߂����߃Z�b�g�����s���Ă������߂̃L���[
	WNDCLASSEX w = {};
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D_FEATURE_LEVEL feature_level;
	for (auto lv : levels) {
		if (D3D12CreateDevice(nullptr, lv, IID_PPV_ARGS(&_dev)) == S_OK) {
			feature_level = lv;
			break;
		}
	}
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
	
	//�A�_�v�^�̗񋓌^
	std::vector<IDXGIAdapter*>adapters;
	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;
	//���p�\�ȃA�_�v�^�[��񋓂��A�񋓂��ꂽ�A�_�v�^�[��std::vector�Ɋi�[
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//�A�_�v�^�[�̐����I�u�W�F�N�g�擾
		std::wstring strDesc = adesc.Description;
		//�T�������A�_�v�^�[�̖��O���m�F
		//NVIDIA���܂܂�Ă���A�_�v�^�[�I�u�W�F�N�g��������tmpAdapter�Ɋi�[
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}
	//�R�}���h�A���P�[�^�[(ID3D12CommandAllocator)�̐���
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
										  IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));

	//�R�}���h�L���[�̐ݒ�
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//�^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//�v���C�I���e�B���ɂȂ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//�R�}���h���X�g�ƍ��킹��
	result = _dev->CreateCommandQueue(&cmdQueueDesc,
		IID_PPV_ARGS(&cmdQueue));//�R�}���h�L���[����

	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//�R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12sample");//�A�v���P�[�V�������Ă��Ƃ��ł���
	w.hInstance = GetModuleHandle(nullptr);//�n���h���̎擾
	RegisterClassEx(&w);//�A�v���P�[�V�����N���X�i�E�B���h�E�̎w���OS�ɓ`����j
	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };//�E�B���h�E�T�C�Y����
	//�֐��ŃE�B���h�E�T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//�E�B���h�E�̐���
	HWND hwnd = CreateWindow(w.lpszClassName,//�N���X���w��
		_T("DX12�e�X�g"),//�^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW,//�^�C�g���o�[�Ƌ��E��������E�B���h�E
		CW_USEDEFAULT,//X���W��OS�ɔC����
		CW_USEDEFAULT,//Y���W��OS�ɔC����
		wrc.right - wrc.left,//�E�B���h�E��
		wrc.bottom - wrc.top,//�E�B���h�E��
		nullptr,//�e�E�B���h�E�^�C�g��
		nullptr,
		w.hInstance,//�Ăяo���A�v���P�[�V�����n���h��
		nullptr);

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};//�����_�����O���ꂽ�f�[�^���o�͂���O�Ɋi�[���邽�߂̃I�u�W�F�N�g
	swapchainDesc.Width = WINDOW_WIDTH;
	swapchainDesc.Height = WINDOW_HEIGHT;
	swapchainDesc.Format =DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;//�o�b�N�o�b�t�@�͐L�яk�݉\
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;//�t���b�v��͑��₩�ɔj��
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//���Ɏw��Ȃ�
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;//�E�B���h�E�t���X�N���[���؂�ւ��\
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	

	result = _dxgiFactory->CreateSwapChainForHwnd(
		cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);

	
	ShowWindow(hwnd, SW_SHOW);
	MSG msg = {};
	while (true) {
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT) {
			break;
		}
	}
	UnregisterClass(w.lpszClassName, w.hInstance);//�����N���X���g��Ȃ��̂œo�^����


	DebugOutputFormatString("Show window test.");
	getchar();
	return 0;
}


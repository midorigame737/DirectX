#include<Windows.h>
#include<vector>
#ifdef _DEBUG 
#include<iostream>
#endif
#include <tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

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
void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(
		IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();//デバッグレイヤー有効化
	//後で調べる
	debugLayer->Release();//有効化したらインターフェースを開放する
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//ウィンドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);//OSに対してアプリの終了を知らせる
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);//基底の処理
}
//Direct3D基本オブジェクト生成
ID3D12Device* _dev = nullptr;//コマンドキューとかコマンドリストとか色々作成するためのもの
IDXGIFactory6* _dxgiFactory = nullptr;//GPU設定に基づいたグラフィックスアダプタを選択する
IDXGISwapChain4* _swapchain = nullptr;//
ID3D12CommandAllocator* _cmdAllocator = nullptr;//GPUコマンド用のストレージ割り当てとかそこへのインターフェース
ID3D12GraphicsCommandList* _cmdList = nullptr;//レンダリング用のグラフィックスコマンドの命令オブジェクト
ID3D12CommandQueue* cmdQueue = nullptr;//コマンドリストでためた命令セットを実行していくためのキュー
#ifdef _DEBUG
int main() {
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	DebugOutputFormatString("Show window test.");
	HINSTANCE hInst = GetModuleHandle(nullptr);
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure;//コールバック関数の指定
	w.lpszClassName = _T("DX12sample");//アプリケーション名てきとうでいい
	w.hInstance = GetModuleHandle(nullptr);//ハンドルの取得
	RegisterClassEx(&w);//アプリケーションクラス（ウィンドウの指定をOSに伝える）
	RECT wrc = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT };//ウィンドウサイズ決定
	//関数でウィンドウサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);
	//ウィンドウの生成
	HWND hwnd = CreateWindow(w.lpszClassName,//クラス名指定
		_T("DX12テスト"),//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,//タイトルバーと境界線があるウィンドウ
		CW_USEDEFAULT,//X座標はOSに任せる
		CW_USEDEFAULT,//Y座標はOSに任せる
		wrc.right - wrc.left,//ウィンドウ幅
		wrc.bottom - wrc.top,//ウィンドウ高
		nullptr,//親ウィンドウタイトル
		nullptr,
		w.hInstance,//呼び出しアプリケーションハンドル
		nullptr);

#ifdef _DEBUG
	//デバッグレイヤー有効化
	//デバイス生成前にやっておかないとデバイスがロスするらしい
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory));
#else
	CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));
#endif
	
	EnableDebugLayer();
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
	
	//アダプタの列挙型
	std::vector<IDXGIAdapter*>adapters;
	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter* tmpAdapter = nullptr;
	//利用可能なアダプターを列挙し、列挙されたアダプターをstd::vectorに格納
	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);//アダプターの説明オブジェクト取得
		std::wstring strDesc = adesc.Description;
		//探したいアダプターの名前を確認
		//NVIDIAが含まれているアダプターオブジェクトを見つけてtmpAdapterに格納
		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}
	//コマンドアロケーター(ID3D12CommandAllocator)の生成
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
										  IID_PPV_ARGS(&_cmdAllocator));
	result = _dev->CreateCommandList(0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator, nullptr,
		IID_PPV_ARGS(&_cmdList));
	
	//コマンドキューの設定
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;//タイムアウトなし
	cmdQueueDesc.NodeMask = 0;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//プライオリティ特になし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;//コマンドリストと合わせる
	result = _dev->CreateCommandQueue(&cmdQueueDesc,
		IID_PPV_ARGS(&cmdQueue));//コマンドキュー生成

	

	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};//レンダリングされたデータを出力する前に格納するためのオブジェクト
	swapchainDesc.Width = WINDOW_WIDTH;
	swapchainDesc.Height = WINDOW_HEIGHT;
	swapchainDesc.Format =DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.Stereo = false;
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapchainDesc.BufferCount = 2;//バックバッファは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;//フリップ後は速やかに破棄
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//特に指定なし
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;//ウィンドウフルスクリーン切り替え可能
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	result = _dxgiFactory->CreateSwapChainForHwnd(
		//swapchainの出力ウィンドウのハンドルに関連付けられているswapchain作成
		cmdQueue,
		hwnd,
		&swapchainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapchain);
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};//ディスクリプタヒープ（ディスクリプタの内容を格納しておくところ）を作るための設定を書くためのオブジェクト
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//バッファの用途を教えてあげるところ、レンダターゲットだからRTV
	heapDesc.NodeMask = 0;//本来GPuが複数あるときのためのものなので0
	heapDesc.NumDescriptors = 2;//表と裏だから2つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//ビュー情報にあたつ情報をシェーダ側から参照する必要性があるかどうか、今回は特に指定なし
	ID3D12DescriptorHeap* rtvHeaps = nullptr;
	result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&rtvHeaps));
	DXGI_SWAP_CHAIN_DESC swcDesc = {};//
	result = _swapchain->GetDesc(&swcDesc);//[investigate]
	std::vector<ID3D12Resource*>_backBuffers(swcDesc.BufferCount);
	for (int idx = 0; idx < swcDesc.BufferCount; ++idx) {//バックバッファの数だけ設定が必要なのでループ
		result = _swapchain->GetBuffer(idx, IID_PPV_ARGS(&_backBuffers[idx]));//スワップチェーン上のバックバッファ取得
		D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();//[invistigate]
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(//フェンス作る、よくわからんからあとでリファレンス見る
		_fenceVal,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_fence));
	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();//[invistigate]

	D3D12_RESOURCE_BARRIER BarrierDesc = {};//このあたりよくわからんから後でしらべる
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//遷移
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
	BarrierDesc.Transition.Subresource = 0;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//直前はPRESENT状態
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//今からレンダターゲット状態
	_cmdList->ResourceBarrier(1, &BarrierDesc);//バリア指定実行

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


		result = _cmdAllocator->Reset();//[invistigate]

		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);

		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_cmdList->OMSetRenderTargets(1, &rtvH, true, nullptr);

		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//黄色で画面クリア
		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		//命令のクローズ
		_cmdList->Close();
		ID3D12CommandList* cmdlists[] = { _cmdList };
		cmdQueue->ExecuteCommandLists(1, cmdlists);
		if (_fence->GetCompletedValue() != _fenceVal) {
			//多分GPUの諸々イベントハンドル取得してるけど
			//よくわからんから後で調べる
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);//イベントが発生するまで待ち続ける
			WaitForSingleObject(event, INFINITE);
			//イベントハンドルを閉じる
			CloseHandle(event);
			////待ち
			cmdQueue->Signal(_fence, ++_fenceVal);
			_cmdAllocator->Reset();
			_cmdList->Reset(_cmdAllocator, nullptr);//再びコマンドリストをためる準備
			//フリップ
			_swapchain->Present(1, 0);
		}
	}
	UnregisterClass(w.lpszClassName, w.hInstance);//もうクラスを使わないので登録解除
	
	getchar();

	return 0;
}


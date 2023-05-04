#include<Windows.h>
#include<DirectXMath.h>
#include<d3dcompiler.h>
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
#pragma comment(lib,"d3dcompiler.lib")
using namespace DirectX;
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

void OutBloberror(ID3DBlob* errorBlob) {
	std::string errstr;
	errstr.resize(errorBlob->GetBufferSize());//大きさ必要な分に変える
	std::copy_n(//データをコピー
		(char*)errorBlob->GetBufferPointer(),
		errorBlob->GetBufferSize(),
		errstr.begin());
	OutputDebugStringA(errstr.c_str());//データ表示
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
ID3DBlob* vsBlob = nullptr;
ID3DBlob* psBlob = nullptr;
ID3DBlob* errorBlob = nullptr;
XMFLOAT3 vertices[]={//頂点座標定義
	{-1.0f,-1.0f,0.0f},//左下
	{-1.0f,1.0f,0.0f},//左上
	{1.0f,-1.0f,0.0f}//右下
};
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
	EnableDebugLayer();
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
	//HRESULT result = S_OK;
	/*if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory)))) {
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&_dxgiFactory)))) {
			return -1;
		}
	}*/

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
	//頂点バッファの生成
	D3D12_HEAP_PROPERTIES heapprop = {};//ヒープのプロパティ
	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;//ヒープのタイプ
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;//ヒープのページプロパティを指定するD3D12_CPU_PAGE_PROPERTY型指定された値。(よくわからん)
	D3D12_RESOURCE_DESC resdesc = {};//テクスチャなどのリソース
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;//リソースのディメンションかバッファを指定する
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;//リソースの幅
	resdesc.DepthOrArraySize = 1;//3Dの場合はリソースの深さを1Dor2D の場合配列サイズをの指定
	resdesc.MipLevels = 1;//MIP レベルの数を指定
	resdesc.Format= DXGI_FORMAT_UNKNOWN;//DXGI_FORMATを指定
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;//D3D12_TEXTURE_LAYOUTの 1 つのメンバーを指定

	ID3D12Resource* vertBuff = nullptr;

	result = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertBuff));

	XMFLOAT3* vertMap = nullptr;

	result = vertBuff->Map(//バッファの仮想上のアドレスを得るメソッド
		0,//リソース配列やミップマップの場合、サブリソース番号（今回は違うので0）
		nullptr,//マップしたい範囲、全範囲なのでぬるぽ
		(void**) &vertMap);//ポインタへのポインタ
	std::copy(std::begin(vertices),
	std::end(vertices), vertMap);
		vertBuff->Unmap(0, nullptr);
		//ディスクリプタ:GPUメモリ上に存在する、様々なデータやバッファの種類や位置、大きさ
		D3D12_VERTEX_BUFFER_VIEW vbView = {};
		vbView.BufferLocation = vertBuff->GetGPUVirtualAddress();//バッファの仮想アドレス
		vbView.SizeInBytes = sizeof(vertices);//全バイト数
		vbView.StrideInBytes = sizeof(vertices[0]);//1頂点あたりのバイト数
		_cmdList->IASetVertexBuffers(0, 1, &vbView);
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
	D3D12_CPU_DESCRIPTOR_HANDLE handle = rtvHeaps->GetCPUDescriptorHandleForHeapStart();//[invistigate]
	for (size_t idx = 0; idx < swcDesc.BufferCount; ++idx) {//バックバッファの数だけ設定が必要なのでループ
		result = _swapchain->GetBuffer(static_cast<UINT>(idx), IID_PPV_ARGS(&_backBuffers[idx]));//スワップチェーン上のバックバッファ取得
		handle.ptr += idx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		_dev->CreateRenderTargetView(_backBuffers[idx], nullptr, handle);
	}
	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	result = _dev->CreateFence(//フェンス作る、よくわからんからあとでリファレンス見る
		_fenceVal,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&_fence));
	ShowWindow(hwnd, SW_SHOW);
	MSG msg = {};
	
	//頂点シェーダーの読み込み
	result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",//シェーダファイル、Lついてるからワイド文字列
		nullptr,//シェーダーマクロオブジェクト
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//インクルードはデフォルト
		"BasicVS", "vs_5_0",//関数はBasicVS、対象シェーダーはvs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,//デバッグ用及び最適化なし
		0,
		&vsBlob, &errorBlob);//エラー時にerrorBlobにメッセージが入る
	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見つかりません");
			return 0;
		}
		else {
			OutBloberror(errorBlob);
		}
	}
	//ピクセルシェーダー読み込み
	result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",//シェーダー名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",//関数はBasicPS,対象シェーダーはps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&psBlob, &errorBlob);

	if (FAILED(result)) {
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			::OutputDebugStringA("ファイルが見つかりません");
			return 0;
		}
		else {
			OutBloberror(errorBlob);
		}
	}

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
			"POSITION",//データが何を表すか、今回は座標なのでPOSITION
			0,//同じセマンティクスのがあるときのインデックス、ないので0
			DXGI_FORMAT_R32G32B32_FLOAT,//データのフォーマット
			0,//入力スロットのインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,//データオフセットの位置
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,//データの内容一頂点ごとにこのデータが入ってる
			0//一度に描画するインスタンスの数
			},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr;//ルートシグネクチャあとで設定
	//シェーダの設定
	gpipeline.VS.pShaderBytecode = vsBlob->GetBufferPointer();//バイトコードのポインタ
	gpipeline.VS.BytecodeLength = vsBlob->GetBufferSize();//サイズ情報
	gpipeline.PS.pShaderBytecode = psBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = psBlob->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//デフォルトのサンプルマスク(ANDに使うやつ)定数(0xffffffff)
	gpipeline.RasterizerState.MultisampleEnable = false;//サンプルマスクの設定
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//カリング(見えないところ描画するかどうか)しない、塗りつぶす
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//中身を塗りつぶす
	gpipeline.RasterizerState.DepthClipEnable = true;//深度方向のクリッピング(描画範囲)は有効に
	gpipeline.BlendState.AlphaToCoverageEnable = false;//αテストの有無
	gpipeline.BlendState.IndependentBlendEnable = false;//レンダターゲットへのブレンドステートの割り当ての設定(全部同じ)
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};//レンダターゲットごとの設定
	renderTargetBlendDesc.BlendEnable = false;//αブレンドの有無
	renderTargetBlendDesc.LogicOpEnable = false;//論理演算の有無
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;//RGBAの情報全部ブレンドするときに使う
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;//1版最初のレンダターゲットに書いてきたやつ設定
	
	//入力レイアウトの設定
	gpipeline.InputLayout.pInputElementDescs = inputLayout;//レイアウト先頭アドレス
	gpipeline.InputLayout.NumElements = _countof(inputLayout);//レイアウト配列の要素数
	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//トライアングルストリップでカットなし
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//構成要素は三角形
	gpipeline.NumRenderTargets = 1;
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0~1に正規化されたRGBA
	gpipeline.SampleDesc.Count = 1;//サンプルは1ピクセルに付き一つ
	gpipeline.SampleDesc.Quality = 0;//クオリティは最低
	ID3D12PipelineState* _pipelinestate = nullptr;
	
	ID3D12RootSignature* rootsignature = nullptr;


	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	//ルートシグネクチャのバイナリコードを作成する
	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(
		&rootSignatureDesc,//ルートシグネクチャ(グ)設定
		D3D_ROOT_SIGNATURE_VERSION_1_0,//ルートシグネクチャバージョン
		&rootSigBlob,//シェーダーを作った時と同じ
		&errorBlob); // エラーも同様
	result = _dev->CreateRootSignature(0,//nodemask 0でいい
		rootSigBlob->GetBufferPointer(),//シェーダーのときと同様
		rootSigBlob->GetBufferSize(),//シェーダーのときと同様
		IID_PPV_ARGS(&rootsignature)//不要になったので解放
	);
	gpipeline.pRootSignature = rootsignature;

	//バイナリコードをもとにルートシグネクチャオブジェクトを生成
	result = _dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = static_cast<float>(WINDOW_WIDTH);
	viewport.Height = static_cast<float>(WINDOW_HEIGHT);
	viewport.TopLeftX = 0;//出力先の左上座標X
	viewport.TopLeftY = 0;//出力先の左上座標Y
	viewport.MaxDepth = 1.0f;//深度最大値
	viewport.MinDepth = 0.0f;//深度最低値
	
	//シザ―短形(ビューポートのどこまで描画するか)設定
	D3D12_RECT scissorrect = {};
	scissorrect.top = 0;//切り抜き上座標
	scissorrect.left = 0;//切り抜き左座標
	scissorrect.right = scissorrect.left + WINDOW_WIDTH;//切り抜き右座標
	scissorrect.bottom = scissorrect.top + WINDOW_HEIGHT;//切り抜き下座標
	while (true) {
		if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		//アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}
		auto bbIdx = _swapchain->GetCurrentBackBufferIndex();//[invistigate]
		D3D12_RESOURCE_BARRIER BarrierDesc = {};//このあたりよくわからんから後でしらべる
		BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//遷移
		BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		BarrierDesc.Transition.pResource = _backBuffers[bbIdx];
		BarrierDesc.Transition.Subresource = 0;
		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//直前はPRESENT状態
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//今からレンダターゲット状態
		_cmdList->ResourceBarrier(1, &BarrierDesc);//バリア指定実行
		
		
		//レンダターゲット指定
		auto rtvH = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr +=  static_cast<ULONG_PTR>(bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

		float clearColor[] = { 1.0f,1.0f,0.0f,1.0f };//黄色で画面クリア

		_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);
		_cmdList->SetPipelineState(_pipelinestate);
		_cmdList->SetGraphicsRootSignature(rootsignature);

		_cmdList->RSSetViewports(1, &viewport);
		_cmdList->RSSetScissorRects(1, &scissorrect);
		_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		_cmdList->IASetVertexBuffers(0, 1, &vbView);

		_cmdList->DrawInstanced(3, 1, 0, 0);//描画命令実行

		BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		_cmdList->ResourceBarrier(1, &BarrierDesc);
		//命令のクローズ
		_cmdList->Close();
		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { _cmdList };
		cmdQueue->ExecuteCommandLists(1, cmdlists);
		////待ち
		cmdQueue->Signal(_fence, ++_fenceVal);
		if (_fence->GetCompletedValue() != _fenceVal) {
			//多分GPUの諸々イベントハンドル取得してるけど
			//よくわからんから後で調べる
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);//イベントが発生するまで待ち続ける
			WaitForSingleObject(event, INFINITE);
			//イベントハンドルを閉じる
			CloseHandle(event);
			
			
		}
		_cmdAllocator->Reset();
		_cmdList->Reset(_cmdAllocator, nullptr);//再びコマンドリストをためる準備
		
		//フリップ
		_swapchain->Present(1, 0);
	}
	UnregisterClass(w.lpszClassName, w.hInstance);//もうクラスを使わないので登録解除
	
	getchar();

	return 0;
}


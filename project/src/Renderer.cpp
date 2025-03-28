#include "pch.h"
#include "Renderer.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		// Create some data for our mesh
		//std::vector<Vertex> vertices
		//{
		//	{ {  0.f ,  0.5f, 0.5f } , { 1.f, 0.f, 0.f } },
		//	{ {  0.5f, -0.5f, 0.5f } , { 0.f, 1.f, 0.f } },
		//	{ { -0.5f, -0.5f, 0.5f } , { 0.f, 0.f, 1.f } }
		//};
		//std::vector<uint32_t> indices{ 0, 1, 2 };

		m_pVehicleEffect = new FullShadeEffect(m_pDevice, L"resources/PosCol3D.fx");
		Texture* texture = Texture::LoadFromFile("resources/vehicle_diffuse.png", m_pDevice);
		m_pVehicleEffect->SetDiffuseMap(texture);
		delete texture;
		texture = Texture::LoadFromFile("resources/vehicle_specular.png", m_pDevice);
		m_pVehicleEffect->SetNormalMap(texture);
		delete texture;
		texture = Texture::LoadFromFile("resources/vehicle_specular.png", m_pDevice);
		m_pVehicleEffect->SetSpecularMap(texture);
		delete texture;
		texture = Texture::LoadFromFile("resources/vehicle_gloss.png", m_pDevice);
		m_pVehicleEffect->SetGlossinessMap(texture);
		delete texture;

		m_pVehicleMesh = new Mesh(m_pDevice, "resources/vehicle.obj", m_pVehicleEffect);


		m_pFireEffect = new FlatShadeEffect(m_pDevice, L"resources/Fire.fx");
		texture = Texture::LoadFromFile("resources/fireFX_diffuse.png", m_pDevice);
		m_pFireEffect->SetDiffuseMap(texture);
		delete texture;

		m_pFireMesh = new Mesh(m_pDevice, "resources/fireFX.obj", m_pFireEffect);

		m_Camera.Initialize(45.f, { 0.f, 0.f , -10.f }, m_Width / static_cast<float>(m_Height));
	}

	Renderer::~Renderer()
	{
		delete m_pFireEffect;
		delete m_pFireMesh;
		delete m_pVehicleEffect;
		delete m_pVehicleMesh;

		if (m_pRenderTargetView)		m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer)		m_pRenderTargetBuffer->Release();
		if (m_pDepthStencilView)		m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer)		m_pDepthStencilBuffer->Release(); 
		if (m_pSwapChain)				m_pSwapChain->Release(); 
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);
		m_pVehicleEffect->SetWorldMatrix(m_pVehicleMesh->GetWorldMatrix());

		m_pVehicleEffect->SetCameraPosition(m_Camera.origin);

		//m_pVehicleMesh->SetWorldMatrix(Matrix::CreateRotationY(pTimer->GetTotal() * PI_DIV_2));
		//m_pFireMesh->SetWorldMatrix(Matrix::CreateRotationY(pTimer->GetTotal() * PI_DIV_2));

		Matrix wvpm = m_pVehicleMesh->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
		m_pVehicleEffect->SetWorldViewProjectionMatrix(wvpm);
		wvpm = m_pFireMesh->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
		m_pFireEffect->SetWorldViewProjectionMatrix(wvpm);
	}


	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		// 1. CLEAR RTV & DSV
		constexpr float color[4] = { 0.f, 0.f, 0.3f, 1.f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		// 2. SET PIPELINE + INVOKE DRAW CALLS (= RENDER)
		m_pVehicleMesh->Render(m_pDeviceContext);
		m_pFireMesh->Render(m_pDeviceContext);

		// 3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::ToggleTechniqueIndex()
	{
		m_pVehicleMesh->ToggleTechniqueIndex();
		m_pFireMesh->ToggleTechniqueIndex();
	}

	HRESULT Renderer::InitializeDirectX()
	{
		// 1. Create Device & DeviceContext
		//=====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
			return result;

		// Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;

		////////////////////////////
		//IDXGIAdapter* adapter = nullptr;
		//IDXGIAdapter* selectedAdapter = nullptr;
		//for (UINT i = 0; pDxgiFactory->EnumAdapters(i, &adapter) !=
		//	DXGI_ERROR_NOT_FOUND; ++i)
		//{
		//	DXGI_ADAPTER_DESC desc;
		//	adapter->GetDesc(&desc);
		//	std::wcout << L"Adapter " << i << L": " << desc.Description << L"\n";
		//	if (desc.VendorId != 0x8086)
		//	{
		//		selectedAdapter = adapter;
		//		break;
		//	}
		//}
		//if (selectedAdapter == nullptr)
		//{
		//	//TODO error handling when no adapter found!!
		//}
		//// Change by adding selected adapter and pick type unknown
		//result = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN,
		//	0, createDeviceFlags, &featureLevel, 1,
		//	D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		////////////////////////////

		// 2. Create Swapchain
		//=====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		// Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		// 3. Create DepthStencil (DS) & DepthStencilView (DSV)
		// Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		// 4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//=====

		// Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		// View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		// 5. Bind RTV & DSV to Output Merger Stage
		//=====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// 6. Set Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return S_OK;
	}
}

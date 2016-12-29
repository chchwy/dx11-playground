#include "d3drenderwidget.h"
#include <QDebug>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using namespace DirectX;

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if(x) { x->Release(); x = NULL; }
#endif

#ifndef BOOL_CHECK( x )
#define BOOL_CHECK( x ) { bool b1 = (x); if ( b1 == false ) { Q_ASSERT(b1); return b1; } }
#endif

D3DRenderWidget::D3DRenderWidget(QWidget* parent) : QWidget( parent )
{
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NativeWindow, true);
}

D3DRenderWidget::~D3DRenderWidget()
{
    if ( m_pSwapChain )
    {
        m_pSwapChain->SetFullscreenState( false, NULL );
    }

    SAFE_RELEASE( m_pRasterState );
    SAFE_RELEASE( m_pDepthStencilView );
    SAFE_RELEASE( m_pDepthStencilState );
    SAFE_RELEASE( m_pDepthStencilBuffer );
    SAFE_RELEASE( m_pRenderTargetView );
    SAFE_RELEASE( m_pDevice );
    SAFE_RELEASE( m_pContext );
    SAFE_RELEASE( m_pSwapChain );
}

bool D3DRenderWidget::initialize()
{
    BOOL_CHECK( createDevice() );
	BOOL_CHECK( createDepthBuffer() );
	BOOL_CHECK( setupRasterization() );
	BOOL_CHECK( setupViewport() );
	BOOL_CHECK( createMatrices() );
}

void D3DRenderWidget::resizeEvent(QResizeEvent* evt)
{
	//releaseBuffers();
	m_pSwapChain->ResizeBuffers( 1, width(), height(), DXGI_FORMAT_R8G8B8A8_UNORM, 0 );
	//m_pSwapChain->GetDesc( &swapChainDesc_ );
	setupViewport();
	//createBuffers();
}

void D3DRenderWidget::paintEvent(QPaintEvent* evt)
{
    render();
}

bool D3DRenderWidget::createDevice()
{
    // Create swap chain, device and device context
    UINT createDeviceFlags = //D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;
       D3D11_CREATE_DEVICE_DEBUG;

    D3D_FEATURE_LEVEL myFeatureLevel;

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = width();
    sd.BufferDesc.Height = height();
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;

    sd.BufferCount  = 1;
    sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = (HWND)winId();
    sd.Windowed     = true;
    sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags        = 0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain( NULL,  // Default Adapter
												D3D_DRIVER_TYPE_HARDWARE,
												NULL,  // No software device
												createDeviceFlags,
												0, 0,  // Use default feature list
												D3D11_SDK_VERSION,
												&sd,
												&m_pSwapChain,
												&m_pDevice,
												&myFeatureLevel,
												&m_pContext );
    Q_ASSERT( SUCCEEDED( hr ) );
    if ( FAILED( hr ) )
    {
        return false;
    }

    if ( myFeatureLevel != D3D_FEATURE_LEVEL_11_0 )
    {
        return false;
    }

    // Set back buffer and render target
    ID3D11Texture2D* pBackBuffer = NULL;
    m_pSwapChain->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);

    hr = m_pDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pRenderTargetView );
    if ( FAILED( hr ) )
    {
		return false;
    }
    SAFE_RELEASE( pBackBuffer );
    
	return true;
}

bool D3DRenderWidget::createDepthBuffer()
{
    // Initialize depth buffer
    D3D11_TEXTURE2D_DESC td;
    memset( &td, 0, sizeof( D3D11_TEXTURE2D_DESC ) );

    td.Width = width();
    td.Height = height();
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    td.SampleDesc.Count = 1;
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;

    HRESULT hr = m_pDevice->CreateTexture2D( &td, 0, &m_pDepthStencilBuffer );
    if ( FAILED( hr ) )
    {
        return false;
    }

    D3D11_DEPTH_STENCIL_DESC dsd;

    dsd.DepthEnable = true;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsd.DepthFunc = D3D11_COMPARISON_LESS;

    dsd.StencilEnable = true;
    dsd.StencilReadMask = 0xFF;
    dsd.StencilWriteMask = 0xFF;

    dsd.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsd.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsd.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    dsd.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsd.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsd.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    hr = m_pDevice->CreateDepthStencilState( &dsd, &m_pDepthStencilState );
    if ( FAILED( hr ) )
    {
        return false;
    }
    m_pContext->OMSetDepthStencilState( m_pDepthStencilState, 1 );

	// Create depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;
	dsvd.Flags = 0;

	hr = m_pDevice->CreateDepthStencilView( m_pDepthStencilBuffer, &dsvd, &m_pDepthStencilView );
	if ( FAILED( hr ) )
	{
		return false;
	}

    return true;
}

bool D3DRenderWidget::setupRasterization()
{
    // Set Render target
    m_pContext->OMSetRenderTargets( 1, &m_pRenderTargetView, m_pDepthStencilView );

    // Set Rasterizer state
    CD3D11_RASTERIZER_DESC rd( D3D11_DEFAULT );

    HRESULT hr = m_pDevice->CreateRasterizerState( &rd, &m_pRasterState );
    if ( FAILED( hr ) )
    {
       return false;
    }

    m_pContext->RSSetState( m_pRasterState );

    return true;
}

bool D3DRenderWidget::setupViewport()
{
	// Set viewport
	D3D11_VIEWPORT viewPort;
	viewPort.Width = static_cast<float>( width() );
	viewPort.Height = static_cast<float>( height() );
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	m_pContext->RSSetViewports( 1, &viewPort );
	return true;
}

bool D3DRenderWidget::createMatrices()
{
    float filedOfView = XM_PI / 4.0f;
    float screenAspect = ( float )width() / ( float )height();

    m_projectionMatrix = XMMatrixPerspectiveFovLH( filedOfView,
                                                   screenAspect,
                                                   0.1,
                                                   1000 );
    m_worldMatrix = XMMatrixIdentity();
    m_orthoMatrix = XMMatrixOrthographicLH( width(), height(), 0.1, 1000 );

    return true;
}

bool D3DRenderWidget::render()
{
	if ( m_pContext )
	{
		float color[] = { 0, 0, 0, 1 };
		m_pContext->ClearRenderTargetView( m_pRenderTargetView, color );
		m_pContext->ClearDepthStencilView( m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
		m_pSwapChain->Present( 1, 0 );
		qDebug() << "render!";
	}
    return true;
}

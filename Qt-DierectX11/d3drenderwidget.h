#ifndef D3DRENDERWIDGET_H
#define D3DRENDERWIDGET_H

#include <QWidget>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>


class D3DRenderWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(D3DRenderWidget)

public:
    D3DRenderWidget(QWidget* parent = NULL);
    virtual ~D3DRenderWidget();
    virtual QPaintEngine* paintEngine() const { return NULL; }

    bool initialize();

protected:
    virtual void resizeEvent(QResizeEvent* evt) override;
    virtual void paintEvent(QPaintEvent* evt) override;

private:
    bool createDevice();
    bool createDepthBuffer();
    bool setupRasterization();
	bool setupViewport();
    bool createMatrices();
    bool render();

    ID3D11Device*        m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;
    IDXGISwapChain*      m_pSwapChain = nullptr;

    ID3D11RenderTargetView*  m_pRenderTargetView = nullptr;
    ID3D11Texture2D*         m_pDepthStencilBuffer = nullptr;
    ID3D11DepthStencilState* m_pDepthStencilState = nullptr;
    ID3D11DepthStencilView*  m_pDepthStencilView = nullptr;
    ID3D11RasterizerState*   m_pRasterState = nullptr;

    DirectX::XMMATRIX m_projectionMatrix;
    DirectX::XMMATRIX m_worldMatrix;
    DirectX::XMMATRIX m_orthoMatrix;
};

#endif // D3DRENDERWIDGET_H

#pragma once

#include "GameComponent.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>

namespace megaEngine {

    class TriangleComponent : public GameComponent
    {
    public:
        bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
        void Render(ID3D11DeviceContext* context) override;
        void Shutdown() override;

    private:
        bool CompileShader(const wchar_t* filename, const char* entryPoint,
            const char* target, ID3DBlob** blob, D3D_SHADER_MACRO* macros = nullptr);

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

        // COM(Component Object Model) — это стандарт Microsoft для создания компонентов программного обеспечения, 
        // которые могут взаимодействовать друг с другом, даже если они написаны на разных языках или находятся в
        // разных процессах.

        ID3D11DeviceContext* context_ = nullptr;
        UINT stride_ = 32; // sizeof(XMFLOAT4) * 2 (позиция + цвет)
        UINT offset_ = 0;
        UINT indexCount_ = 6;
    };

}
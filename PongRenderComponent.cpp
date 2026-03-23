#include "PongRenderComponent.h"
#include <d3dcompiler.h>
#include <string>

#pragma comment(lib, "d3dcompiler.lib")

namespace megaEngine {

    using DirectX::SpriteBatch;
    using DirectX::SpriteFont;

    bool PongRenderComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND)
    {
        device_ = device;

        // === HLSL ===
        const char* vsCode =
            "struct VS_IN { float3 pos : POSITION; float4 col : COLOR; };"
            "struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };"
            "PS_IN main(VS_IN input) {"
            "   PS_IN o;"
            "   o.pos = float4(input.pos, 1);"
            "   o.col = input.col;"
            "   return o;"
            "}";

        const char* psCode =
            "struct PS_IN { float4 pos : SV_POSITION; float4 col : COLOR; };"
            "float4 main(PS_IN input) : SV_TARGET { return input.col; }";

        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;

        D3DCompile(vsCode, strlen(vsCode), nullptr, nullptr, nullptr,
            "main", "vs_5_0", 0, 0, &vsBlob, nullptr);

        D3DCompile(psCode, strlen(psCode), nullptr, nullptr, nullptr,
            "main", "ps_5_0", 0, 0, &psBlob, nullptr);

        device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vs_);
        device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &ps_);

        D3D11_INPUT_ELEMENT_DESC layout[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };

        device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(), &layout_);

        vsBlob->Release();
        psBlob->Release();


        spriteBatch_ = std::make_unique<SpriteBatch>(context);
        spriteFont_ = std::make_unique<SpriteFont>(device, L"Fonts/myfile.spritefont");

        return true;
    }

    void PongRenderComponent::SetScore(int left, int right)
    {
        scoreLeft_ = left;
        scoreRight_ = right;
    }

    void PongRenderComponent::DrawScore(ID3D11DeviceContext* context)
    {
        spriteBatch_->Begin();

        std::wstring scoreText = std::to_wstring(scoreLeft_) + L" : " + std::to_wstring(scoreRight_);

        const float fieldWidth = 800.0f;
        const float fieldHeight = 600.0f;

        DirectX::XMVECTOR textSize = spriteFont_->MeasureString(scoreText.c_str());
        float textWidth = DirectX::XMVectorGetX(textSize);
        float textHeight = DirectX::XMVectorGetY(textSize);

        Vector2 textPos(
            (fieldWidth - textWidth) * 0.5f,  // центр по X
            scoreY_                            // отступ сверху
        );

        spriteFont_->DrawString(
            spriteBatch_.get(),
            scoreText.c_str(),
            textPos,
            DirectX::Colors::White
        );

        spriteBatch_->End();
    }

    void PongRenderComponent::DrawRect(ID3D11DeviceContext* ctx, Vector2 pos, Vector2 size)
    {
        float w = 800.0f;
        float h = 800.0f;

        auto toNDC = [&](float x, float y) {
            return Vector3(
                (x / w) * 2.0f - 1.0f,
                1.0f - (y / h) * 2.0f,
                0.0f);
            };

        Vertex verts[] = {
            { toNDC(pos.x, pos.y), {1,1,1,1} },
            { toNDC(pos.x + size.x, pos.y), {1,1,1,1} },
            { toNDC(pos.x, pos.y + size.y), {1,1,1,1} },

            { toNDC(pos.x + size.x, pos.y), {1,1,1,1} },
            { toNDC(pos.x + size.x, pos.y + size.y), {1,1,1,1} },
            { toNDC(pos.x, pos.y + size.y), {1,1,1,1} },
        };

        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_IMMUTABLE;
        bd.ByteWidth = sizeof(verts);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA data = {};
        data.pSysMem = verts;

        Microsoft::WRL::ComPtr<ID3D11Buffer> vb;
        device_->CreateBuffer(&bd, &data, &vb); 

        UINT stride = sizeof(Vertex);
        UINT offset = 0;

        ctx->IASetVertexBuffers(0, 1, vb.GetAddressOf(), &stride, &offset);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->IASetInputLayout(layout_.Get());

        ctx->VSSetShader(vs_.Get(), nullptr, 0);
        ctx->PSSetShader(ps_.Get(), nullptr, 0);

        ctx->Draw(6, 0);
    }

    void PongRenderComponent::Render(ID3D11DeviceContext* ctx)
    {
        DrawRect(ctx, left->position, { 10, left->height });

        DrawRect(ctx, right->position, { 10, right->height });

        DrawRect(ctx, ball->position, { 10, 10 });

        DrawScore(ctx);
    }

}
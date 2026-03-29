#include "PongRenderComponent.h"
#include <d3dcompiler.h>
#include <string>
#include <windows.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace megaEngine {

    using DirectX::SpriteBatch;
    using DirectX::SpriteFont;

    bool PongRenderComponent::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd)
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

        // --- Background shaders (full-screen triangle) ---
        const char* vsFull =
            "struct VS_OUT { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; };"
            "VS_OUT main(uint id : SV_VertexID) {"
            "    VS_OUT o;"
            "    float2 positions[3] = { float2(-1.0f, -1.0f), float2(-1.0f, 3.0f), float2(3.0f, -1.0f) };"
            "    float2 uvs[3] = { float2(0.0f, 0.0f), float2(0.0f, 2.0f), float2(2.0f, 0.0f) };"
            "    o.pos = float4(positions[id], 0.0f, 1.0f);"
            "    o.uv = uvs[id];"
            "    return o;"
            "}";


        const char* psChecker =
            "cbuffer BGCB : register(b0) { float2 resolution; float2 tileSize; };"
            "float4 main(float2 uv : TEXCOORD0, float4 spos : SV_POSITION) : SV_TARGET"
            "{"
            "    float2 p = spos.xy / resolution;"
            "    float2 coord = floor(p / tileSize);"
            "    float parity = fmod(coord.x + coord.y, 2.0f);"
            "    float color = parity < 1.0f ? 0.0f : 1.0f;"
            "    return float4(color, color, color, 1.0f);"
            "}";

        ID3DBlob* vsBgBlob = nullptr;
        ID3DBlob* psBgBlob = nullptr;

        D3DCompile(vsFull, strlen(vsFull), nullptr, nullptr, nullptr, "main", "vs_5_0", 0, 0, &vsBgBlob, nullptr);
        D3DCompile(psChecker, strlen(psChecker), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &psBgBlob, nullptr);

        device->CreateVertexShader(vsBgBlob->GetBufferPointer(), vsBgBlob->GetBufferSize(), nullptr, &vs_bg_);
        device->CreatePixelShader(psBgBlob->GetBufferPointer(), psBgBlob->GetBufferSize(), nullptr, &ps_bg_);

        vsBgBlob->Release();
        psBgBlob->Release();

        // Create constant buffer for background (resolution + tileSize)
        struct BGCB { float resolution[2]; float tileSize[2]; } bgcbData;
        RECT rc = {};
        if (hwnd) GetClientRect(hwnd, &rc);
        float width = (rc.right - rc.left) > 0 ? float(rc.right - rc.left) : 800.0f;
        float height = (rc.bottom - rc.top) > 0 ? float(rc.bottom - rc.top) : 600.0f;

        bgcbData.resolution[0] = width;
        bgcbData.resolution[1] = height;
        // tile size in normalized fraction of screen (we prefer pixel sizes): use e.g. 80 px tiles
        float tilePx = 80.0f;
        bgcbData.tileSize[0] = tilePx / width;
        bgcbData.tileSize[1] = tilePx / height;

        D3D11_BUFFER_DESC cbd = {};
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbd.ByteWidth = sizeof(BGCB);
        cbd.Usage = D3D11_USAGE_DEFAULT;

        D3D11_SUBRESOURCE_DATA csd = {};
        csd.pSysMem = &bgcbData;

        device->CreateBuffer(&cbd, &csd, &cb_bg_);

        // set CB to pixel shader (will be set again on render)
        context->PSSetConstantBuffers(0, 1, cb_bg_.GetAddressOf());

        // SpriteBatch/Font
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
            DirectX::Colors::Blue
        );

        spriteBatch_->End();
    }

    void PongRenderComponent::DrawRect(ID3D11DeviceContext* ctx, Vector2 pos, Vector2 size, Vector4 color)
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
            { toNDC(pos.x, pos.y), color },
            { toNDC(pos.x + size.x, pos.y), color },
            { toNDC(pos.x, pos.y + size.y), color },

            { toNDC(pos.x + size.x, pos.y), color },
            { toNDC(pos.x + size.x, pos.y + size.y), color },
            { toNDC(pos.x, pos.y + size.y), color },
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
        // Draw checkerboard background using full-screen triangle
        ctx->IASetInputLayout(nullptr);
        ctx->VSSetShader(vs_bg_.Get(), nullptr, 0);
        ctx->PSSetShader(ps_bg_.Get(), nullptr, 0);
        ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        ctx->PSSetConstantBuffers(0, 1, cb_bg_.GetAddressOf());
        ctx->Draw(3, 0);

        // Draw paddles/ball with red color
        Vector4 red = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
        DrawRect(ctx, left->position, { 10, left->height }, red); // left = red

        DrawRect(ctx, right->position, { 10, right->height }, red); // right = red

        DrawRect(ctx, ball->position, { 10, 10 }, red); // ball = red

        DrawScore(ctx);
    }

}
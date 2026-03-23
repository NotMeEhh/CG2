#pragma once
#include "GameComponent.h"
#include "PaddleComponent.h"
#include "BallComponent.h"

#include <d3d11.h>
#include <wrl.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <memory>

using namespace DirectX::SimpleMath;

namespace megaEngine {
    

    class PongRenderComponent : public GameComponent
    {
    public:
        game::PaddleComponent* left;
        game::PaddleComponent* right;
        game::BallComponent* ball;

        bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) override;
        void Render(ID3D11DeviceContext* context) override;

        void SetScore(int left, int right);
    
    private:

        const float scoreY_ = 30.0f;           // отступ сверху
        const float scoreMarginX_ = 100.0f;    // отступ от краёв
        std::unique_ptr<DirectX::SpriteBatch> spriteBatch_;
        std::unique_ptr<DirectX::SpriteFont> spriteFont_;
        int scoreLeft_ = 0;
        int scoreRight_ = 0;

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        Microsoft::WRL::ComPtr<ID3D11VertexShader> vs_;
        Microsoft::WRL::ComPtr<ID3D11PixelShader> ps_;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> layout_;

        struct Vertex {
            Vector3 pos;
            Vector4 color;
        };

        void DrawRect(ID3D11DeviceContext* ctx, Vector2 pos, Vector2 size);
        void DrawScore(ID3D11DeviceContext* context);

        private:
            ID3D11Device* device_ = nullptr;
    };

}
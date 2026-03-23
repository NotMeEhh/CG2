#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <memory>

#include "GameComponent.h"
#include "DisplayWin32.h"
#include "InputDevice.h"

using megaEngine::GameComponent;
using megaEngine::DisplayWin32;
using megaEngine::InputDevice;

namespace game {

    class Game
    {
    public:
        Game();
        ~Game();

        bool Initialize(HINSTANCE hInstance);
        void Run();
        void Shutdown();

    private:
        bool InitializeDirect3D();
        void RenderFrame();
        void UpdateFPS(float deltaTime);

        DisplayWin32 display_;
        InputDevice input_;

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

        std::vector<std::unique_ptr<GameComponent>> components_;

        int screenWidth_ = 800;
        int screenHeight_ = 800;

        float totalTime_ = 0.0f;
        unsigned int frameCount_ = 0;
    };

}
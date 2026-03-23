#include "Game.h"

#include "PaddleComponent.h"
#include "BallComponent.h"
#include "PongRenderComponent.h"
#include "PongGameComponent.h"

#include <dxgi.h>
#include <chrono>
#include <iostream>
#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

using namespace megaEngine;

namespace game {

    Game::Game() = default;
    Game::~Game() { Shutdown(); }

    bool Game::Initialize(HINSTANCE hInstance)
    {
        if (!display_.Initialize(L"Pong DX11", screenWidth_, screenHeight_, hInstance, &input_))
            return false;

        if (!InitializeDirect3D()) return false;
        auto left = std::make_unique<PaddleComponent>();
        auto right = std::make_unique<PaddleComponent>();
        auto ball = std::make_unique<BallComponent>();
        auto pong = std::make_unique<PongGameComponent>();
        auto renderer = std::make_unique<PongRenderComponent>();

        PaddleComponent* leftPtr = left.get();
        PaddleComponent* rightPtr = right.get();
        BallComponent* ballPtr = ball.get();

        left->input = &input_;
        right->input = &input_;

        left->upKey = 'W';
        left->downKey = 'S';

        right->upKey = VK_UP;
        right->downKey = VK_DOWN;

        left->position = { 20.0f, 400.0f };
        right->position = { 780.0f, 400.0f };

        pong->left = leftPtr;
        pong->right = rightPtr;
        pong->ball = ballPtr;
        pong->renderer = renderer.get();

        pong->fieldWidth = (float)screenWidth_;
        pong->fieldHeight = (float)screenHeight_;

        renderer->left = leftPtr;
        renderer->right = rightPtr;
        renderer->ball = ballPtr;

        if (!renderer->Initialize(device_.Get(), context_.Get(), display_.GetHwnd()))
            return false;
        
        components_.push_back(std::move(left));
        components_.push_back(std::move(right));
        components_.push_back(std::move(ball));
        components_.push_back(std::move(pong));
        components_.push_back(std::move(renderer));

        return true;
    }

    bool Game::InitializeDirect3D()
    {
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;

        DXGI_SWAP_CHAIN_DESC swapDesc = {};
        swapDesc.BufferCount = 2;
        swapDesc.BufferDesc.Width = screenWidth_;
        swapDesc.BufferDesc.Height = screenHeight_;
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapDesc.BufferDesc.RefreshRate = { 60, 1 };
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapDesc.OutputWindow = display_.GetHwnd();
        swapDesc.Windowed = TRUE;
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapDesc.SampleDesc.Count = 1;

        HRESULT res = D3D11CreateDeviceAndSwapChain(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1,
            D3D11_SDK_VERSION, &swapDesc,
            &swapChain_, &device_, nullptr, &context_);

        if (FAILED(res)) {
            std::cerr << "D3D11CreateDeviceAndSwapChain failed: " << res << std::endl;
            return false;
        }

        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
        if (FAILED(swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) return false;
        if (FAILED(device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView_))) return false;

        return true;
    }

    void Game::Run()
    {
        auto prevTime = std::chrono::steady_clock::now();

        while (display_.IsRunning())
        {
            if (!display_.ProcessMessages(&input_)) break;

            auto curTime = std::chrono::steady_clock::now();
            float deltaTime = std::chrono::duration<float>(curTime - prevTime).count();
            prevTime = curTime;

            for (auto& comp : components_)
                comp->Update(deltaTime);

            UpdateFPS(deltaTime);
            RenderFrame();
        }
    }

    void Game::RenderFrame()
    {
        float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };

        context_->ClearState();
        context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);

        D3D11_VIEWPORT vp = {
            0, 0,
            (float)screenWidth_,
            (float)screenHeight_,
            0.0f, 1.0f
        };

        context_->RSSetViewports(1, &vp);
        context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), nullptr);

        for (auto& comp : components_)
            comp->Render(context_.Get());

        swapChain_->Present(1, 0);
    }

    void Game::UpdateFPS(float deltaTime)
    {
        totalTime_ += deltaTime;
        frameCount_++;

        if (totalTime_ >= 1.0f) {
            float fps = frameCount_ / totalTime_;

            wchar_t title[128];
            swprintf_s(title, L"Pong DX11 - FPS: %.1f", fps);
            SetWindowText(display_.GetHwnd(), title);

            totalTime_ = 0.0f;
            frameCount_ = 0;
        }
    }

    void Game::Shutdown()
    {
        for (auto& comp : components_)
            comp->Shutdown();

        components_.clear();

        renderTargetView_.Reset();
        context_.Reset();
        device_.Reset();
        swapChain_.Reset();

        display_.Shutdown();
    }

}
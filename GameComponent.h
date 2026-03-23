#pragma once
#include <d3d11.h>

namespace megaEngine {
    /// Базовый класс для всех игровых объектов
    class GameComponent
    {
    public:
        virtual ~GameComponent() = default;

        virtual bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context, HWND hwnd) { return true; }

        virtual void Update(float deltaTime) {}

        virtual void Render(ID3D11DeviceContext* context) {}

        virtual void Shutdown() {}
    };
}
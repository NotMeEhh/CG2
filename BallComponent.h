#pragma once
#include "GameComponent.h"
#include <SimpleMath.h>
#include <DirectXCollision.h>

namespace game {
    class BallComponent : public megaEngine::GameComponent
    {
    public:
        DirectX::SimpleMath::Vector2 position = { 400, 300 };
        DirectX::SimpleMath::Vector2 velocity = { 300, 0 };
        float radius = 5.0f;

        DirectX::BoundingSphere GetBounds() const
        {
            using namespace DirectX;
            return BoundingSphere(
                DirectX::SimpleMath::Vector3(position.x, position.y, 0), 
                radius                              
            );
        }

        void Update(float dt) override
        {
            position += velocity * dt;
        }
    };
}
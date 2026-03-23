#pragma once

#include "GameComponent.h"
#include "InputDevice.h"


#include <SimpleMath.h>
#include <DirectXCollision.h>
#include <algorithm> 

namespace game {
    class PaddleComponent : public megaEngine::GameComponent
    {
    public:
        DirectX::SimpleMath::Vector2 position = { 0, 0 }; 
        float speed = 500.0f;
        float height = 100.0f;
        float width = 10.0f;

        int upKey;
        int downKey;
        megaEngine::InputDevice* input = nullptr;


        DirectX::BoundingBox GetBounds() const
        {
            return DirectX::BoundingBox(
                DirectX::SimpleMath::Vector3(position.x + width * 0.5, position.y + height * 0.5f, 0),            // центр
                DirectX::SimpleMath::Vector3(width * 0.5f, height * 0.5f, 0.1f)    
            );
        }

        void Update(float dt) override
        {
            if (input->IsKeyPressed(upKey))
                position.y -= speed * dt;
            if (input->IsKeyPressed(downKey))
                position.y += speed * dt;

        }
    };
}
#pragma once 

#include "GameComponent.h"
#include "InputDevice.h"
#include "PaddleComponent.h"
#include "BallComponent.h"
#include "PongRenderComponent.h"

#include <DirectXCollision.h>
#include <SimpleMath.h>
using namespace DirectX::SimpleMath;


namespace game {
    class PongGameComponent : public megaEngine::GameComponent
    {
    public:
        PaddleComponent* left;
        PaddleComponent* right;
        BallComponent* ball;

        int scoreLeft = 0;
        int scoreRight = 0;

        float fieldWidth = 800;
        float fieldHeight = 600;

        megaEngine::PongRenderComponent* renderer = nullptr;

        void Update(float dt) override;

    private:
        void ReflectFromPaddle(PaddleComponent* paddle, const DirectX::BoundingSphere& ballBounds);
        void ResetBall();

    };
}

#include "PongGameComponent.h"
#include <iostream>
#include <cmath>

namespace game {
    using DirectX::BoundingSphere;
    using DirectX::BoundingBox;

    void PongGameComponent::Update(float dt)
    {
        left->position.y = std::clamp(left->position.y,            0.f,             fieldHeight - left->height);

        right->position.y = std::clamp(right->position.y,            0.f,            fieldHeight - right->height);


        // === Границы поля (верх/низ) ===
        if (ball->position.y - ball->radius <= 0.0f)
        {
            ball->position.y = ball->radius;  // Выталкиваем мяч обратно в поле
            ball->velocity.y = std::abs(ball->velocity.y); // Гарантируем движение ВНИЗ (+)
        }

        // === НИЖНЯЯ граница (Y = fieldHeight) ===
        else if (ball->position.y + ball->radius >= fieldHeight)
        {
            ball->position.y = fieldHeight - ball->radius;  // Выталкиваем обратно
            ball->velocity.y = -std::abs(ball->velocity.y); // Гарантируем движение ВВЕРХ (-)
        }

        // === Коллизии с ракетками через DirectXCollision ===
        auto ballBounds = ball->GetBounds();

        BoundingBox leftBox = left->GetBounds();
        if (ballBounds.Intersects(leftBox))
        {
            ReflectFromPaddle(left, ballBounds);
        }

        BoundingBox rightBox = right->GetBounds();
        if (ballBounds.Intersects(rightBox))
        {
            ReflectFromPaddle(right, ballBounds);
        }

        // === Голы ===
        if (ball->position.x < 0)
        {
            scoreRight++;
            std::cout << "Goal Right! " << scoreLeft << " : " << scoreRight << std::endl;

            // Обновляем отображение счёта
            if (renderer) renderer->SetScore(scoreLeft, scoreRight);

            ResetBall();
        }
        else if (ball->position.x > fieldWidth)
        {
            scoreLeft++;
            std::cout << "Goal Left! " << scoreLeft << " : " << scoreRight << std::endl;

            // Обновляем отображение счёта
            if (renderer) renderer->SetScore(scoreLeft, scoreRight);

            ResetBall();
        }
    }

    void PongGameComponent::ReflectFromPaddle(PaddleComponent* paddle, const DirectX::BoundingSphere& ballBounds)
    {
        float relativeIntersectY = paddle->position.y - ball->position.y;
        float normalized = relativeIntersectY / (paddle->height * 0.5f);
        normalized = std::clamp(normalized, -1.0f, 1.0f);

        const float maxAngle = 75.0f * DirectX::XM_PI / 180.0f;
        float bounceAngle = normalized * maxAngle;

        float speed = ball->velocity.Length() * 1.1f;
        speed = (std::min)(speed, 1200.0f);

        int direction = (paddle == left) ? 1 : -1;

        ball->velocity.x = direction * speed * cosf(bounceAngle);
        ball->velocity.y = -speed * sinf(bounceAngle);

        //float overlap = paddle->width  + ball->radius  ;

        //if (direction == 1) {
        //    ball->position.x = paddle->position.x + overlap;
        //}
        //else {
        //    ball->position.x = paddle->position.x - overlap;
        //}

        //float paddleTop = paddle->position.y - paddle->height * 0.5f + ball->radius;
        //float paddleBottom = paddle->position.y + paddle->height * 0.5f - ball->radius;

        //if (ball->position.y < paddleTop) {
        //    ball->position.y = paddleTop + 1.0f;
        //}
        //else if (ball->position.y > paddleBottom) {
        //    ball->position.y = paddleBottom - 1.0f;
        //}
        
    }

    void PongGameComponent::ResetBall()
    {
        ball->position = { fieldWidth * 0.5f, fieldHeight * 0.5f };

        float angle = (rand() % 70 - 35) * 3.14159f / 180.0f; // от -35° до +35°
        int dir = (rand() % 2 == 0) ? 1 : -1;

        ball->velocity = { dir * 300.0f * cos(angle), 300.0f * sin(angle) };
    }
}
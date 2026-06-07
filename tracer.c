#include "raylib.h"
#include <math.h>

#define WIDTH 1000
#define HEIGHT 700
#define NUM_RAYS 1000

typedef struct {
    float x, y;
} Vec2f;

typedef struct {
    Vec2f center;
    float radius;
    Color color;
} Circle;

static Vec2f sub(Vec2f a, Vec2f b)
{
    return (Vec2f){a.x - b.x, a.y - b.y};
}

static Vec2f add(Vec2f a, Vec2f b)
{
    return (Vec2f){a.x + b.x, a.y + b.y};
}

static Vec2f mul(Vec2f a, float s)
{
    return (Vec2f){a.x * s, a.y * s};
}

static float dot(Vec2f a, Vec2f b)
{
    return a.x * b.x + a.y * b.y;
}

static float lengthVec(Vec2f a)
{
    return sqrtf(dot(a, a));
}

static Vec2f normalize(Vec2f a)
{
    float l = lengthVec(a);
    if (l < 0.0001f) return (Vec2f){0, 0};
    return (Vec2f){a.x / l, a.y / l};
}

static float rayCircle(Vec2f ro, Vec2f rd, Circle c)
    {
        Vec2f oc = sub(ro, c.center);

        float b = dot(oc, rd);
        float cc = dot(oc, oc) - c.radius * c.radius;

        float h = b * b - cc;

        if (h < 0.0f)
            return -1.0f;

        h = sqrtf(h);

        float t = -b - h;

        if (t > 0.0f)
            return t;

        t = -b + h;

        if (t > 0.0f)
            return t;

        return -1.0f;
    }

int main(void)
{
    InitWindow(WIDTH, HEIGHT, "2D Ray Tracer");

    SetTargetFPS(60);

    Circle circles[] = {
        {{300, 300}, 90, RED},
        {{550, 250}, 120, GREEN},
        {{700, 500}, 100, BLUE},
        {{420, 520}, 70, YELLOW}
    };

    int circleCount = sizeof(circles) / sizeof(circles[0]);

    while (!WindowShouldClose())
    {
        Vec2f camera = {
            (float)GetMouseX(),
            (float)GetMouseY()
        };

        Vec2f light = {
            WIDTH * 0.8f,
            HEIGHT * 0.2f
        };

        BeginDrawing();

        ClearBackground(BLACK);

        DrawCircleV((Vector2){light.x, light.y}, 10, WHITE);

        for (int r = 0; r < NUM_RAYS; r++)
        {
            float angle = (2.0f * PI * r) / NUM_RAYS;

            Vec2f dir = {
                cosf(angle),
                sinf(angle)
            };

            float nearest = 1e9f;
            int hitIndex = -1;

            for (int i = 0; i < circleCount; i++)
            {
                float t = rayCircle(camera, dir, circles[i]);

                if (t > 0 && t < nearest)
                {
                    nearest = t;
                    hitIndex = i;
                }
            }

            if (hitIndex >= 0)
            {
                Circle c = circles[hitIndex];

                Vec2f hit = add(camera, mul(dir, nearest));

                Vec2f normal =
                    normalize(sub(hit, c.center));

                Vec2f lightDir =
                    normalize(sub(light, hit));

                float brightness =
                    dot(normal, lightDir);

                if (brightness < 0)
                    brightness = 0;

                brightness =
                    0.15f + brightness * 0.85f;

                Color shaded = {
                    (unsigned char)(c.color.r * brightness),
                    (unsigned char)(c.color.g * brightness),
                    (unsigned char)(c.color.b * brightness),
                    255
                };

                DrawLineV(
                    (Vector2){camera.x, camera.y},
                    (Vector2){hit.x, hit.y},
                    Fade(shaded, 0.25f)
                );

                DrawPixelV(
                    (Vector2){hit.x, hit.y},
                    shaded
                );
            }
            else
            {
                DrawLineV(
                    (Vector2){camera.x, camera.y},
                    (Vector2){
                        camera.x + dir.x * 2000,
                        camera.y + dir.y * 2000
                    },
                    Fade(DARKGRAY, 0.03f)
                );
            }
        }

        for (int i = 0; i < circleCount; i++)
        {
            DrawCircleV(
                (Vector2){
                    circles[i].center.x,
                    circles[i].center.y
                },
                circles[i].radius,
                Fade(circles[i].color, 0.25f)
            );

            DrawCircleLines(
                (int)circles[i].center.x,
                (int)circles[i].center.y,
                circles[i].radius,
                circles[i].color
            );
        }

        DrawCircleV(
            (Vector2){camera.x, camera.y},
            6,
            WHITE
        );

        DrawText(
            "Move mouse = camera/ray source",
            10,
            10,
            20,
            WHITE
        );

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
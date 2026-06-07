# 2D Ray Tracer in C

A simple real-time 2D ray tracer written entirely in C.

This program emits thousands of rays from a movable camera position and traces them against a scene composed of circles. Every ray computes its closest intersection, estimates surface lighting using geometric normals, and visualizes the result in real time.

The goal of the project is to demonstrate the core ideas behind ray tracing without requiring complex 3D mathematics, matrices, or rendering APIs.

No game engine.
No physics engine.
No GPU ray tracing.

Just geometry, vectors, and rays.

---

## This Project Focuses On

* Ray casting fundamentals
* Ray-circle intersection testing
* Vector math in 2D
* Surface normal computation
* Lambertian diffuse lighting
* Real-time rendering loops
* Interactive camera movement
* Geometric visibility testing
* Basic rendering pipelines

---

## Features

* Real-time ray tracing
* Interactive camera controlled by mouse
* Multiple circle primitives
* Dynamic point light source
* Lambertian diffuse shading
* Surface normal visualization through lighting
* Closest-hit intersection testing
* Pure geometric rendering
* Written entirely in C

---

# How It Works

For every frame, the renderer emits rays in every direction from the camera position.

Each ray searches for the closest object it intersects.

When an intersection is found, the renderer computes the surface normal, determines the direction to the light source, and calculates how much light reaches that point.

The result is a simple but complete ray tracing pipeline.

---

## Rendering Pipeline

### 1. Camera Position

The camera acts as the origin of all rays.

In this implementation, the camera follows the mouse position:

```c
camera.x = GetMouseX();
camera.y = GetMouseY();
```

Moving the mouse changes where rays originate.

---

### 2. Ray Generation

The renderer emits rays in all directions around the camera.

For every ray:

```c
angle = 2π * i / NUM_RAYS;
```

A direction vector is constructed:

```c
dir.x = cos(angle);
dir.y = sin(angle);
```

Each ray now represents a line extending infinitely through the scene.

---

### 3. Circle Geometry

Objects in the scene are circles:

```c
typedef struct {
    Vec2 center;
    float radius;
} Circle;
```

Every circle is defined by:

* Center position
* Radius
* Surface color

These circles act as the scene geometry.

---

### 4. Ray-Circle Intersection

Every ray is tested against every circle.

A ray intersects a circle whenever:

```text
|P(t) - C|² = r²
```

where:

* P(t) is the ray position
* C is the circle center
* r is the radius

Expanding this equation produces a quadratic.

The discriminant determines whether a hit occurred:

```c
h = b*b - c;
```

If:

```c
h < 0
```

the ray misses the circle.

Otherwise the nearest positive root is selected.

This gives the distance from the camera to the hit point.

---

### 5. Closest Hit Selection

A ray may intersect multiple circles.

The renderer stores:

```c
nearest_distance
```

and updates it whenever a closer intersection is found.

Only the closest object is rendered.

This reproduces visibility naturally without requiring depth buffers.

---

### 6. Hit Point Computation

Once the nearest intersection is known:

```c
hit = origin + direction * distance;
```

This gives the exact location where the ray strikes the object.

All lighting calculations happen at this point.

---

### 7. Surface Normals

A surface normal describes which direction the surface faces.

For a circle:

```c
normal = normalize(hit - center);
```

The normal always points outward from the surface.

Normals are essential for lighting calculations.

---

### 8. Diffuse Lighting

A point light illuminates the scene.

The light direction is:

```c
lightDir = normalize(light - hit);
```

Brightness is computed using Lambert's cosine law:

```c
brightness =
    max(dot(normal, lightDir), 0);
```

This produces realistic shading:

* Surfaces facing the light become brighter
* Surfaces facing away become darker

The effect emerges entirely from vector math.

---

### 9. Color Shading

The final color is scaled by the computed brightness:

```c
finalColor =
    objectColor * brightness;
```

Brighter surfaces retain more of their original color.

Darker surfaces approach black.

This creates the illusion of depth and curvature.

---

### 10. Rendering

Once all rays have been traced:

* Rays are drawn
* Hit points are rendered
* Circles are displayed
* Camera and light source are visualized

The entire process repeats every frame.

---

# Build

Compile using:

```bash
gcc tracer2d.c -o tracer2d -lraylib -lm
```

---

# Run

```bash
./tracer2d
```

Move the mouse to change the ray origin.

The scene updates in real time.

---

# Customizing

Try modifying:

```c
NUM_RAYS
```

Higher values produce smoother lighting.

---

Add more circles:

```c
{ {300, 300}, 80, RED }
```

Experiment with:

* Circle positions
* Circle sizes
* Number of rays
* Light position
* Window resolution

---

# Concepts Practiced

* Ray tracing
* Ray casting
* Ray-circle intersection
* Vector mathematics
* Surface normals
* Diffuse lighting
* Closest-hit visibility
* Real-time rendering
* Interactive graphics
* Computational geometry
* Rendering pipelines

---

# Dependencies

Only:

```text
raylib
math.h
```

No game engine required.

---

# Inspiration

This project is a minimal introduction to ray tracing.

Before reflections, shadows, global illumination, and physically based rendering, every ray tracer begins with a single question:

"What is the first object this ray hits?"

Everything else is built on top of that idea.

The purpose of this project is to make that process visible and easy to understand.


# Real-Time Ray Tracer in C

A terminal-based, full-color ray tracer written entirely in C.

This program renders three reflective spheres on a checkered ground plane under a gradient sky, with soft shadows, one bounce of reflection, and a camera that orbits the scene in real time — all using only standard C and ANSI 256-color escape sequences.

No graphics library. No GPU. Just rays, vectors, and stdout.

This project focuses on learning:

- ray-sphere and ray-plane intersection
- vector math in 3D
- diffuse + shadow shading
- recursive ray tracing for reflections
- camera basis construction
- ANSI 256-color terminal rendering
- frame buffering
- real-time rendering loops in C

---

# Features

- Three reflective spheres on a checkered plane
- Gradient sky with horizon falloff
- Lambertian diffuse shading
- Hard shadows against all spheres
- One bounce of reflection per ray
- Orbiting camera
- Gamma-corrected ANSI 256-color output
- Pure terminal rendering
- Written entirely in C
- Standard libraries only

---

# How It Works

For every pixel in the terminal, the program builds a ray from the camera through that pixel and *traces* it through the scene. It checks the ray against every sphere and the ground plane to find the closest hit, then computes how much light reaches that surface from the scene's single light source.

For every frame:

- Rotate the camera around the scene
- Build a camera basis (forward, right, up)
- For each pixel, construct a ray
- Trace the ray: find the closest intersection
- At the hit point: compute diffuse lighting + shadow + reflection
- Convert the final color to an ANSI 256-color index
- Output the colored pixel as a space with a colored background
- Move the camera forward in its orbit

The animation repeats forever, with the camera continuously sweeping around the scene.

---

# Tutorial / Rendering Pipeline

## 1. Vector Math

Everything is built on a tiny vector library:

```
typedef struct { float x, y, z; } V3;
V3 vadd(V3 a, V3 b);
V3 vsub(V3 a, V3 b);
V3 vmul(V3 a, float s);
float vdot(V3 a, V3 b);
V3 vcross(V3 a, V3 b);
V3 vnorm(V3 a);
V3 vreflect(V3 i, V3 n);
```

3D rendering is mostly just *applying these in the right order*.

---

## 2. Camera Basis

Every frame, the camera position orbits the origin:

```
V3 cam = v3(R * cos(angle), height, R * sin(angle));
```

From the camera position we build an orthonormal basis pointing at the scene:

```
fwd   = normalize(target - cam);
right = normalize(cross(fwd, world_up));
up    = cross(right, fwd);
```

These three vectors define the camera's coordinate system.

---

## 3. Ray Construction

For each pixel `(x, y)`, normalized device coordinates `(u, v)` are mapped through the camera basis:

```
dir = normalize(right*u*FOV + up*v*FOV*aspect + fwd);
```

The `aspect` factor compensates for terminal characters being about twice as tall as they are wide.

---

## 4. Ray-Sphere Intersection

A ray `P(t) = O + t D` hits a sphere of center `C` and radius `r` when:

```
|O + t D - C|^2 = r^2
```

Expanding gives a quadratic in `t` with closed-form solution:

```
oc   = O - C
b    = dot(oc, D)
c    = dot(oc, oc) - r*r
disc = b*b - c
if disc < 0   -> no hit
t    = -b - sqrt(disc)   // take the nearer root
```

---

## 5. Ray-Plane Intersection

The ground plane `y = Y0` is even simpler:

```
t = (Y0 - O.y) / D.y
```

Only valid if `t > 0` and `D.y != 0`.

---

## 6. Shading

For each hit point we compute:

- **Albedo** — the surface color (or checkered pattern for the ground)
- **Diffuse** — `max(0, dot(normal, light_dir))`
- **Shadow** — shoot a ray from the hit point toward the light, see if any sphere blocks it
- **Reflection** — recursively trace a ray bounced off the surface

The final color is:

```
ambient + diffuse * shadow * albedo + reflectivity * reflected_color
```

---

## 7. Sky Gradient

When a ray hits nothing, it samples the sky — a vertical gradient from warm white near the horizon to blue overhead:

```
t = 0.5 * (ray.y + 1.0);
sky = mix(white, blue, t);
```

---

## 8. ANSI 256-Color Output

Each pixel's RGB value is gamma-corrected and quantized to the 6x6x6 RGB cube of ANSI 256 colors:

```
index = 16 + 36 * R + 6 * G + B    (with R, G, B in [0, 5])
```

The pixel is then printed as a space with that color as its background:

```
printf("\x1b[48;5;%dm ", index);
```

Adjacent pixels with the same color share one escape code to keep output fast.

---

## 9. Terminal Rendering

Each frame:

- Move cursor home: `printf("\x1b[H");`
- Render each row into a buffer, flush once per row
- Reset color at the end of each row

This redraws frames in-place to create animation.

---

# Build

Compile using:

```
gcc tracer.c -o tracer -lm
```

Or just:

```
make
```

The `-lm` flag links the math library.

---

# Run

```
./tracer
```

Press `Ctrl+C` to exit.

For best results, run in a 256-color terminal at roughly 80 columns × 32 rows.

---

# Customizing

Edit the constants at the top of `tracer.c`:

- `WIDTH`, `HEIGHT` — resolution (each cell is one terminal character)
- `MAX_DEPTH` — recursion depth for reflections (try 3 or 4)
- `spheres[]` — change colors, positions, sizes, and reflectivity
- `LIGHT` — move the light source

Try adding a fourth sphere, or making one of them a perfect mirror with `reflect = 1.0`.

---

# Concepts Practiced

- Ray tracing
- Ray-primitive intersection (sphere, plane)
- Vector math in 3D
- Phong-style diffuse + shadow shading
- Recursive reflection
- Camera basis construction
- Perspective ray generation
- ANSI 256-color rendering
- Gamma correction
- Frame buffering
- Animation loops
- Terminal graphics
- Real-time rendering

---

# Dependencies

Standard C libraries only:

- `stdio.h`
- `stdlib.h`
- `string.h`
- `math.h`
- `unistd.h`

No graphics engine required.

---

# Inspiration

This is a tiny version of the classic ray tracer from Peter Shirley's *"Ray Tracing in One Weekend"* series, ported to a 256-color terminal.

The whole point: rendering is just math, applied to every pixel, every frame.

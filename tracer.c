/*
 * tracer.c - Real-time terminal ray tracer in C
 *
 * Renders three reflective spheres on a checkered plane with a
 * gradient sky, soft shadows, and one bounce of reflection.
 * Camera orbits the scene. Output is full-color ANSI 256-color.
 *
 * Build:  gcc tracer.c -o tracer -lm
 * Run:    ./tracer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define WIDTH      80
#define HEIGHT     32
#define MAX_DEPTH  2

/* ---------- vector helpers ---------- */
typedef struct { float x, y, z; } V3;
static inline V3 v3(float x, float y, float z) { V3 r = {x, y, z}; return r; }
static inline V3 vadd(V3 a, V3 b) { return v3(a.x+b.x, a.y+b.y, a.z+b.z); }
static inline V3 vsub(V3 a, V3 b) { return v3(a.x-b.x, a.y-b.y, a.z-b.z); }
static inline V3 vmul(V3 a, float s) { return v3(a.x*s, a.y*s, a.z*s); }
static inline V3 vmix(V3 a, V3 b, float t) {
    return v3(a.x*(1-t)+b.x*t, a.y*(1-t)+b.y*t, a.z*(1-t)+b.z*t);
}
static inline float vdot(V3 a, V3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline float vlen(V3 a) { return sqrtf(vdot(a,a)); }
static inline V3 vnorm(V3 a) { float l = vlen(a); return v3(a.x/l, a.y/l, a.z/l); }
static inline V3 vcross(V3 a, V3 b) {
    return v3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}
static inline V3 vreflect(V3 i, V3 n) {
    return vsub(i, vmul(n, 2.0f * vdot(i, n)));
}

/* ---------- scene ---------- */
typedef struct {
    V3   center;
    float r;
    V3   color;
    float reflect;
} Sphere;

#define NSPHERES 3
static Sphere spheres[NSPHERES] = {
    {{ 0.0f,  0.0f,  0.0f}, 1.0f, {0.95f, 0.25f, 0.30f}, 0.30f},   /* red    */
    {{ 2.2f, -0.3f,  0.5f}, 0.7f, {0.25f, 0.90f, 0.35f}, 0.50f},   /* green  */
    {{-1.9f, -0.4f,  1.1f}, 0.6f, {0.30f, 0.45f, 0.95f}, 0.70f},   /* blue   */
};
static const float GROUND_Y = -1.0f;
static const V3    LIGHT    = {4.0f, 6.0f, 4.0f};

/* ---------- intersections ---------- */
static float sphere_hit(Sphere s, V3 ro, V3 rd) {
    V3 oc = vsub(ro, s.center);
    float b = vdot(oc, rd);
    float c = vdot(oc, oc) - s.r * s.r;
    float disc = b * b - c;
    if (disc < 0) return -1.0f;
    float sq = sqrtf(disc);
    float t = -b - sq;
    if (t < 0.001f) t = -b + sq;
    if (t < 0.001f) return -1.0f;
    return t;
}

static float plane_hit(V3 ro, V3 rd) {
    if (fabsf(rd.y) < 1e-5f) return -1.0f;
    float t = (GROUND_Y - ro.y) / rd.y;
    return (t > 0.001f) ? t : -1.0f;
}

/* ---------- shading ---------- */
static V3 sky(V3 rd) {
    float t = 0.5f * (rd.y + 1.0f);
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return vmix(v3(1.00f, 0.95f, 0.90f), v3(0.30f, 0.55f, 0.95f), t);
}

static V3 trace(V3 ro, V3 rd, int depth) {
    if (depth >= MAX_DEPTH) return sky(rd);

    float best = 1e18f;
    int hit_sphere = -1;
    int hit_plane  = 0;

    for (int i = 0; i < NSPHERES; i++) {
        float t = sphere_hit(spheres[i], ro, rd);
        if (t > 0 && t < best) { best = t; hit_sphere = i; hit_plane = 0; }
    }
    {
        float t = plane_hit(ro, rd);
        if (t > 0 && t < best) { best = t; hit_sphere = -1; hit_plane = 1; }
    }
    if (hit_sphere < 0 && !hit_plane) return sky(rd);

    V3 p = vadd(ro, vmul(rd, best));
    V3 n;
    V3 albedo;
    float reflectivity;

    if (hit_sphere >= 0) {
        n = vnorm(vsub(p, spheres[hit_sphere].center));
        albedo = spheres[hit_sphere].color;
        reflectivity = spheres[hit_sphere].reflect;
    } else {
        n = v3(0, 1, 0);
        int cx = (int)floorf(p.x + 1000.0f);
        int cz = (int)floorf(p.z + 1000.0f);
        if ((cx + cz) & 1) albedo = v3(0.85f, 0.85f, 0.85f);
        else               albedo = v3(0.15f, 0.15f, 0.15f);
        reflectivity = 0.15f;
    }

    V3 ldir = vnorm(vsub(LIGHT, p));
    float diff = vdot(n, ldir);
    if (diff < 0) diff = 0;

    /* hard shadow against spheres */
    float shadow = 1.0f;
    V3 sho = vadd(p, vmul(n, 0.001f));
    for (int i = 0; i < NSPHERES; i++) {
        float t = sphere_hit(spheres[i], sho, ldir);
        if (t > 0 && t < vlen(vsub(LIGHT, p))) { shadow = 0.25f; break; }
    }

    V3 lit = vmul(albedo, 0.15f + diff * shadow * 0.85f);

    if (reflectivity > 0.01f && depth + 1 < MAX_DEPTH) {
        V3 rdir = vreflect(rd, n);
        V3 rc = trace(sho, vnorm(rdir), depth + 1);
        lit = vmix(lit, rc, reflectivity);
    }
    return lit;
}

/* ---------- color to ANSI 256 ---------- */
static int rgb_to_ansi(V3 c) {
    float r = c.x, g = c.y, b = c.z;
    /* gamma 2.2 for nicer perceptual range */
    r = powf(r < 0 ? 0 : r > 1 ? 1 : r, 1.0f / 2.2f);
    g = powf(g < 0 ? 0 : g > 1 ? 1 : g, 1.0f / 2.2f);
    b = powf(b < 0 ? 0 : b > 1 ? 1 : b, 1.0f / 2.2f);
    int ri = (int)(r * 5.0f + 0.5f);
    int gi = (int)(g * 5.0f + 0.5f);
    int bi = (int)(b * 5.0f + 0.5f);
    return 16 + 36 * ri + 6 * gi + bi;
}

/* ---------- main loop ---------- */
int main(void) {
    printf("\x1b[?25l\x1b[2J");
    float angle = 0.0f;
    int prev = -1;
    char line[WIDTH * 24 + 32];

    for (;;) {
        V3 cam = v3(4.5f * cosf(angle), 1.8f, 4.5f * sinf(angle));
        V3 fwd = vnorm(vsub(v3(0, 0, 0.3f), cam));
        V3 right = vnorm(vcross(fwd, v3(0, 1, 0)));
        V3 up = vcross(right, fwd);
        const float FOV = 1.0f;
        const float ASPECT_CHAR = 0.5f;   /* terminal chars are ~2x tall */

        printf("\x1b[H");

        for (int y = 0; y < HEIGHT; y++) {
            prev = -1;
            char *q = line;
            for (int x = 0; x < WIDTH; x++) {
                float u = (float)(x - WIDTH/2) / (WIDTH/2);
                float v = (float)(HEIGHT/2 - y) / (HEIGHT/2);
                V3 dir = vnorm(vadd(vadd(vmul(right, u * FOV),
                                         vmul(up,    v * FOV * ASPECT_CHAR)),
                                    fwd));
                V3 c = trace(cam, dir, 0);
                int col = rgb_to_ansi(c);
                if (col != prev) {
                    q += sprintf(q, "\x1b[48;5;%dm", col);
                    prev = col;
                }
                *q++ = ' ';
            }
            q += sprintf(q, "\x1b[0m\n");
            fwrite(line, 1, q - line, stdout);
        }
        fflush(stdout);

        angle += 0.04f;
        usleep(20000);
    }
    return 0;
}

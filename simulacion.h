#pragma once
#include <algorithm>
#include <cmath>

namespace sim {
const float PI = 3.14159265359f;
inline float limitar(float n, float a = 0, float b = 1) { return std::max(a, std::min(b, n)); }
inline float mezclar(float a, float b, float t) { return a + (b - a) * t; }
struct Color { float r, g, b; };
inline Color mezcla(Color a, Color b, float t) {
    return {mezclar(a.r,b.r,t), mezclar(a.g,b.g,t), mezclar(a.b,b.b,t)};
}
struct Tren {
    float z, velocidad, puertas, restante;
    int sentido, estacion; // 0: Central, 1: Parque; -1: en recorrido.
};
// Ciclo de 72 s: parada 9 s, viaje 27 s, parada 9 s, regreso 27 s.
// Smoothstep da posicion y velocidad continuas, con frenado en ambas paradas.
inline Tren evaluarTren(double tiempo) {
    float t = static_cast<float>(std::fmod(tiempo, 72.0));
    if (t < 0) t += 72;
    const bool regreso = t >= 36;
    const float fase = regreso ? t - 36 : t;
    const float desde = regreso ? 150.0f : -50.0f;
    const float hasta = regreso ? -50.0f : 150.0f;
    if (fase < 9) {
        const float puerta = std::min(limitar((fase - .6f) / 1.2f), limitar((8.2f - fase) / 1.2f));
        return {desde, 0, puerta, 9 - fase, regreso ? -1 : 1, regreso ? 1 : 0};
    }
    const float u = (fase - 9) / 27;
    return {mezclar(desde, hasta, u*u*(3-2*u)), (hasta-desde)*6*u*(1-u)/27,
            0, 36-fase, regreso ? -1 : 1, -1};
}
struct Ambiente { Color cielo, sol, ambiente; float dia, lamparas; };
inline Ambiente ambiente(float hora) {
    const float h[] = {0,5,7,10,16,18.5f,20,24};
    const Color cielo[] = {{.025f,.04f,.095f},{.05f,.07f,.15f},{.73f,.57f,.43f},
        {.49f,.72f,.81f},{.53f,.73f,.82f},{.65f,.34f,.27f},{.055f,.07f,.16f},{.025f,.04f,.095f}};
    const Color sol[] = {{.12f,.17f,.29f},{.15f,.19f,.29f},{.95f,.62f,.35f},
        {.91f,.87f,.73f},{.95f,.83f,.64f},{1,.47f,.22f},{.13f,.17f,.30f},{.12f,.17f,.29f}};
    const float dia[] = {0,0,.60f,1,1,.35f,0,0};
    int i = 0; while (i < 6 && hora > h[i+1]) ++i;
    const float t = limitar((hora-h[i])/(h[i+1]-h[i]));
    const float d = mezclar(dia[i],dia[i+1],t);
    return {mezcla(cielo[i],cielo[i+1],t),mezcla(sol[i],sol[i+1],t),
        mezcla({.14f,.17f,.25f},{.39f,.40f,.38f},d), d, 1-limitar((d-.2f)/.55f)};
}
}

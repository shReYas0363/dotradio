#pragma once
#include <stdbool.h>

typedef struct {
    float x;
    float y;
    float z;
} Vector3D;

typedef struct {
    float sx;
    float sy;
    float depth;
    bool visible;
} ScreenPoint;

/* Precomputed sin/cos for one pitch/yaw pair. */
typedef struct {
    float cx, sx;
    float cy, sy;
} Rotation;

/* Globe Point to a Unit Sphere */
Vector3D geoToSphere(float latitude, float longitude);

Rotation rotation_make(float pitch_rad, float yaw_rad);
Vector3D rotateXYZ(Vector3D point, const Rotation *rot);

ScreenPoint projectToScreen(Vector3D cam_point, int width, int height, float globe_radius_px);

bool hit_test_point(ScreenPoint point, float mx, float my, float hit_radius_px);

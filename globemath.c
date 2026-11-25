#include "globemath.h"
#include <math.h>

#define PI 3.14159265358979323846f

static float deg2rad(float degrees) {
    return degrees * (PI / 180.0f);
}

Vector3D geoToSphere(float latitude, float longitude) {
    float lat_rad = deg2rad(latitude);
    float lon_rad = deg2rad(longitude);

    Vector3D point;
    point.x = cosf(lat_rad) * cosf(lon_rad);
    point.y = sinf(lat_rad);
    point.z = cosf(lat_rad) * sinf(lon_rad);

    return point;
}

Rotation rotation_make(float pitch_rad, float yaw_rad) {
    Rotation rot;
    rot.cx = cosf(pitch_rad);
    rot.sx = sinf(pitch_rad);
    rot.cy = cosf(yaw_rad);
    rot.sy = sinf(yaw_rad);
    return rot;
}

Vector3D rotateXYZ(Vector3D p, const Rotation *rot) {
    float y1 = p.y * rot->cx - p.z * rot->sx;
    float z1 = p.y * rot->sx + p.z * rot->cx;
    float x1 = p.x;

    Vector3D out;
    out.x = x1 * rot->cy + z1 * rot->sy;
    out.y = y1;
    out.z = -x1 * rot->sy + z1 * rot->cy;
    return out;
}

ScreenPoint projectToScreen(Vector3D cam_pt, int width, int height, float globe_radius_px) {
    float cx = (float)width * 0.5f;
    float cy = (float)height * 0.5f;

    ScreenPoint out;
    out.sx = cx + cam_pt.x * globe_radius_px;
    out.sy = cy - cam_pt.y * globe_radius_px;
    out.depth = cam_pt.z;
    out.visible = (cam_pt.z > 0.0f);
    return out;
}

bool hit_test_point(ScreenPoint pt, float mx, float my, float hit_radius_px) {
    if (!pt.visible || hit_radius_px <= 0.0f) {
        return false;
    }

    float dx = pt.sx - mx;
    float dy = pt.sy - my;
    float r2 = hit_radius_px * hit_radius_px;

    return (dx * dx + dy * dy) <= r2;
}

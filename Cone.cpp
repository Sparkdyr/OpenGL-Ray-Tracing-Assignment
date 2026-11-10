#include "Cone.h"
#include <math.h>
#include <cmath>

float Cone::intersect(glm::vec3 p0, glm::vec3 dir) {

    float k = radius / height;
    float k_sq = k * k;

    float vdx = p0.x - center.x;
    float vdz = p0.z - center.z;
    float vdy = height - p0.y + center.y;

    float A = (dir.x * dir.x) + (dir.z * dir.z) - (k_sq * dir.y * dir.y);
    float B = 2.0f * ((dir.x * vdx) + (dir.z * vdz) + (k_sq * dir.y * vdy));
    float C = (vdx * vdx) + (vdz * vdz) - (k_sq * vdy * vdy);

    float delta = (B * B) - (4.0f * A * C);
    if (delta < 0.001f) {
        return -1.0f;
    }

    float t1 = (-B - sqrt(delta)) / (2.0f * A);
    float t2 = (-B + sqrt(delta)) / (2.0f * A);

    float hit_height1 = p0.y + t1 * dir.y;
    float hit_height2 = p0.y + t2 * dir.y;

    if ((t1 > 0) && (hit_height1 >= center.y) && (hit_height1 <= center.y + height)) {
        return t1;
    } else if ((t2 > 0) && (hit_height2 >= center.y) && (hit_height2 <= center.y + height)) {
        return t2;
    }
    return -1.0f;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cone::normal(glm::vec3 p) {
	glm::vec3 n;
    float alpha = std::atan2((p.x-center.x), (p.z-center.z));
    float theta = std::atan(radius/height);

    // make sure alpha between -pi and pi
    n = glm::vec3((std::sin(alpha) * std::cos(theta),
         std::sin(theta),
         std::cos(alpha) * std::cos(theta)));
    
    return glm::normalize(n);
}

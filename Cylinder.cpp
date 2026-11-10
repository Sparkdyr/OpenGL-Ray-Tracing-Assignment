#include "Cylinder.h"
#include <math.h>

float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir) {

    float A = (dir.x * dir.x) + (dir.z * dir.z);
    float B = 2.0f * (dir.x * (p0.x - center.x) + dir.z * (p0.z - center.z));
    float C = ((p0.x - center.x) * (p0.x - center.x)) + 
            ((p0.z - center.z) * (p0.z - center.z)) - 
            (radius * radius);
    float delta = (B * B) - (4.0f * A * C);
    
    if (delta < 0.0f) {
        return -1.0f; // Includes zero and negative values
    }

    float t1 = (-B - sqrt(delta)) / (2.0f * A);
    float t2 = (-B + sqrt(delta)) / (2.0f * A);

    float hit_height1 = p0.y + (t1*dir.y);
    float hit_height2 = p0.y + (t2*dir.y);

    if (t1 > 0 && hit_height1 <= center.y + height && hit_height1 >= center.y) {
        return t1;
    }
    if (t2 > 0 && hit_height2 <= center.y + height && hit_height2 >= center.y) {
        return (center.y + height - p0.y) / dir.y;
    }
    return -1.0f;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cylinder::normal(glm::vec3 p) {
	glm::vec3 n;

    n = glm::vec3(p.x, 0, p.z) - center;
    
    if (p.y >= center.y + height) {
        n = glm::vec3(0, 1, 0);
    } else if (p.y <= center.y) {
        n = glm::vec3(0, -1, 0);
    }
    n = glm::normalize(n);
    return n;
}

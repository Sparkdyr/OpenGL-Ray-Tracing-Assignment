/*==================================================================================
* COSC 363  Computer Graphics
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf   for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Cylinder.h"
#include "Cone.h"
using namespace std;

const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -10.0;
const float XMAX = 10.0;
const float YMIN = -10.0;
const float YMAX = 10.0;
const float PI = 3.14159265359;
const int AA_SAMPLES = 2;

vector<SceneObject*> sceneObjects;
TextureBMP texture;

//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step) {
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(29.9, 29.9, 1.0);					//Light's position
	glm::vec3 lightPos1(-29.9, 29.9, 1.0);
	glm::vec3 color(0);
	SceneObject* obj;

	ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
	if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	if (ray.index == 1) { // cyan sphere
		glm::vec3 objNorm = (obj->normal(ray.hit));
		float texcoords = 0.5 + std::atan2(-objNorm[0], -objNorm[2])/(2*PI);
		float texcoordt = 0.5 - std::asin(-objNorm[1])/PI;
		if(texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1) {
			color = texture.getColorAt(texcoords, texcoordt);
			obj->setColor(color);
		}
	}

	glm::vec3 lighting1 = obj->lighting(lightPos, -ray.dir, ray.hit);
	glm::vec3 lighting2 = obj->lighting(lightPos1, -ray.dir, ray.hit);

	glm::vec3 ambient = 0.2f * obj->getColor();
	color = ambient;

	//light1
	glm::vec3 lightVec = lightPos - ray.hit;
	float lightDist = glm::length(lightVec);
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);

	if ((shadowRay.index > -1) && (shadowRay.dist < lightDist)) {
		SceneObject* blocker = sceneObjects[shadowRay.index];

		if (blocker->isTransparent() || blocker->isRefractive()) {
        	color += (lighting1 - ambient) * 0.7f;
    	}
	} else {
		color += lighting1 - ambient;
	}

	//light 2
	glm::vec3 lightVec1 = lightPos1 - ray.hit;
	float lightDist1 = glm::length(lightVec1);
	Ray shadowRay1(ray.hit, lightVec1);
	shadowRay1.closestPt(sceneObjects);

	if ((shadowRay1.index > -1) && (shadowRay1.dist < lightDist1)) {
		SceneObject* blocker = sceneObjects[shadowRay1.index];

		if (blocker->isTransparent() || blocker->isRefractive()) {
			color += (lighting2 - ambient) * 0.7f;
		}
	} else {
		color += lighting2 - ambient;
	}


	if (obj->isReflective() && step < MAX_STEPS) {
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}

	if (obj->isRefractive() && step < MAX_STEPS) {
		float refrc = obj->getRefractionCoeff();
		float refri = obj->getRefractiveIndex();
		glm::vec3 n = obj->normal(ray.hit);
		glm::vec3 g = glm::refract(ray.dir, n, 1/refri);
		Ray refrRay(ray.hit, g);
		refrRay.closestPt(sceneObjects);
		glm::vec3 m = obj->normal(refrRay.hit);
		glm::vec3 h = glm::refract(g, -m, refri/1);
		Ray refrRay1(refrRay.hit, h);
		glm::vec3 refractedColor = trace(refrRay1, step + 1);
		color = color + (refrc * refractedColor);

	}

	if (obj->isTransparent() && step < MAX_STEPS) {
		float tranc = obj->getTransparencyCoeff();
		Ray tranRay(ray.hit, ray.dir);
		glm::vec3 tranColor = trace(tranRay, step + 1);
		color = color + (tranc * tranColor);
	}
	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display() {
	float xp, yp;  //grid point
	float cellX = (XMAX - XMIN) / NUMDIV;  //cell width
	float cellY = (YMAX - YMIN) / NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for (int i = 0; i < NUMDIV; i++) {	//Scan every cell of the image plane
		xp = XMIN + i * cellX;
		for (int j = 0; j < NUMDIV; j++) {
			yp = YMIN + j * cellY;
			// //Non Anti-Aliasing
			// glm::vec3 dir(xp + 0.5 * cellX, yp + 0.5 * cellY, -EDIST);	//direction of the primary ray

			// Ray ray = Ray(eye, dir);

			// glm::vec3 col = trace(ray, 1); //Trace the primary ray and get the colour value

			//Anti-Aliasing
			glm::vec3 col = glm::vec3(0.0f);

			for (int a = 0; a < AA_SAMPLES; a++) {
				for (int b = 0; b < AA_SAMPLES; b++) {
					float offsetX = (a + 0.5f) / AA_SAMPLES;
					float offsetY = (b + 0.5f) / AA_SAMPLES;

					glm::vec3 dir(xp + offsetX * cellX, yp + offsetY * cellY, -EDIST);
					Ray ray(eye, dir);
					col += trace(ray, 1);
				}
			}

			col /= float(AA_SAMPLES * AA_SAMPLES);

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp + cellX, yp);
			glVertex2f(xp + cellX, yp + cellY);
			glVertex2f(xp, yp + cellY);
		}
	}

	glEnd();
	glFlush();
}

//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL 2D orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize() {
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

	glClearColor(0, 0, 0, 1);

	texture = TextureBMP("Earth.bmp");

	Sphere *sphere1 = new Sphere(glm::vec3(-15.0, -15, -110.0), 5.0);
	sphere1->setColor(glm::vec3(0, 0, 0));   //Reflective
	sphere1->setReflectivity(true, 0.8);
	sceneObjects.push_back(sphere1);

	Sphere *sphere2 = new Sphere(glm::vec3(0.0, -13, -110.0), 3.0);
	sphere2->setColor(glm::vec3(0, 1, 1));   //Textured
	sceneObjects.push_back(sphere2);

	Sphere *sphere3 = new Sphere(glm::vec3(0, -17, -90.0), 3.0);
	sphere3->setColor(glm::vec3(0.0, 0, 0));   //Transparent
	sphere3->setTransparency(true, 0.6);
	sceneObjects.push_back(sphere3);

	Sphere *sphere4 = new Sphere(glm::vec3(15.0, -15, -110.0), 5.0);
	sphere4->setColor(glm::vec3(0, 0, 0));   //Refractive
	sphere4->setRefractivity(true, 0.6, 1.5);
	sceneObjects.push_back(sphere4);

	Cylinder *cylinder1 = new Cylinder(glm::vec3(-7.5, -20, -110.0), 2.0, 7.0);
	cylinder1->setColor(glm::vec3(0, 0.8, 0.2));
	sceneObjects.push_back(cylinder1);

	Cylinder *cylinder2 = new Cylinder(glm::vec3(7.5, -20, -110.0), 2.0, 7.0);
	cylinder2->setColor(glm::vec3(0, 0.8, 0.2));
	cylinder1->setSpecularity(true);
	sceneObjects.push_back(cylinder2);

	Cone *cone1 = new Cone(glm::vec3(0, -20, -110.0), 3.0, 4.0);
	cone1->setColor(glm::vec3(0.2, 0.5, 0.7));
	sceneObjects.push_back(cone1);

	Plane *mirror = new Plane (glm::vec3(-20., -20, -179.9), //Point A
								glm::vec3(20., -20, -179.9), //Point B
								glm::vec3(20., 10, -179.9), //Point C
								glm::vec3(-20., 10, -179.9)); //Point D
	mirror->setColor(glm::vec3(0, 0, 0));
	mirror->setReflectivity(true);
	mirror->setSpecularity(false);
	sceneObjects.push_back(mirror);
	
	//-------------------------------------------Box Planes---------------------------------------------------------------------------------------------

	Plane *floor_plane = new Plane (glm::vec3(-30., -20, 1.1), //Point A
									glm::vec3(30., -20, 1.1), //Point B
									glm::vec3(30., -20, -180), //Point C
									glm::vec3(-30., -20, -180)); //Point D
	floor_plane->setColor(glm::vec3(0.8, 0.8, 0));
	floor_plane->setSpecularity(false);
	sceneObjects.push_back(floor_plane);
	
	Plane *left_plane = new Plane (glm::vec3(-30., -20, 1.1), //Point A
								   glm::vec3(-30., -20, -180), //Point B
								   glm::vec3(-30., 30, -180), //Point C
								   glm::vec3(-30., 30, 1.1)); //Point D
	left_plane->setColor(glm::vec3(0, 0.8, 0.8));
	left_plane->setSpecularity(false);
	sceneObjects.push_back(left_plane);

	Plane *back_plane = new Plane (glm::vec3(-30., -20, -180), //Point A
								   glm::vec3(30., -20, -180), //Point B
								   glm::vec3(30., 30, -180), //Point C
								   glm::vec3(-30., 30, -180)); //Point D
	back_plane->setColor(glm::vec3(0.5, 0.25, 0));
	back_plane->setSpecularity(false);
	sceneObjects.push_back(back_plane);

	Plane *right_plane = new Plane (glm::vec3(30., -20, -180), //Point A
									glm::vec3(30., -20, 1.1), //Point B
									glm::vec3(30., 30, 1.1), //Point C
									glm::vec3(30., 30, -180)); //Point D
	right_plane->setColor(glm::vec3(0.8, 0, 0));
	right_plane->setSpecularity(false);
	sceneObjects.push_back(right_plane);

	Plane *front_plane = new Plane (glm::vec3(-30., -20, 1.1), //Point A
									glm::vec3(30., -20, 1.1), //Point B
									glm::vec3(30., 30, 1.1), //Point C
									glm::vec3(-30., 30, 1.1)); //Point D
	front_plane->setColor(glm::vec3(0, 1, 0.2));
	front_plane->setSpecularity(false);
	sceneObjects.push_back(front_plane);

	Plane *roof_plane = new Plane (glm::vec3(-30., 30, 1.1), //Point A
								   glm::vec3(30., 30, 1.1), //Point B
								   glm::vec3(30., 30, -180), //Point C
								   glm::vec3(-30., 30, -180)); //Point D
	roof_plane->setColor(glm::vec3(0.8, 0, 0.8));
	roof_plane->setSpecularity(false);
	sceneObjects.push_back(roof_plane);

}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(20, 20);
	glutCreateWindow("Raytracing");

	glutDisplayFunc(display);
	initialize();

	glutMainLoop();
	return 0;
}

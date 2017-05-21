// ComputerGraphics.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <iostream>
#include "Vec3.h"

using namespace std;

struct Rectangle {
	vector<Vec3<float>> points; //topLeft, topRight, bottomLeft, bottomRight;
	Vec3<float> normal;
	int color;
};

struct Ray {
	Vec3<float> startPoint;
	Vec3<float> direction;
	float length;
};

struct PointLight {
	Vec3<float> location;
	float intensity;
	float maxDistance;
};


//Check if point is within polygon
bool pointInPoly(Rectangle &obj, Vec3<float> &point) {
	bool inside = false;
	int absX = abs(obj.normal.x), absY = abs(obj.normal.y), absZ = abs(obj.normal.z);
	//Project 3D plane into 2D by omitting dominant coordinate to maximize remaining area
	//(Y, Z) Plane Projection
	if (absX > absY && absX > absZ) {
		for (int i = 0, j = obj.points.size() - 1; i < obj.points.size(); j = i++) {
			if (((obj.points[i].z > point.z) != (obj.points[j].z > point.z)) && //test for whether or not the edge crosses the ray starting at our test point)
				(point.y < (obj.points[j].y - obj.points[i].y) * (point.z - obj.points[i].z) / (obj.points[j].z - obj.points[i].z) + obj.points[i].y))
				inside = !inside;
		}
		return inside;
	}
	//(X, Z) Plane Projection
	else if (absY > absX && absY > absZ) {
		for (int i = 0, j = obj.points.size() - 1; i < obj.points.size(); j = i++) {
			if (((obj.points[i].z > point.z) != (obj.points[j].z > point.z)) && //test for whether or not the edge crosses the ray starting at our test point)
				(point.x < (obj.points[j].x - obj.points[i].x) * (point.z - obj.points[i].z) / (obj.points[j].z - obj.points[i].z) + obj.points[i].x))
				inside = !inside;
		}
		return inside;
	}
	//(X, Y) Plane Projection
	else {
		for (int i = 0, j = obj.points.size() - 1; i < obj.points.size(); j = i++) {
			if (((obj.points[i].y > point.y) != (obj.points[j].y > point.y)) && //test for whether or not the edge crosses the ray starting at our test point)
				(point.x < (obj.points[j].x - obj.points[i].x) * (point.y - obj.points[i].y) / (obj.points[j].y - obj.points[i].y) + obj.points[i].x))
				inside = !inside;
		}
		return inside;
	}
}

//Currently Calculates Intersect with infinite plane of polygon, need to check bounds of polygon first!
bool intersect(Rectangle &obj, Ray primaryRay, Vec3<float> *pHit) {
	float denom = primaryRay.direction.dotProd(obj.normal);

	if (abs(denom) > 0.0001) /*may need to fix for float point error*/ {
		float num = (obj.points[0] - primaryRay.startPoint).dotProd(obj.normal);
		float dist = num / denom;
		pHit = &(primaryRay.startPoint + primaryRay.direction * dist); //May be deallocated and cause pointer issues
		return pointInPoly(obj, *pHit);
	}
	else 
		return false;
}

void raytrace() {
	//Camera is at origin point
	Vec3<float> camera = Vec3<float>(0);

	//Object is 1 Rectangle Plane 10 units in front in the Z direction
	vector<Rectangle> objects(1);
	for (int k = 0; k < objects.size(); k++){
		Rectangle *r = &objects[k];
		//r->points = vector<Vec3<float>>(4);
		//r->points[0] = Vec3<float>(-10, 10, 1.5);
		//r->points[1] = Vec3<float>(10, 10, 1.5);
		//r->points[2] = Vec3<float>(10, -10, 1.5);
		//r->points[3] = Vec3<float>(-10, -10, 1.5);
		r->points = vector<Vec3<float>>(3);
		r->points[0] = Vec3<float>(-10, -10, 1.5);
		r->points[1] = Vec3<float>(10, -10, 1.5);
		r->points[2] = Vec3<float>(0, 20, 1.5);
		r->normal = ((r->points[0] - r->points[1]).crossProd(r->points[1] - r->points[2])).normalize();
		r->color = 1;
	}


	Vec3<float> currPixel;
	Ray primaryRay;
	Ray shadowRay;
	PointLight sourceLight;
	sourceLight.location = Vec3<float>(0, 30, 3);
	sourceLight.intensity = 2;
	sourceLight.maxDistance = 100;
	//Viewing plane is 80x80 and 1 unit in front camera in the Z direction
	for (int j = 40; j > -40; j--) {
		for (int i = -40; i < 40; i++) {
			currPixel = Vec3<float>(i, j, 2);
			primaryRay.startPoint = camera;
			primaryRay.direction = (currPixel - primaryRay.startPoint).normalize();
			primaryRay.length = 100;

			Vec3<float> pHit;
			float minDist = INFINITY;
			Rectangle *hit = NULL;
			for (int k = 0; k < objects.size(); k++) {
				if (intersect(objects[k], primaryRay, &pHit)) {
					float dist = (camera - pHit).length();
					if (dist < minDist) {
						hit = &objects[k];
						minDist = dist;
					}
				}
			}

			if (hit != NULL) {
				shadowRay.startPoint = pHit;
				shadowRay.direction = (sourceLight.location - pHit).normalize();
				bool shadowed = false;
				for (int k = 0; k < objects.size(); k++) {
					if (hit != &objects[k] && intersect(objects[k], shadowRay, &pHit)) {
						shadowed = true;
						break;
					}
				}

				if (!shadowed)
					cout << hit->color; //Later on when bmp works remove if and hit->color * (sourceLight.maxDistance - (sourceLight.location - pHit).length) / sourceLight.maxDistance
				else
					cout << "0";
			}
			else
				cout << "0";
		}
		cout << endl;
	}

}

int main()
{
	raytrace();
	return 0;
}

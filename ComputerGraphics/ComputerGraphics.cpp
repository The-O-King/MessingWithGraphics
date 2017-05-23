// ComputerGraphics.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <iostream>
#include "Vec3.h"
#include "EasyBMP/EasyBMP.h"

using namespace std;

struct Rectangle {
	vector<Vec3<float>> points; //topLeft, topRight, bottomLeft, bottomRight;
	Vec3<float> normal;
	RGBApixel color;
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
bool intersect(Rectangle &obj, Ray primaryRay, Vec3<float> &pHit) {
	float denom = primaryRay.direction.dotProd(obj.normal);

	if (abs(denom) > 0.0001) /*may need to fix for float point error*/ {
		float num = (obj.points[0] - primaryRay.startPoint).dotProd(obj.normal);
		float dist = num / denom;
		pHit = primaryRay.startPoint + primaryRay.direction * dist; //May be deallocated and cause pointer issues
		return pointInPoly(obj, pHit);
	}
	else 
		return false;
}

void raytrace() {
	BMP output;
	output.SetSize(1920, 1080);
	output.SetBitDepth(24);
	//Camera is at origin point
	Vec3<float> camera = Vec3<float>(0);

	//Object is 1 Rectangle Plane 10 units in front in the Z direction
	vector<Rectangle> objects(1);
	for (int k = 0; k < objects.size(); k++){
		Rectangle *r = &objects[k];
		r->points = vector<Vec3<float>>(4);
		r->points[0] = Vec3<float>(-500, 500, 10);
		r->points[1] = Vec3<float>(500, 500, 10);
		r->points[2] = Vec3<float>(500, -500, 1.5);
		r->points[3] = Vec3<float>(-500, -500, 1.5);

		//r->points = vector<Vec3<float>>(3);
		//r->points[0] = Vec3<float>(-50, -10, 1.5);
		//r->points[1] = Vec3<float>(50, -10, 5);
		//r->points[2] = Vec3<float>(0, 75, 1.5);

		r->normal = ((r->points[1] - r->points[0]).crossProd(r->points[2] - r->points[1])).normalize();
		r->color.Red = 255;
		r->color.Green = 0;
		r->color.Blue = 0;
	}


	Vec3<float> currPixel;
	Ray primaryRay;
	Ray shadowRay;
	PointLight sourceLight;
	RGBApixel currColor;
	sourceLight.location = Vec3<float>(0, 0, 0);
	sourceLight.intensity = 2;
	sourceLight.maxDistance = 500;
	//Viewing plane is 1920x1080 and 2 unit in front camera in the Z direction
	for (int j = 0; j < 1080 ; j++) {
		for (int i = 0; i < 1920; i++) {

			currPixel = Vec3<float>(i - 1920 / 2, 1080/2 - j, 2);
			primaryRay.startPoint = camera;
			primaryRay.direction = (currPixel - primaryRay.startPoint).normalize();
			primaryRay.length = 100;

			Vec3<float> pHit;
			float minDist = INFINITY;
			Rectangle *hit = NULL;
			for (int k = 0; k < objects.size(); k++) {
				if (intersect(objects[k], primaryRay, pHit)) {
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
					if (hit != &objects[k] && intersect(objects[k], shadowRay, pHit)) {
						shadowed = true;
						break;
					}
				}
				
				if (!shadowed) {
					float intensity = (sourceLight.maxDistance - (sourceLight.location - pHit).length()) / sourceLight.maxDistance;
					intensity = 0 > intensity ? 0 : intensity;
					currColor.Red = hit->color.Red * intensity;
					currColor.Green = hit->color.Green * intensity;
					currColor.Blue = hit->color.Blue * intensity;
					output.SetPixel(i, j, currColor);
				}
				else {
					currColor.Red = 0;
					currColor.Green = 0;
					currColor.Blue = 0;
					output.SetPixel(i, j, currColor);
				}
			}
			else {
				currColor.Red = 0;
				currColor.Green = 0;
				currColor.Blue = 0;
				output.SetPixel(i, j, currColor);
			}
		}
	}
	output.WriteToFile("Test.bmp");
}

int main()
{
	raytrace();
	return 0;
}

// RayTrace.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <vector>
#include <iostream>
#include "Vec3.h"
#include "EasyBMP/EasyBMP.h"

using namespace std;

struct Polygon {
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
bool pointInPoly(Polygon &obj, Vec3<float> &point) {
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
bool intersect(Polygon &obj, Ray primaryRay, Vec3<float> &pHit, float &dist) {
	float denom = primaryRay.direction.dotProd(obj.normal);

	if (abs(denom) > 1e-6) /*may need to fix for float point error*/ {
		float num = (obj.points[0] - primaryRay.startPoint).dotProd(obj.normal);
		dist = num / denom;
		pHit = primaryRay.startPoint + primaryRay.direction * dist; //May be deallocated and cause pointer issues
		return pointInPoly(obj, pHit);
	}
	else
		return false;
}

void raytrace(Vec3<float> &camera, vector<Polygon> &objects, vector<PointLight> &lights) {
	BMP output;
	output.SetSize(1280, 720);
	output.SetBitDepth(24);

	Vec3<float> currPixel;
	Ray primaryRay;
	Ray shadowRay;
	PointLight sourceLight = lights[0]; //currently only takes into account single light source (the first)
	RGBApixel currColor;

	//Viewing plane is 4x4 and 1 unit in front camera in the Z direction
	for (int j = 0; j < 720; j++) {
		for (int i = 0; i < 1280; i++) {
			//Convert pixel coordinate in screen space to world space
			currPixel = Vec3<float>(10.0 * i / 1280 - 5, 5 - 10.0 * j / 720, 1);
			primaryRay.startPoint = camera;
			primaryRay.direction = (currPixel - primaryRay.startPoint).normalize();
			primaryRay.length = 100;

			Vec3<float> pHit;
			Vec3<float> currHit;
			float currDist;
			float minDist = INFINITY;
			Polygon *hit = NULL;
			for (int k = 0; k < objects.size(); k++) {
				if (intersect(objects[k], primaryRay, currHit, currDist)) {
					if (currDist < minDist) {
						hit = &objects[k];
						pHit = currHit;
						minDist = currDist;
					}
				}
			}

			if (hit != NULL) {
				shadowRay.startPoint = pHit;
				shadowRay.direction = (sourceLight.location - pHit).normalize();
				bool shadowed = false;
				for (int k = 0; k < objects.size(); k++) {
					if (hit != &objects[k] && intersect(objects[k], shadowRay, pHit, currDist)) {
						if (currDist > 1e-6) {
							shadowed = true;
							break;
						}
					}
				}

				if (!shadowed) {
					float intensity = (sourceLight.maxDistance - (sourceLight.location - shadowRay.startPoint).length()) / sourceLight.maxDistance;
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
				currColor.Red = 127;
				currColor.Green = 127;
				currColor.Blue = 127;
				output.SetPixel(i, j, currColor);
			}
		}
	}
	output.WriteToFile("Test.bmp");
}

int main()
{
	int numLights, numPoly;
	Vec3<float> camera;


	cout << "Enter camera location (x, y, z): ";
	cin >> camera.x;
	cin >> camera.y;
	cin >> camera.z;

	cout << "Enter the number of polygons: ";
	cin >> numPoly;
	vector<Polygon> scenePolygons(numPoly);
	for (int x = 0; x < numPoly; x++) {
		Polygon *r = &scenePolygons[x];
		int numPoints;
		cout << "Enter the number of points: ";
		cin >> numPoints;
		r->points = vector<Vec3<float>>(numPoints);
		for (int y = 0; y < numPoints; y++) {
			cout << "Enter the point (x, y, z): ";
			r->points[y] = Vec3<float>();
			cin >> r->points[y].x;
			cin >> r->points[y].y;
			cin >> r->points[y].z;
		}
		r->normal = ((r->points[0] - r->points[1]).crossProd(r->points[2] - r->points[1])).normalize();
		cout << "Enter the color (R, G, B): ";
		int tmpColor;
		cin >> tmpColor;
		r->color.Red = tmpColor;
		cin >> tmpColor;
		r->color.Green = tmpColor;
		cin >> tmpColor;
		r->color.Blue = tmpColor;
	}

	cout << "Enter the number of lights: ";
	cin >> numLights;
	vector<PointLight> sceneLights(numLights);
	for (int x = 0; x < numLights; x++) {
		cout << "Enter Light Location (x, y, z): ";
		cin >> sceneLights[x].location.x;
		cin >> sceneLights[x].location.y;
		cin >> sceneLights[x].location.z;
		cout << "Enter Light Max Distance: ";
		cin >> sceneLights[x].maxDistance;
		cout << "Enter Light Intensity: ";
		cin >> sceneLights[x].intensity;
	}

	raytrace(camera, scenePolygons, sceneLights);
	return 0;
}

#define _CRT_SECURE_NO_WARNINGS 1
#include <vector>
#include <cmath>
#include <random>
#include <limits>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <omp.h>

#ifndef M_PI
#define M_PI 3.14159265358979323856
#endif

static std::default_random_engine engine[32];
static std::uniform_real_distribution<double> uniform(0, 1);

double sqr(double x) { return x * x; };

class Vector {
public:
	explicit Vector(double x = 0, double y = 0, double z = 0) {
		data[0] = x;
		data[1] = y;
		data[2] = z;
	}
	double norm2() const {
		return data[0] * data[0] + data[1] * data[1] + data[2] * data[2];
	}
	double norm() const {
		return sqrt(norm2());
	}
	void normalize() {
		double n = norm();
		data[0] /= n;
		data[1] /= n;
		data[2] /= n;
	}
	double operator[](int i) const { return data[i]; };
	double& operator[](int i) { return data[i]; };
	double data[3];
};

Vector operator+(const Vector& a, const Vector& b) {
	return Vector(a[0] + b[0], a[1] + b[1], a[2] + b[2]);
}
Vector operator-(const Vector& a, const Vector& b) {
	return Vector(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
}
Vector operator*(const double a, const Vector& b) {
	return Vector(a*b[0], a*b[1], a*b[2]);
}
Vector operator*(const Vector& a, const double b) {
	return Vector(a[0]*b, a[1]*b, a[2]*b);
}
Vector operator/(const Vector& a, const double b) {
	return Vector(a[0] / b, a[1] / b, a[2] / b);
}
double dot(const Vector& a, const Vector& b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
Vector cross(const Vector& a, const Vector& b) {
	return Vector(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
}

Vector random_cos(const Vector& N) {
	const double r1 = uniform(engine[omp_get_thread_num()]);
	const double r2 = uniform(engine[omp_get_thread_num()]);
	const double x = cos(2 * M_PI * r1) * sqrt(1 - r2);
	const double y = sin(2 * M_PI * r1) * sqrt(1 - r2);
	const double z = sqrt(r2);

	Vector T1;
	if (fabs(N[0]) <= fabs(N[1]) && fabs(N[0]) <= fabs(N[2])) {
		T1 = Vector(0, -N[2], N[1]);
	} else if (fabs(N[1]) <= fabs(N[0]) && fabs(N[1]) <= fabs(N[2])) {
		T1 = Vector(-N[2], 0, N[0]);
	} else {
		T1 = Vector(-N[1], N[0], 0);
	}
	T1.normalize();

	return x * T1 + y * cross(N, T1) + z * N;
}

void boxMuller(const double stdev, double& x, double& y) {
	const double r1 = uniform(engine[omp_get_thread_num()]);
	const double r2 = uniform(engine[omp_get_thread_num()]);
	x = sqrt(-2 * log(r1)) * cos(2 * M_PI * r2) * stdev;
	y = sqrt(-2 * log(r1)) * sin(2 * M_PI * r2) * stdev;
}

class Ray {
public:
	Ray(const Vector& origin, const Vector& unit_direction) : O(origin), u(unit_direction) {};
	Vector O, u;
};

class Object {
public:
	Object(const Vector& albedo, bool mirror = false, bool transparent = false) : albedo(albedo), mirror(mirror), transparent(transparent) {};

	virtual bool intersect(const Ray& ray, Vector& P, double& t, Vector& N) const = 0;

	Vector albedo;
	bool mirror, transparent;
};

class Sphere : public Object {
public:
	Sphere(const Vector& center, double radius, const Vector& albedo, bool mirror = false, bool transparent = false) : ::Object(albedo, mirror, transparent), C(center), R(radius) {};

	// returns true iif there is an intersection between the ray and the sphere
	// if there is an intersection, also computes the point of intersection P, 
	// t>=0 the distance between the ray origin and P (i.e., the parameter along the ray)
	// and the unit normal N
	bool intersect(const Ray& ray, Vector& P, double &t, Vector& N) const {
		 // TODO (lab 1) : compute the intersection (just true/false at the begining of lab 1, then P, t and N as well)
		const Vector c = ray.O - C;
		const double delta = pow(dot(ray.u, c), 2) - c.norm2() + pow(R, 2);
		if (delta < 0) return false;

		 for (int i = -1; i <= 1; i+=2) {
		 	const double tmp = dot(ray.u, -1*c) + i*sqrt(delta);
		 	if (tmp >= 0) {
		 		t = tmp;
		 		P = ray.O + t * ray.u;
		 		N = P - C;
		 		N.normalize();
		 		return true;
		 	}
		 }

		return false;
	}

	double R;
	Vector C;
};


// I will provide you with an obj mesh loader (labs 3 and 4)
class TriangleMesh : public Object {
public:
	TriangleMesh(const Vector& albedo, bool mirror = false, bool transparent = false) : ::Object(albedo, mirror, transparent) {};

	bool intersect(const Ray& ray, Vector& P, double& t, Vector& N) const {
		// TODO (labs 3 and 4)
		return false;
	}
};


class Scene {
public:
	Scene() {};
	void addObject(const Object* obj) {
		objects.push_back(obj);
	}

	// returns true iif there is an intersection between the ray and any object in the scene
    // if there is an intersection, also computes the point of the *nearest* intersection P, 
    // t>=0 the distance between the ray origin and P (i.e., the parameter along the ray)
    // and the unit normal N. 
	// Also returns the index of the object within the std::vector objects in object_id
	bool intersect(const Ray& ray, Vector& P, double& t, Vector& N, int &object_id) const  {

		// TODO (lab 1): iterate through the objects and check the intersections with all of them, 
		// and keep the closest intersection, i.e., the one if smallest positive value of t

		double min_val = std::numeric_limits<double>::max();
		for (int i = 0; i < objects.size(); i++) {
			Vector P_p, N_p;
			double t_p;
			if (objects[i]->intersect(ray, P_p, t_p, N_p) && t_p < min_val) {
				min_val = t_p;
				P = P_p;
				t = t_p;
				N = N_p;
				object_id = i;
			}
		}

		return min_val != std::numeric_limits<double>::max();
	}


	// return the radiance (color) along ray
	Vector getColor(const Ray& ray, int recursion_depth) {

		if (recursion_depth >= max_light_bounce) return Vector(0, 0, 0);

		// TODO (lab 1) : if intersect with ray, use the returned information to compute the color ; otherwise black 
		// in lab 1, the color only includes direct lighting with shadows

		Vector P, N;
		double t;
		int object_id;
		if (intersect(ray, P, t, N, object_id)) {

		if (objects[object_id]->mirror) {
			const Vector vec = ray.u - 2 * dot(ray.u, N) * N;
			const Ray reflection(P + 1e-5 * N, vec);
			return getColor(reflection, recursion_depth+1);
			// return getColor in the reflected direction, with recursion_depth+1 (recursively)
		} // else

		if (objects[object_id]->transparent) { // optional

			// return getColor in the refraction direction, with recursion_depth+1 (recursively)
		} // else

		// test if there is a shadow by sending a new ray
		// if there is no shadow, compute the formula with dot products etc.

		const Vector l_p = light_position - P;
		const Vector l_dir = l_p / l_p.norm();

		Vector direct(0, 0, 0);
		const Ray shadow(P + 1e-5 * N, l_dir);
		Vector P_p;
		double t_p;
		Vector N_p;
		int object_id_p;
		if (!intersect(shadow, P_p, t_p, N_p, object_id_p) || (P_p - P).norm2() > l_p.norm2()) { // shadow
			const double attenuation = light_intensity / (4 * M_PI * l_p.norm2());
			const Vector material = objects[object_id]->albedo / M_PI;
			const double solid_angle = std::max(0., dot(N, l_dir));
			direct = attenuation * material * solid_angle;
		}

		// TODO (lab 2) : add indirect lighting component with a recursive call
		const Vector wi = random_cos(N);
		const Ray indirect_ray(P + 1e-5 * N, wi);
		Vector Li = getColor(indirect_ray, recursion_depth + 1);
		Vector rho = objects[object_id]->albedo;
		const Vector indirect(rho[0] * Li[0], rho[1] * Li[1], rho[2] * Li[2]);

		return direct + indirect;
	}

		

		return Vector(0, 0, 0);
	}

	std::vector<const Object*> objects;

	Vector camera_center, light_position;
	double fov, gamma, light_intensity;
	int max_light_bounce;
};


int main() {
	int W = 512;
	int H = 512;

	for (int i = 0; i<32; i++) {
		engine[i].seed(i);
	}

	Sphere center_sphere(Vector(0, 0, 0), 10., Vector(0.8, 0.8, 0.8));
	Sphere wall_left(Vector(-1000, 0, 0), 940, Vector(0.5, 0.8, 0.1));
	Sphere wall_right(Vector(1000, 0, 0), 940, Vector(0.9, 0.2, 0.3));
	Sphere wall_front(Vector(0, 0, -1000), 940, Vector(0.1, 0.6, 0.7));
	Sphere wall_behind(Vector(0, 0, 1000), 940, Vector(0.8, 0.2, 0.9));
	Sphere ceiling(Vector(0, 1000, 0), 940, Vector(0.3, 0.5, 0.3));
	Sphere floor(Vector(0, -1000, 0), 990, Vector(0.6, 0.5, 0.7));

	Scene scene;
	scene.camera_center = Vector(0, 0, 55);
	scene.light_position = Vector(-10,20,40);
	scene.light_intensity = 3E7;
	scene.fov = 60 * M_PI / 180.;
	scene.gamma = 2.2;    // TODO (lab 1) : play with gamma ; typically, gamma = 2.2
	scene.max_light_bounce = 5;

	scene.addObject(&center_sphere);

	scene.addObject(&wall_left);
	scene.addObject(&wall_right);
	scene.addObject(&wall_front);
	scene.addObject(&wall_behind);
	scene.addObject(&ceiling);
	scene.addObject(&floor);

	std::vector<unsigned char> image(W * H * 3, 0);

	int nb_samples = 32;
	double focus_distance = 55.;
	double aperture_radius = 0.5;

#pragma omp parallel for schedule(dynamic, 1)
	for (int i = 0; i < H; i++) {
		for (int j = 0; j < W; j++) {
			Vector color(0, 0, 0);

			// TODO (lab 2) : add Monte Carlo / averaging of random ray contributions here
			for (int s = 0; s < nb_samples; s++) {
				// TODO (lab 2) : add antialiasing by altering the ray_direction here
				double dx, dy;
				boxMuller(0.5, dx, dy);

				// TODO (lab 1) : correct ray_direction so that it goes through each pixel (j, i)
				const double x = j - W/2 + 0.5 + dx;
				const double y = H/2 - i - 0.5 + dy;
				const double z = -W / (2 * tan(scene.fov / 2));
				Vector ray_direction(x, y, z);
				ray_direction.normalize();

				// TODO (lab 2) : add depth of field effect by altering the ray origin (and direction) here
				double r1 = uniform(engine[omp_get_thread_num()]);
				double r2 = uniform(engine[omp_get_thread_num()]);
				double r = aperture_radius * sqrt(r1);
				double theta = 2 * M_PI * r2;
				Vector focus_point = scene.camera_center + (focus_distance / std::abs(ray_direction[2])) * ray_direction;
				Vector origin = scene.camera_center + Vector(r * cos(theta), r * sin(theta), 0);
				ray_direction = focus_point - origin;
				ray_direction.normalize();

				Ray ray(origin, ray_direction);
				color = color + scene.getColor(ray, 0);
			}
			color = color / nb_samples;

			image[(i * W + j) * 3 + 0] = std::min(255., std::max(0., 255. * std::pow(color[0] / 255., 1. / scene.gamma)));
			image[(i * W + j) * 3 + 1] = std::min(255., std::max(0., 255. * std::pow(color[1] / 255., 1. / scene.gamma)));
			image[(i * W + j) * 3 + 2] = std::min(255., std::max(0., 255. * std::pow(color[2] / 255., 1. / scene.gamma)));
		}
	}
	stbi_write_png("image.png", W, H, 3, &image[0], 0);

	return 0;
}
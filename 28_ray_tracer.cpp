#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <algorithm>
#include <random>
#include <fstream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <iomanip>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Forward declarations
class Vector3;
class Ray;
class Material;
class Object;
class Sphere;
class Plane;
class Light;
class Camera;
class Scene;
class RayTracer;

// 3D Vector class with comprehensive operations
class Vector3 {
public:
    double x, y, z;
    
    Vector3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    
    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(double s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(double s) const { return Vector3(x / s, y / s, z / s); }
    Vector3 operator-() const { return Vector3(-x, -y, -z); }
    
    Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    Vector3& operator*=(double s) { x *= s; y *= s; z *= s; return *this; }
    
    double dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
    
    Vector3 cross(const Vector3& v) const {
        return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
    
    double length() const { return sqrt(x * x + y * y + z * z); }
    double lengthSquared() const { return x * x + y * y + z * z; }
    
    Vector3 normalize() const {
        double len = length();
        return (len > 0) ? Vector3(x / len, y / len, z / len) : Vector3(0, 0, 0);
    }
    
    Vector3 reflect(const Vector3& normal) const {
        return *this - normal * 2 * dot(normal);
    }
    
    Vector3 refract(const Vector3& normal, double eta) const {
        double cosI = -dot(normal);
        double sinT2 = eta * eta * (1.0 - cosI * cosI);
        if (sinT2 > 1.0) return Vector3(0, 0, 0); // Total internal reflection
        
        double cosT = sqrt(1.0 - sinT2);
        return *this * eta + normal * (eta * cosI - cosT);
    }
    
    // Component-wise operations
    Vector3 multiply(const Vector3& v) const { return Vector3(x * v.x, y * v.y, z * v.z); }
    
    // Utility functions
    static Vector3 random(double min = 0.0, double max = 1.0) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(min, max);
        return Vector3(dis(gen), dis(gen), dis(gen));
    }
    
    static Vector3 randomInUnitSphere() {
        Vector3 p;
        do {
            p = random(-1, 1);
        } while (p.lengthSquared() >= 1.0);
        return p;
    }
    
    static Vector3 randomInHemisphere(const Vector3& normal) {
        Vector3 inUnitSphere = randomInUnitSphere();
        if (inUnitSphere.dot(normal) > 0.0) // In same hemisphere as normal
            return inUnitSphere;
        else
            return -inUnitSphere;
    }
    
    std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

// Color class (inherits from Vector3)
using Color = Vector3;

// Ray class
class Ray {
public:
    Vector3 origin;
    Vector3 direction;
    
    Ray() = default;
    Ray(const Vector3& o, const Vector3& d) : origin(o), direction(d.normalize()) {}
    
    Vector3 at(double t) const { return origin + direction * t; }
    
    std::string toString() const {
        return "Ray(origin: " + origin.toString() + ", direction: " + direction.toString() + ")";
    }
};

// Hit record for ray-object intersections
struct HitRecord {
    Vector3 point;
    Vector3 normal;
    double t;
    bool frontFace;
    std::shared_ptr<Material> material;
    
    void setFaceNormal(const Ray& ray, const Vector3& outwardNormal) {
        frontFace = ray.direction.dot(outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

// Material base class
class Material {
public:
    Color albedo;
    double roughness;
    double metallic;
    double transparency;
    double refractiveIndex;
    Color emissive;
    
    Material(const Color& a = Color(0.5, 0.5, 0.5), double r = 0.5, double m = 0.0, 
             double t = 0.0, double ri = 1.0, const Color& e = Color(0, 0, 0))
        : albedo(a), roughness(r), metallic(m), transparency(t), refractiveIndex(ri), emissive(e) {}
    
    virtual ~Material() = default;
    
    virtual bool scatter(const Ray& rayIn, const HitRecord& rec, Color& attenuation, Ray& scattered) const {
        // Default Lambertian scattering
        Vector3 scatterDirection = rec.normal + Vector3::randomInUnitSphere().normalize();
        
        // Catch degenerate scatter direction
        if (scatterDirection.lengthSquared() < 1e-8)
            scatterDirection = rec.normal;
            
        scattered = Ray(rec.point, scatterDirection);
        attenuation = albedo;
        return true;
    }
    
    virtual Color emit() const { return emissive; }
};

// Lambertian (diffuse) material
class Lambertian : public Material {
public:
    Lambertian(const Color& a) : Material(a, 1.0, 0.0) {}
    
    bool scatter(const Ray& rayIn, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        Vector3 scatterDirection = rec.normal + Vector3::randomInUnitSphere().normalize();
        
        if (scatterDirection.lengthSquared() < 1e-8)
            scatterDirection = rec.normal;
            
        scattered = Ray(rec.point, scatterDirection);
        attenuation = albedo;
        return true;
    }
};

// Metal material
class Metal : public Material {
public:
    Metal(const Color& a, double fuzz = 0.0) : Material(a, fuzz, 1.0) {}
    
    bool scatter(const Ray& rayIn, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        Vector3 reflected = rayIn.direction.reflect(rec.normal);
        reflected = reflected.normalize() + Vector3::randomInUnitSphere() * roughness;
        scattered = Ray(rec.point, reflected);
        attenuation = albedo;
        return scattered.direction.dot(rec.normal) > 0;
    }
};

// Dielectric (glass) material
class Dielectric : public Material {
private:
    static double reflectance(double cosine, double refIdx) {
        // Schlick's approximation for reflectance
        double r0 = (1 - refIdx) / (1 + refIdx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
    
public:
    Dielectric(double ri) : Material(Color(1.0, 1.0, 1.0), 0.0, 0.0, 0.9, ri) {}
    
    bool scatter(const Ray& rayIn, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        attenuation = Color(1.0, 1.0, 1.0);
        double refractionRatio = rec.frontFace ? (1.0 / refractiveIndex) : refractiveIndex;
        
        Vector3 unitDirection = rayIn.direction.normalize();
        double cosTheta = std::min(-unitDirection.dot(rec.normal), 1.0);
        double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
        
        bool cannotRefract = refractionRatio * sinTheta > 1.0;
        Vector3 direction;
        
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        if (cannotRefract || reflectance(cosTheta, refractionRatio) > dis(gen)) {
            direction = unitDirection.reflect(rec.normal);
        } else {
            direction = unitDirection.refract(rec.normal, refractionRatio);
        }
        
        scattered = Ray(rec.point, direction);
        return true;
    }
};

// Emissive material
class Emissive : public Material {
public:
    Emissive(const Color& e, double intensity = 1.0) : Material(Color(0, 0, 0), 0, 0, 0, 1.0, e * intensity) {}
    
    bool scatter(const Ray& rayIn, const HitRecord& rec, Color& attenuation, Ray& scattered) const override {
        return false; // Emissive materials don't scatter light
    }
    
    Color emit() const override { return emissive; }
};

// Object base class
class Object {
public:
    std::shared_ptr<Material> material;
    
    Object(std::shared_ptr<Material> m) : material(m) {}
    virtual ~Object() = default;
    
    virtual bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const = 0;
    virtual Vector3 getCenter() const = 0;
    virtual std::string toString() const = 0;
};

// Sphere object
class Sphere : public Object {
public:
    Vector3 center;
    double radius;
    
    Sphere(const Vector3& c, double r, std::shared_ptr<Material> m)
        : Object(m), center(c), radius(r) {}
    
    bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const override {
        Vector3 oc = ray.origin - center;
        double a = ray.direction.lengthSquared();
        double halfB = oc.dot(ray.direction);
        double c = oc.lengthSquared() - radius * radius;
        
        double discriminant = halfB * halfB - a * c;
        if (discriminant < 0) return false;
        
        double sqrtd = sqrt(discriminant);
        double root = (-halfB - sqrtd) / a;
        if (root < tMin || tMax < root) {
            root = (-halfB + sqrtd) / a;
            if (root < tMin || tMax < root)
                return false;
        }
        
        rec.t = root;
        rec.point = ray.at(rec.t);
        Vector3 outwardNormal = (rec.point - center) / radius;
        rec.setFaceNormal(ray, outwardNormal);
        rec.material = material;
        
        return true;
    }
    
    Vector3 getCenter() const override { return center; }
    
    std::string toString() const override {
        return "Sphere(center: " + center.toString() + ", radius: " + std::to_string(radius) + ")";
    }
};

// Plane object
class Plane : public Object {
public:
    Vector3 point;
    Vector3 normal;
    
    Plane(const Vector3& p, const Vector3& n, std::shared_ptr<Material> m)
        : Object(m), point(p), normal(n.normalize()) {}
    
    bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const override {
        double denom = normal.dot(ray.direction);
        if (abs(denom) < 1e-8) return false; // Ray is parallel to plane
        
        double t = (point - ray.origin).dot(normal) / denom;
        if (t < tMin || t > tMax) return false;
        
        rec.t = t;
        rec.point = ray.at(t);
        rec.setFaceNormal(ray, normal);
        rec.material = material;
        
        return true;
    }
    
    Vector3 getCenter() const override { return point; }
    
    std::string toString() const override {
        return "Plane(point: " + point.toString() + ", normal: " + normal.toString() + ")";
    }
};

// Triangle object
class Triangle : public Object {
public:
    Vector3 v0, v1, v2;
    Vector3 normal;
    
    Triangle(const Vector3& a, const Vector3& b, const Vector3& c, std::shared_ptr<Material> m)
        : Object(m), v0(a), v1(b), v2(c) {
        normal = (v1 - v0).cross(v2 - v0).normalize();
    }
    
    bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const override {
        // Möller-Trumbore intersection algorithm
        Vector3 edge1 = v1 - v0;
        Vector3 edge2 = v2 - v0;
        Vector3 h = ray.direction.cross(edge2);
        double a = edge1.dot(h);
        
        if (a > -1e-8 && a < 1e-8) return false; // Ray is parallel to triangle
        
        double f = 1.0 / a;
        Vector3 s = ray.origin - v0;
        double u = f * s.dot(h);
        
        if (u < 0.0 || u > 1.0) return false;
        
        Vector3 q = s.cross(edge1);
        double v = f * ray.direction.dot(q);
        
        if (v < 0.0 || u + v > 1.0) return false;
        
        double t = f * edge2.dot(q);
        
        if (t < tMin || t > tMax) return false;
        
        rec.t = t;
        rec.point = ray.at(t);
        rec.setFaceNormal(ray, normal);
        rec.material = material;
        
        return true;
    }
    
    Vector3 getCenter() const override { return (v0 + v1 + v2) / 3.0; }
    
    std::string toString() const override {
        return "Triangle(" + v0.toString() + ", " + v1.toString() + ", " + v2.toString() + ")";
    }
};

// Light source
class Light {
public:
    Vector3 position;
    Color color;
    double intensity;
    
    Light(const Vector3& pos, const Color& col, double intens = 1.0)
        : position(pos), color(col), intensity(intens) {}
    
    std::string toString() const {
        return "Light(pos: " + position.toString() + ", color: " + color.toString() + 
               ", intensity: " + std::to_string(intensity) + ")";
    }
};

// Camera class
class Camera {
public:
    Vector3 position;
    Vector3 target;
    Vector3 up;
    double fov;
    double aspectRatio;
    double aperture;
    double focusDistance;
    
    Vector3 u, v, w;
    double lensRadius;
    Vector3 horizontal;
    Vector3 vertical;
    Vector3 lowerLeftCorner;
    
    Camera(const Vector3& pos, const Vector3& tar, const Vector3& upVec, 
           double verticalFov, double aspect, double aperture_ = 0.0, double focusDist = 1.0)
        : position(pos), target(tar), up(upVec), fov(verticalFov), 
          aspectRatio(aspect), aperture(aperture_), focusDistance(focusDist) {
        
        lensRadius = aperture / 2;
        
        double theta = fov * M_PI / 180;
        double halfHeight = tan(theta / 2);
        double halfWidth = aspectRatio * halfHeight;
        
        w = (position - target).normalize();
        u = up.cross(w).normalize();
        v = w.cross(u);
        
        horizontal = u * halfWidth * focusDistance * 2;
        vertical = v * halfHeight * focusDistance * 2;
        lowerLeftCorner = position - horizontal/2 - vertical/2 - w * focusDistance;
    }
    
    Ray getRay(double s, double t) const {
        Vector3 rd = Vector3::randomInUnitSphere() * lensRadius;
        Vector3 offset = u * rd.x + v * rd.y;
        
        Vector3 rayOrigin = position + offset;
        Vector3 rayDirection = lowerLeftCorner + horizontal * s + vertical * t - position - offset;
        
        return Ray(rayOrigin, rayDirection);
    }
    
    std::string toString() const {
        return "Camera(pos: " + position.toString() + ", target: " + target.toString() + 
               ", fov: " + std::to_string(fov) + ")";
    }
};

// Scene class
class Scene {
public:
    std::vector<std::shared_ptr<Object>> objects;
    std::vector<Light> lights;
    Color backgroundColor;
    Color ambientLight;
    
    Scene(const Color& bg = Color(0.5, 0.7, 1.0), const Color& ambient = Color(0.1, 0.1, 0.1))
        : backgroundColor(bg), ambientLight(ambient) {}
    
    void addObject(std::shared_ptr<Object> object) {
        objects.push_back(object);
    }
    
    void addLight(const Light& light) {
        lights.push_back(light);
    }
    
    bool hit(const Ray& ray, double tMin, double tMax, HitRecord& rec) const {
        HitRecord tempRec;
        bool hitAnything = false;
        double closestSoFar = tMax;
        
        for (const auto& object : objects) {
            if (object->hit(ray, tMin, closestSoFar, tempRec)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }
        
        return hitAnything;
    }
    
    Color getBackgroundColor(const Ray& ray) const {
        // Gradient background
        Vector3 unitDirection = ray.direction.normalize();
        double t = 0.5 * (unitDirection.y + 1.0);
        return Color(1.0, 1.0, 1.0) * (1.0 - t) + backgroundColor * t;
    }
    
    std::string toString() const {
        return "Scene(objects: " + std::to_string(objects.size()) + 
               ", lights: " + std::to_string(lights.size()) + ")";
    }
};

// Ray tracer class
class RayTracer {
private:
    int imageWidth;
    int imageHeight;
    int samplesPerPixel;
    int maxDepth;
    std::vector<std::vector<Color>> image;
    std::atomic<int> pixelsCompleted;
    std::mutex progressMutex;
    
public:
    RayTracer(int width, int height, int samples = 100, int depth = 50)
        : imageWidth(width), imageHeight(height), samplesPerPixel(samples), maxDepth(depth),
          pixelsCompleted(0) {
        image.resize(imageHeight, std::vector<Color>(imageWidth));
    }
    
    Color rayColor(const Ray& ray, const Scene& scene, int depth) const {
        if (depth <= 0) return Color(0, 0, 0);
        
        HitRecord rec;
        if (scene.hit(ray, 0.001, std::numeric_limits<double>::infinity(), rec)) {
            // Add emissive contribution
            Color emitted = rec.material->emit();
            
            Color attenuation;
            Ray scattered;
            if (rec.material->scatter(ray, rec, attenuation, scattered)) {
                return emitted + attenuation.multiply(rayColor(scattered, scene, depth - 1));
            } else {
                return emitted;
            }
        }
        
        return scene.getBackgroundColor(ray);
    }
    
    void renderPixel(int i, int j, const Camera& camera, const Scene& scene) {
        Color pixelColor(0, 0, 0);
        
        for (int s = 0; s < samplesPerPixel; s++) {
            static std::random_device rd;
            static thread_local std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);
            
            double u = (i + dis(gen)) / (imageWidth - 1);
            double v = (j + dis(gen)) / (imageHeight - 1);
            
            Ray ray = camera.getRay(u, v);
            pixelColor += rayColor(ray, scene, maxDepth);
        }
        
        // Average the samples
        pixelColor = pixelColor / samplesPerPixel;
        
        // Gamma correction (gamma = 2.0)
        pixelColor = Color(sqrt(pixelColor.x), sqrt(pixelColor.y), sqrt(pixelColor.z));
        
        image[imageHeight - 1 - j][i] = pixelColor;
        
        // Update progress
        int completed = ++pixelsCompleted;
        if (completed % (imageWidth * imageHeight / 100) == 0) {
            std::lock_guard<std::mutex> lock(progressMutex);
            double progress = 100.0 * completed / (imageWidth * imageHeight);
            std::cout << "Progress: " << std::fixed << std::setprecision(1) 
                      << progress << "%\r" << std::flush;
        }
    }
    
    void render(const Camera& camera, const Scene& scene, int numThreads = 4) {
        std::cout << "Starting ray tracing..." << std::endl;
        std::cout << "Image size: " << imageWidth << "x" << imageHeight << std::endl;
        std::cout << "Samples per pixel: " << samplesPerPixel << std::endl;
        std::cout << "Max depth: " << maxDepth << std::endl;
        std::cout << "Threads: " << numThreads << std::endl;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        pixelsCompleted = 0;
        
        std::vector<std::thread> threads;
        std::atomic<int> nextPixel(0);
        int totalPixels = imageWidth * imageHeight;
        
        for (int t = 0; t < numThreads; t++) {
            threads.emplace_back([&]() {
                int pixel;
                while ((pixel = nextPixel++) < totalPixels) {
                    int i = pixel % imageWidth;
                    int j = pixel / imageWidth;
                    renderPixel(i, j, camera, scene);
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime);
        
        std::cout << "\nRendering completed in " << duration.count() << " seconds" << std::endl;
    }
    
    void saveImage(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return;
        }
        
        // PPM format
        file << "P3\n" << imageWidth << " " << imageHeight << "\n255\n";
        
        for (const auto& row : image) {
            for (const auto& pixel : row) {
                int r = static_cast<int>(256 * std::clamp(pixel.x, 0.0, 0.999));
                int g = static_cast<int>(256 * std::clamp(pixel.y, 0.0, 0.999));
                int b = static_cast<int>(256 * std::clamp(pixel.z, 0.0, 0.999));
                
                file << r << " " << g << " " << b << "\n";
            }
        }
        
        file.close();
        std::cout << "Image saved to " << filename << std::endl;
    }
    
    void printImageStats() const {
        double avgBrightness = 0.0;
        Color minColor(1.0, 1.0, 1.0);
        Color maxColor(0.0, 0.0, 0.0);
        
        for (const auto& row : image) {
            for (const auto& pixel : row) {
                double brightness = (pixel.x + pixel.y + pixel.z) / 3.0;
                avgBrightness += brightness;
                
                minColor.x = std::min(minColor.x, pixel.x);
                minColor.y = std::min(minColor.y, pixel.y);
                minColor.z = std::min(minColor.z, pixel.z);
                
                maxColor.x = std::max(maxColor.x, pixel.x);
                maxColor.y = std::max(maxColor.y, pixel.y);
                maxColor.z = std::max(maxColor.z, pixel.z);
            }
        }
        
        avgBrightness /= (imageWidth * imageHeight);
        
        std::cout << "\n=== IMAGE STATISTICS ===" << std::endl;
        std::cout << "Average brightness: " << std::fixed << std::setprecision(3) << avgBrightness << std::endl;
        std::cout << "Min color: " << minColor.toString() << std::endl;
        std::cout << "Max color: " << maxColor.toString() << std::endl;
    }
};

// Scene builder utility
class SceneBuilder {
public:
    static std::unique_ptr<Scene> createCornellBox() {
        auto scene = std::make_unique<Scene>(Color(0, 0, 0));
        
        // Materials
        auto red = std::make_shared<Lambertian>(Color(0.65, 0.05, 0.05));
        auto white = std::make_shared<Lambertian>(Color(0.73, 0.73, 0.73));
        auto green = std::make_shared<Lambertian>(Color(0.12, 0.45, 0.15));
        auto light = std::make_shared<Emissive>(Color(1, 1, 1), 15);
        
        // Walls
        scene->addObject(std::make_shared<Plane>(Vector3(0, 0, 0), Vector3(0, 0, 1), white));     // Back
        scene->addObject(std::make_shared<Plane>(Vector3(-1, 0, 0), Vector3(1, 0, 0), green));    // Left
        scene->addObject(std::make_shared<Plane>(Vector3(1, 0, 0), Vector3(-1, 0, 0), red));      // Right
        scene->addObject(std::make_shared<Plane>(Vector3(0, -1, 0), Vector3(0, 1, 0), white));    // Floor
        scene->addObject(std::make_shared<Plane>(Vector3(0, 1, 0), Vector3(0, -1, 0), white));    // Ceiling
        
        // Light
        scene->addObject(std::make_shared<Sphere>(Vector3(0, 0.8, -0.3), 0.15, light));
        
        // Objects
        auto glass = std::make_shared<Dielectric>(1.5);
        auto metal = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
        
        scene->addObject(std::make_shared<Sphere>(Vector3(-0.3, -0.7, -0.3), 0.3, glass));
        scene->addObject(std::make_shared<Sphere>(Vector3(0.3, -0.6, -0.6), 0.4, metal));
        
        return scene;
    }
    
    static std::unique_ptr<Scene> createRandomScene() {
        auto scene = std::make_unique<Scene>();
        
        // Ground
        auto ground = std::make_shared<Lambertian>(Color(0.5, 0.5, 0.5));
        scene->addObject(std::make_shared<Sphere>(Vector3(0, -1000, 0), 1000, ground));
        
        // Random spheres
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        for (int a = -11; a < 11; a++) {
            for (int b = -11; b < 11; b++) {
                double chooseMat = dis(gen);
                Vector3 center(a + 0.9 * dis(gen), 0.2, b + 0.9 * dis(gen));
                
                if ((center - Vector3(4, 0.2, 0)).length() > 0.9) {
                    std::shared_ptr<Material> sphereMaterial;
                    
                    if (chooseMat < 0.8) {
                        // Diffuse
                        Color albedo = Color::random() * Color::random();
                        sphereMaterial = std::make_shared<Lambertian>(albedo);
                    } else if (chooseMat < 0.95) {
                        // Metal
                        Color albedo = Color::random(0.5, 1);
                        double fuzz = dis(gen) * 0.5;
                        sphereMaterial = std::make_shared<Metal>(albedo, fuzz);
                    } else {
                        // Glass
                        sphereMaterial = std::make_shared<Dielectric>(1.5);
                    }
                    
                    scene->addObject(std::make_shared<Sphere>(center, 0.2, sphereMaterial));
                }
            }
        }
        
        // Large spheres
        auto material1 = std::make_shared<Dielectric>(1.5);
        scene->addObject(std::make_shared<Sphere>(Vector3(0, 1, 0), 1.0, material1));
        
        auto material2 = std::make_shared<Lambertian>(Color(0.4, 0.2, 0.1));
        scene->addObject(std::make_shared<Sphere>(Vector3(-4, 1, 0), 1.0, material2));
        
        auto material3 = std::make_shared<Metal>(Color(0.7, 0.6, 0.5), 0.0);
        scene->addObject(std::make_shared<Sphere>(Vector3(4, 1, 0), 1.0, material3));
        
        return scene;
    }
    
    static std::unique_ptr<Scene> createReflectionScene() {
        auto scene = std::make_unique<Scene>(Color(0.1, 0.1, 0.2));
        
        // Materials
        auto ground = std::make_shared<Lambertian>(Color(0.8, 0.8, 0.0));
        auto center = std::make_shared<Lambertian>(Color(0.1, 0.2, 0.5));
        auto left = std::make_shared<Dielectric>(1.5);
        auto leftInner = std::make_shared<Dielectric>(1.0/1.5);
        auto right = std::make_shared<Metal>(Color(0.8, 0.6, 0.2), 0.0);
        auto light = std::make_shared<Emissive>(Color(1, 1, 1), 5);
        
        // Objects
        scene->addObject(std::make_shared<Sphere>(Vector3(0, -100.5, -1), 100, ground));
        scene->addObject(std::make_shared<Sphere>(Vector3(0, 0, -1), 0.5, center));
        scene->addObject(std::make_shared<Sphere>(Vector3(-1, 0, -1), 0.5, left));
        scene->addObject(std::make_shared<Sphere>(Vector3(-1, 0, -1), -0.4, leftInner));
        scene->addObject(std::make_shared<Sphere>(Vector3(1, 0, -1), 0.5, right));
        
        // Light source
        scene->addObject(std::make_shared<Sphere>(Vector3(0, 5, -1), 1, light));
        
        return scene;
    }
};

// Demo function
void runRayTracerDemo() {
    std::cout << "=== ADVANCED RAY TRACER DEMO ===" << std::endl;
    
    // Create different scenes
    std::vector<std::pair<std::string, std::function<std::unique_ptr<Scene>()>>> scenes = {
        {"Cornell Box", []() { return SceneBuilder::createCornellBox(); }},
        {"Reflection Scene", []() { return SceneBuilder::createReflectionScene(); }},
        {"Random Scene (Small)", []() { return SceneBuilder::createRandomScene(); }}
    };
    
    for (size_t sceneIdx = 0; sceneIdx < scenes.size(); sceneIdx++) {
        std::cout << "\n" << std::string(50, '=') << std::endl;
        std::cout << "Rendering Scene " << (sceneIdx + 1) << ": " << scenes[sceneIdx].first << std::endl;
        std::cout << std::string(50, '=') << std::endl;
        
        // Create scene
        auto scene = scenes[sceneIdx].second();
        
        // Set up camera based on scene
        Camera camera(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(0, 1, 0), 45, 16.0/9.0);
        
        if (sceneIdx == 0) { // Cornell Box
            camera = Camera(Vector3(0, 0, 1), Vector3(0, 0, -1), Vector3(0, 1, 0), 45, 1.0);
        } else if (sceneIdx == 1) { // Reflection Scene
            camera = Camera(Vector3(-2, 2, 1), Vector3(0, 0, -1), Vector3(0, 1, 0), 20, 16.0/9.0);
        } else { // Random Scene
            camera = Camera(Vector3(13, 2, 3), Vector3(0, 0, 0), Vector3(0, 1, 0), 20, 16.0/9.0, 0.1, 10.0);
        }
        
        std::cout << "Camera: " << camera.toString() << std::endl;
        std::cout << "Scene: " << scene->toString() << std::endl;
        
        // Create ray tracer with appropriate settings
        int width = (sceneIdx == 2) ? 200 : 300;  // Smaller for random scene
        int height = (sceneIdx == 0) ? width : static_cast<int>(width / camera.aspectRatio);
        int samples = (sceneIdx == 2) ? 50 : 100; // Fewer samples for complex scene
        
        RayTracer rayTracer(width, height, samples, 50);
        
        // Render
        rayTracer.render(camera, *scene, std::thread::hardware_concurrency());
        
        // Save image
        std::string filename = "scene_" + std::to_string(sceneIdx + 1) + "_" + 
                              std::to_string(width) + "x" + std::to_string(height) + ".ppm";
        rayTracer.saveImage(filename);
        
        // Print statistics
        rayTracer.printImageStats();
        
        std::cout << "\nScene " << (sceneIdx + 1) << " completed!" << std::endl;
    }
    
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "Ray tracing demo completed!" << std::endl;
    std::cout << "Generated images:" << std::endl;
    std::cout << "- scene_1_300x300.ppm (Cornell Box)" << std::endl;
    std::cout << "- scene_2_300x169.ppm (Reflection Scene)" << std::endl;
    std::cout << "- scene_3_200x112.ppm (Random Scene)" << std::endl;
    std::cout << "\nNote: PPM files can be viewed with image viewers that support the format" << std::endl;
    std::cout << "or converted to other formats using tools like ImageMagick." << std::endl;
}

int main() {
    try {
        runRayTracerDemo();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

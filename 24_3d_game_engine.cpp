#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <map>
#include <functional>
#include <random>
#include <fstream>

// Simple 3D vector class
class Vector3 {
public:
    float x, y, z;

    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    float dot(const Vector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    float magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalize() const {
        float mag = magnitude();
        if (mag < 0.0001f) return Vector3();
        return Vector3(x / mag, y / mag, z / mag);
    }
};

// Simple 4x4 matrix class for transformations
class Matrix4 {
public:
    float m[4][4];

    Matrix4() {
        // Initialize as identity matrix
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }

    static Matrix4 translation(const Vector3& v) {
        Matrix4 result;
        result.m[0][3] = v.x;
        result.m[1][3] = v.y;
        result.m[2][3] = v.z;
        return result;
    }

    static Matrix4 rotationX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix4 result;
        result.m[1][1] = c;
        result.m[1][2] = -s;
        result.m[2][1] = s;
        result.m[2][2] = c;
        return result;
    }

    static Matrix4 rotationY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix4 result;
        result.m[0][0] = c;
        result.m[0][2] = s;
        result.m[2][0] = -s;
        result.m[2][2] = c;
        return result;
    }

    static Matrix4 rotationZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        Matrix4 result;
        result.m[0][0] = c;
        result.m[0][1] = -s;
        result.m[1][0] = s;
        result.m[1][1] = c;
        return result;
    }

    static Matrix4 scale(const Vector3& v) {
        Matrix4 result;
        result.m[0][0] = v.x;
        result.m[1][1] = v.y;
        result.m[2][2] = v.z;
        return result;
    }

    static Matrix4 perspective(float fov, float aspectRatio, float near, float far) {
        float tanHalfFov = std::tan(fov / 2.0f);
        Matrix4 result;
        result.m[0][0] = 1.0f / (aspectRatio * tanHalfFov);
        result.m[1][1] = 1.0f / tanHalfFov;
        result.m[2][2] = -(far + near) / (far - near);
        result.m[2][3] = -(2.0f * far * near) / (far - near);
        result.m[3][2] = -1.0f;
        result.m[3][3] = 0.0f;
        return result;
    }

    static Matrix4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
        Vector3 zaxis = (eye - target).normalize();
        Vector3 xaxis = up.cross(zaxis).normalize();
        Vector3 yaxis = zaxis.cross(xaxis);

        Matrix4 result;
        result.m[0][0] = xaxis.x;
        result.m[0][1] = xaxis.y;
        result.m[0][2] = xaxis.z;
        result.m[0][3] = -xaxis.dot(eye);

        result.m[1][0] = yaxis.x;
        result.m[1][1] = yaxis.y;
        result.m[1][2] = yaxis.z;
        result.m[1][3] = -yaxis.dot(eye);

        result.m[2][0] = zaxis.x;
        result.m[2][1] = zaxis.y;
        result.m[2][2] = zaxis.z;
        result.m[2][3] = -zaxis.dot(eye);

        return result;
    }

    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = 0.0f;
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }

    Vector3 transform(const Vector3& v) const {
        float x = v.x * m[0][0] + v.y * m[0][1] + v.z * m[0][2] + m[0][3];
        float y = v.x * m[1][0] + v.y * m[1][1] + v.z * m[1][2] + m[1][3];
        float z = v.x * m[2][0] + v.y * m[2][1] + v.z * m[2][2] + m[2][3];
        float w = v.x * m[3][0] + v.y * m[3][1] + v.z * m[3][2] + m[3][3];

        if (std::abs(w) > 0.0001f) {
            return Vector3(x / w, y / w, z / w);
        }
        return Vector3(x, y, z);
    }
};

// Vertex structure
struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector3 color;
};

// Triangle structure
struct Triangle {
    Vertex vertices[3];
};

// Mesh class
class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<int> indices;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;

    Mesh() : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1) {}

    void applyTransform(const Matrix4& transform) {
        for (auto& vertex : vertices) {
            vertex.position = transform.transform(vertex.position);
            
            // Transform normals (ignoring translation)
            Matrix4 normalTransform;
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    normalTransform.m[i][j] = transform.m[i][j];
                }
            }
            vertex.normal = normalTransform.transform(vertex.normal).normalize();
        }
    }

    Matrix4 getModelMatrix() const {
        Matrix4 translationMatrix = Matrix4::translation(position);
        Matrix4 rotationMatrixX = Matrix4::rotationX(rotation.x);
        Matrix4 rotationMatrixY = Matrix4::rotationY(rotation.y);
        Matrix4 rotationMatrixZ = Matrix4::rotationZ(rotation.z);
        Matrix4 scaleMatrix = Matrix4::scale(scale);

        return translationMatrix * rotationMatrixZ * rotationMatrixY * rotationMatrixX * scaleMatrix;
    }

    static Mesh createCube(float size = 1.0f) {
        Mesh mesh;
        float halfSize = size / 2.0f;

        // Define the 8 vertices of the cube
        Vertex vertices[8] = {
            // Front face vertices
            { Vector3(-halfSize, -halfSize, halfSize), Vector3(0, 0, 1), Vector3(1, 0, 0) },  // Bottom-left
            { Vector3(halfSize, -halfSize, halfSize), Vector3(0, 0, 1), Vector3(0, 1, 0) },   // Bottom-right
            { Vector3(halfSize, halfSize, halfSize), Vector3(0, 0, 1), Vector3(0, 0, 1) },    // Top-right
            { Vector3(-halfSize, halfSize, halfSize), Vector3(0, 0, 1), Vector3(1, 1, 0) },   // Top-left
            
            // Back face vertices
            { Vector3(-halfSize, -halfSize, -halfSize), Vector3(0, 0, -1), Vector3(0, 1, 1) }, // Bottom-left
            { Vector3(halfSize, -halfSize, -halfSize), Vector3(0, 0, -1), Vector3(1, 0, 1) },  // Bottom-right
            { Vector3(halfSize, halfSize, -halfSize), Vector3(0, 0, -1), Vector3(1, 1, 1) },   // Top-right
            { Vector3(-halfSize, halfSize, -halfSize), Vector3(0, 0, -1), Vector3(0.5, 0.5, 0.5) } // Top-left
        };

        // Define the 12 triangles (2 per face * 6 faces)
        int indices[36] = {
            // Front face
            0, 1, 2, 0, 2, 3,
            // Back face
            5, 4, 7, 5, 7, 6,
            // Left face
            4, 0, 3, 4, 3, 7,
            // Right face
            1, 5, 6, 1, 6, 2,
            // Top face
            3, 2, 6, 3, 6, 7,
            // Bottom face
            4, 5, 1, 4, 1, 0
        };

        mesh.vertices.assign(vertices, vertices + 8);
        mesh.indices.assign(indices, indices + 36);

        // Set normals for each face
        for (int i = 0; i < 36; i += 3) {
            Vertex& v0 = mesh.vertices[mesh.indices[i]];
            Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
            Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];
            
            Vector3 edge1 = v1.position - v0.position;
            Vector3 edge2 = v2.position - v0.position;
            Vector3 normal = edge1.cross(edge2).normalize();
            
            v0.normal = normal;
            v1.normal = normal;
            v2.normal = normal;
        }

        return mesh;
    }

    static Mesh createSphere(float radius = 1.0f, int segments = 16) {
        Mesh mesh;
        
        // Create vertices
        for (int lat = 0; lat <= segments; lat++) {
            float theta = lat * M_PI / segments;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            
            for (int lon = 0; lon <= segments; lon++) {
                float phi = lon * 2 * M_PI / segments;
                float sinPhi = std::sin(phi);
                float cosPhi = std::cos(phi);
                
                float x = cosPhi * sinTheta;
                float y = cosTheta;
                float z = sinPhi * sinTheta;
                
                Vertex vertex;
                vertex.position = Vector3(x, y, z) * radius;
                vertex.normal = Vector3(x, y, z).normalize();
                vertex.color = Vector3((x + 1) / 2, (y + 1) / 2, (z + 1) / 2);
                
                mesh.vertices.push_back(vertex);
            }
        }
        
        // Create indices
        for (int lat = 0; lat < segments; lat++) {
            for (int lon = 0; lon < segments; lon++) {
                int current = lat * (segments + 1) + lon;
                int next = current + segments + 1;
                
                mesh.indices.push_back(current);
                mesh.indices.push_back(current + 1);
                mesh.indices.push_back(next + 1);
                
                mesh.indices.push_back(current);
                mesh.indices.push_back(next + 1);
                mesh.indices.push_back(next);
            }
        }
        
        return mesh;
    }
};

// Camera class
class Camera {
public:
    Vector3 position;
    Vector3 target;
    Vector3 up;
    float fov;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    Camera(float fov = 60.0f, float aspectRatio = 16.0f / 9.0f, float nearPlane = 0.1f, float farPlane = 100.0f)
        : position(0, 0, 5), target(0, 0, 0), up(0, 1, 0),
          fov(fov * M_PI / 180.0f), aspectRatio(aspectRatio),
          nearPlane(nearPlane), farPlane(farPlane) {}

    Matrix4 getViewMatrix() const {
        return Matrix4::lookAt(position, target, up);
    }

    Matrix4 getProjectionMatrix() const {
        return Matrix4::perspective(fov, aspectRatio, nearPlane, farPlane);
    }
};

// Light class
class Light {
public:
    Vector3 position;
    Vector3 color;
    float intensity;

    Light(const Vector3& position = Vector3(0, 5, 0), 
          const Vector3& color = Vector3(1, 1, 1), 
          float intensity = 1.0f)
        : position(position), color(color), intensity(intensity) {}
};

// Material class
class Material {
public:
    Vector3 ambient;
    Vector3 diffuse;
    Vector3 specular;
    float shininess;

    Material(const Vector3& ambient = Vector3(0.1f, 0.1f, 0.1f),
             const Vector3& diffuse = Vector3(0.7f, 0.7f, 0.7f),
             const Vector3& specular = Vector3(1.0f, 1.0f, 1.0f),
             float shininess = 32.0f)
        : ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}
};

// Renderer class (ASCII-based for simplicity)
class Renderer {
private:
    int width;
    int height;
    std::vector<char> frameBuffer;
    std::vector<float> depthBuffer;
    const char* asciiChars = " .:-=+*#%@";
    int asciiCharsLength;

public:
    Renderer(int width = 80, int height = 40)
        : width(width), height(height), asciiCharsLength(strlen(asciiChars)) {
        frameBuffer.resize(width * height, ' ');
        depthBuffer.resize(width * height, std::numeric_limits<float>::infinity());
    }

    void clear() {
        std::fill(frameBuffer.begin(), frameBuffer.end(), ' ');
        std::fill(depthBuffer.begin(), depthBuffer.end(), std::numeric_limits<float>::infinity());
    }

    void setPixel(int x, int y, float depth, float intensity) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        
        int index = y * width + x;
        if (depth < depthBuffer[index]) {
            depthBuffer[index] = depth;
            int charIndex = std::min(asciiCharsLength - 1, static_cast<int>(intensity * asciiCharsLength));
            frameBuffer[index] = asciiChars[charIndex];
        }
    }

    void render(const std::vector<Mesh>& meshes, const Camera& camera, const std::vector<Light>& lights) {
        clear();
        
        Matrix4 viewMatrix = camera.getViewMatrix();
        Matrix4 projectionMatrix = camera.getProjectionMatrix();
        
        for (const auto& mesh : meshes) {
            Matrix4 modelMatrix = mesh.getModelMatrix();
            Matrix4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
            
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                Vertex v0 = mesh.vertices[mesh.indices[i]];
                Vertex v1 = mesh.vertices[mesh.indices[i + 1]];
                Vertex v2 = mesh.vertices[mesh.indices[i + 2]];
                
                // Transform vertices to clip space
                Vector3 p0 = mvpMatrix.transform(v0.position);
                Vector3 p1 = mvpMatrix.transform(v1.position);
                Vector3 p2 = mvpMatrix.transform(v2.position);
                
                // Simple backface culling
                Vector3 normal = (p1 - p0).cross(p2 - p0);
                if (normal.z < 0) continue;
                
                // Convert to screen space
                int x0 = static_cast<int>((p0.x + 1.0f) * 0.5f * width);
                int y0 = static_cast<int>((1.0f - (p0.y + 1.0f) * 0.5f) * height);
                int x1 = static_cast<int>((p1.x + 1.0f) * 0.5f * width);
                int y1 = static_cast<int>((1.0f - (p1.y + 1.0f) * 0.5f) * height);
                int x2 = static_cast<int>((p2.x + 1.0f) * 0.5f * width);
                int y2 = static_cast<int>((1.0f - (p2.y + 1.0f) * 0.5f) * height);
                
                // Calculate lighting intensity (simplified)
                float intensity = 0.2f;  // Ambient light
                for (const auto& light : lights) {
                    Vector3 lightDir = (light.position - v0.position).normalize();
                    float diff = std::max(0.0f, lightDir.dot(v0.normal));
                    intensity += diff * light.intensity * 0.8f;
                }
                intensity = std::min(1.0f, intensity);
                
                // Draw the triangle (using a simple rasterization approach)
                drawTriangle(x0, y0, p0.z, x1, y1, p1.z, x2, y2, p2.z, intensity);
            }
        }
        
        // Display the frame buffer
        display();
    }

    void drawTriangle(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2, float intensity) {
        // Sort vertices by y-coordinate
        if (y0 > y1) {
            std::swap(x0, x1);
            std::swap(y0, y1);
            std::swap(z0, z1);
        }
        if (y0 > y2) {
            std::swap(x0, x2);
            std::swap(y0, y2);
            std::swap(z0, z2);
        }
        if (y1 > y2) {
            std::swap(x1, x2);
            std::swap(y1, y2);
            std::swap(z1, z2);
        }

        // Compute slopes
        float dx01 = x1 - x0;
        float dy01 = y1 - y0;
        float dz01 = z1 - z0;
        
        float dx02 = x2 - x0;
        float dy02 = y2 - y0;
        float dz02 = z2 - z0;
        
        float dx12 = x2 - x1;
        float dy12 = y2 - y1;
        float dz12 = z2 - z1;

        // Compute scanline endpoints
        float sx = x0, ex = x0;
        float sz = z0, ez = z0;
        
        // Rasterize the triangle
        for (int y = y0; y <= y2; y++) {
            if (y < 0 || y >= height) continue;
            
            float t1 = (dy01 != 0) ? (float)(y - y0) / dy01 : 1.0f;
            float t2 = (dy02 != 0) ? (float)(y - y0) / dy02 : 1.0f;
            
            int startX = static_cast<int>(x0 + t1 * dx01);
            float startZ = z0 + t1 * dz01;
            
            int endX = static_cast<int>(x0 + t2 * dx02);
            float endZ = z0 + t2 * dz02;
            
            // Swap if necessary
            if (startX > endX) {
                std::swap(startX, endX);
                std::swap(startZ, endZ);
            }
            
            // Draw horizontal scanline
            float dz = (endX != startX) ? (endZ - startZ) / (endX - startX) : 0;
            float z = startZ;
            
            for (int x = startX; x <= endX; x++) {
                if (x >= 0 && x < width) {
                    setPixel(x, y, z, intensity);
                }
                z += dz;
            }
            
            // Update endpoints for next scanline
            if (y == y1) {
                dx01 = dx12;
                dy01 = dy12;
                dz01 = dz12;
                sx = x1;
                sz = z1;
            }
        }
    }

    void display() {
        system("cls");  // Clear console (Windows)
        // system("clear");  // Clear console (Unix/Linux)
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                std::cout << frameBuffer[y * width + x];
            }
            std::cout << std::endl;
        }
    }
};

// Game Engine class
class GameEngine {
private:
    std::vector<Mesh> meshes;
    std::vector<Light> lights;
    Camera camera;
    Renderer renderer;
    bool running;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameTime;
    float deltaTime;
    std::map<std::string, std::function<void(float)>> updateCallbacks;

public:
    GameEngine(int width = 80, int height = 40)
        : renderer(width, height), running(false), deltaTime(0.0f) {
        // Add a default light
        lights.push_back(Light(Vector3(5, 5, 5)));
    }

    void addMesh(const Mesh& mesh) {
        meshes.push_back(mesh);
    }

    void addLight(const Light& light) {
        lights.push_back(light);
    }

    void setCamera(const Camera& cam) {
        camera = cam;
    }

    void registerUpdateCallback(const std::string& name, std::function<void(float)> callback) {
        updateCallbacks[name] = callback;
    }

    void unregisterUpdateCallback(const std::string& name) {
        updateCallbacks.erase(name);
    }

    void start() {
        running = true;
        lastFrameTime = std::chrono::high_resolution_clock::now();
        
        while (running) {
            // Calculate delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            deltaTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
            lastFrameTime = currentTime;
            
            // Update game state
            update(deltaTime);
            
            // Render frame
            renderer.render(meshes, camera, lights);
            
            // Limit frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(33));  // ~30 FPS
        }
    }

    void stop() {
        running = false;
    }

private:
    void update(float dt) {
        // Call all registered update callbacks
        for (const auto& callback : updateCallbacks) {
            callback.second(dt);
        }
    }
};

// Demo application
int main() {
    // Create game engine
    GameEngine engine(100, 40);
    
    // Create meshes
    Mesh cube = Mesh::createCube(2.0f);
    cube.position = Vector3(0, 0, 0);
    
    Mesh sphere = Mesh::createSphere(1.0f, 12);
    sphere.position = Vector3(3, 0, 0);
    
    // Add meshes to engine
    engine.addMesh(cube);
    engine.addMesh(sphere);
    
    // Set up camera
    Camera camera(60.0f, 2.5f, 0.1f, 100.0f);
    camera.position = Vector3(0, 0, 10);
    engine.setCamera(camera);
    
    // Add lights
    engine.addLight(Light(Vector3(5, 5, 5), Vector3(1, 1, 1), 1.0f));
    engine.addLight(Light(Vector3(-5, 3, 0), Vector3(0.5f, 0.5f, 1.0f), 0.7f));
    
    // Register update callback for animation
    float angle = 0.0f;
    engine.registerUpdateCallback("rotation", [&](float dt) {
        angle += dt * 0.5f;
        cube.rotation = Vector3(angle, angle * 1.5f, 0);
        sphere.rotation = Vector3(0, angle * 2.0f, angle);
    });
    
    std::cout << "3D Game Engine Demo" << std::endl;
    std::cout << "===================" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "Starting in 2 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Start the engine
    try {
        engine.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 
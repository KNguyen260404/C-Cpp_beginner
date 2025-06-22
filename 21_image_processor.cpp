#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <limits>
#include <iomanip>

using namespace std;

// Structure to represent a pixel in RGB format
struct Pixel {
    unsigned char r, g, b;
    
    Pixel() : r(0), g(0), b(0) {}
    Pixel(unsigned char red, unsigned char green, unsigned char blue) : r(red), g(green), b(blue) {}
};

// Structure to represent a BMP file header
#pragma pack(push, 1)
struct BMPHeader {
    // BMP File Header (14 bytes)
    char signature[2];      // "BM"
    uint32_t fileSize;      // Size of the BMP file in bytes
    uint16_t reserved1;     // Reserved, must be 0
    uint16_t reserved2;     // Reserved, must be 0
    uint32_t dataOffset;    // Offset to the start of image data
    
    // DIB Header (40 bytes for BITMAPINFOHEADER)
    uint32_t headerSize;    // Size of the DIB header (40 bytes)
    int32_t width;          // Width of the image in pixels
    int32_t height;         // Height of the image in pixels
    uint16_t planes;        // Number of color planes, must be 1
    uint16_t bitsPerPixel;  // Number of bits per pixel (24 for RGB)
    uint32_t compression;   // Compression method (0 for no compression)
    uint32_t imageSize;     // Size of the image data in bytes
    int32_t xPixelsPerMeter; // Horizontal resolution in pixels per meter
    int32_t yPixelsPerMeter; // Vertical resolution in pixels per meter
    uint32_t colorsUsed;    // Number of colors in the color palette
    uint32_t colorsImportant; // Number of important colors
};
#pragma pack(pop)

// Class to represent an image
class Image {
private:
    int width;
    int height;
    vector<vector<Pixel>> pixels;
    
public:
    // Default constructor
    Image() : width(0), height(0) {}
    
    // Constructor with dimensions
    Image(int w, int h) : width(w), height(h) {
        pixels.resize(height, vector<Pixel>(width));
    }
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    
    // Get a pixel at a specific position
    Pixel getPixel(int x, int y) const {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            return pixels[y][x];
        }
        return Pixel(); // Return black pixel if out of bounds
    }
    
    // Set a pixel at a specific position
    void setPixel(int x, int y, const Pixel& pixel) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y][x] = pixel;
        }
    }
    
    // Load a BMP image from file
    bool loadBMP(const string& filename) {
        ifstream file(filename, ios::binary);
        if (!file) {
            cerr << "Error: Could not open file " << filename << endl;
            return false;
        }
        
        // Read the BMP header
        BMPHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(BMPHeader));
        
        // Check if this is a valid BMP file
        if (header.signature[0] != 'B' || header.signature[1] != 'M') {
            cerr << "Error: Not a valid BMP file" << endl;
            return false;
        }
        
        // Check if this is a 24-bit BMP file
        if (header.bitsPerPixel != 24) {
            cerr << "Error: Only 24-bit BMP files are supported" << endl;
            return false;
        }
        
        // Set the image dimensions
        width = header.width;
        height = abs(header.height); // Height can be negative for top-down BMPs
        
        // Resize the pixel array
        pixels.resize(height, vector<Pixel>(width));
        
        // Calculate row padding (rows are padded to multiples of 4 bytes)
        int padding = (4 - ((width * 3) % 4)) % 4;
        
        // Seek to the start of the pixel data
        file.seekg(header.dataOffset, ios::beg);
        
        // Read the pixel data
        for (int y = 0; y < height; y++) {
            int row = (header.height < 0) ? y : (height - 1 - y); // Handle top-down BMPs
            
            for (int x = 0; x < width; x++) {
                unsigned char color[3];
                file.read(reinterpret_cast<char*>(color), 3);
                
                // BMP stores pixels in BGR format
                pixels[row][x].b = color[0];
                pixels[row][x].g = color[1];
                pixels[row][x].r = color[2];
            }
            
            // Skip padding bytes
            file.seekg(padding, ios::cur);
        }
        
        file.close();
        return true;
    }
    
    // Save a BMP image to file
    bool saveBMP(const string& filename) const {
        ofstream file(filename, ios::binary);
        if (!file) {
            cerr << "Error: Could not create file " << filename << endl;
            return false;
        }
        
        // Calculate row padding (rows are padded to multiples of 4 bytes)
        int padding = (4 - ((width * 3) % 4)) % 4;
        
        // Calculate the size of the pixel data
        int dataSize = height * (width * 3 + padding);
        
        // Create the BMP header
        BMPHeader header;
        header.signature[0] = 'B';
        header.signature[1] = 'M';
        header.fileSize = sizeof(BMPHeader) + dataSize;
        header.reserved1 = 0;
        header.reserved2 = 0;
        header.dataOffset = sizeof(BMPHeader);
        header.headerSize = 40;
        header.width = width;
        header.height = height;
        header.planes = 1;
        header.bitsPerPixel = 24;
        header.compression = 0;
        header.imageSize = dataSize;
        header.xPixelsPerMeter = 2835; // 72 DPI
        header.yPixelsPerMeter = 2835; // 72 DPI
        header.colorsUsed = 0;
        header.colorsImportant = 0;
        
        // Write the header
        file.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
        
        // Write the pixel data
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                // BMP stores pixels in BGR format
                unsigned char color[3] = {pixels[y][x].b, pixels[y][x].g, pixels[y][x].r};
                file.write(reinterpret_cast<const char*>(color), 3);
            }
            
            // Write padding bytes
            unsigned char pad = 0;
            for (int p = 0; p < padding; p++) {
                file.write(reinterpret_cast<const char*>(&pad), 1);
            }
        }
        
        file.close();
        return true;
    }
    
    // Create a new blank image with the specified dimensions
    void createBlank(int w, int h) {
        width = w;
        height = h;
        pixels.resize(height, vector<Pixel>(width));
    }
    
    // Convert to grayscale
    void convertToGrayscale() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Use the luminosity method for grayscale conversion
                unsigned char gray = static_cast<unsigned char>(
                    0.299 * pixels[y][x].r + 
                    0.587 * pixels[y][x].g + 
                    0.114 * pixels[y][x].b);
                
                pixels[y][x].r = gray;
                pixels[y][x].g = gray;
                pixels[y][x].b = gray;
            }
        }
    }
    
    // Adjust brightness
    void adjustBrightness(int amount) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Adjust each color channel
                pixels[y][x].r = clamp(pixels[y][x].r + amount, 0, 255);
                pixels[y][x].g = clamp(pixels[y][x].g + amount, 0, 255);
                pixels[y][x].b = clamp(pixels[y][x].b + amount, 0, 255);
            }
        }
    }
    
    // Adjust contrast
    void adjustContrast(double factor) {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Adjust each color channel
                pixels[y][x].r = clamp(static_cast<int>(128 + factor * (pixels[y][x].r - 128)), 0, 255);
                pixels[y][x].g = clamp(static_cast<int>(128 + factor * (pixels[y][x].g - 128)), 0, 255);
                pixels[y][x].b = clamp(static_cast<int>(128 + factor * (pixels[y][x].b - 128)), 0, 255);
            }
        }
    }
    
    // Invert colors
    void invertColors() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Invert each color channel
                pixels[y][x].r = 255 - pixels[y][x].r;
                pixels[y][x].g = 255 - pixels[y][x].g;
                pixels[y][x].b = 255 - pixels[y][x].b;
            }
        }
    }
    
    // Apply blur filter
    void applyBlur(int radius) {
        // Create a copy of the original image
        vector<vector<Pixel>> originalPixels = pixels;
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int totalR = 0, totalG = 0, totalB = 0;
                int count = 0;
                
                // Average the pixels in the neighborhood
                for (int dy = -radius; dy <= radius; dy++) {
                    for (int dx = -radius; dx <= radius; dx++) {
                        int nx = x + dx;
                        int ny = y + dy;
                        
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            totalR += originalPixels[ny][nx].r;
                            totalG += originalPixels[ny][nx].g;
                            totalB += originalPixels[ny][nx].b;
                            count++;
                        }
                    }
                }
                
                // Set the pixel to the average color
                if (count > 0) {
                    pixels[y][x].r = static_cast<unsigned char>(totalR / count);
                    pixels[y][x].g = static_cast<unsigned char>(totalG / count);
                    pixels[y][x].b = static_cast<unsigned char>(totalB / count);
                }
            }
        }
    }
    
    // Apply edge detection
    void applyEdgeDetection() {
        // Create a copy of the original image
        vector<vector<Pixel>> originalPixels = pixels;
        
        // Convert to grayscale first
        convertToGrayscale();
        
        // Sobel operators for edge detection
        const int sobelX[3][3] = {
            {-1, 0, 1},
            {-2, 0, 2},
            {-1, 0, 1}
        };
        
        const int sobelY[3][3] = {
            {-1, -2, -1},
            {0, 0, 0},
            {1, 2, 1}
        };
        
        // Apply Sobel operator
        for (int y = 1; y < height - 1; y++) {
            for (int x = 1; x < width - 1; x++) {
                int gx = 0, gy = 0;
                
                // Apply the Sobel kernels
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int pixel = originalPixels[y + ky][x + kx].r; // Use red channel (grayscale)
                        gx += pixel * sobelX[ky + 1][kx + 1];
                        gy += pixel * sobelY[ky + 1][kx + 1];
                    }
                }
                
                // Calculate the magnitude of the gradient
                int magnitude = static_cast<int>(sqrt(gx * gx + gy * gy));
                magnitude = clamp(magnitude, 0, 255);
                
                // Set the pixel to the edge value
                pixels[y][x].r = static_cast<unsigned char>(magnitude);
                pixels[y][x].g = static_cast<unsigned char>(magnitude);
                pixels[y][x].b = static_cast<unsigned char>(magnitude);
            }
        }
    }
    
    // Resize the image
    void resize(int newWidth, int newHeight) {
        if (newWidth <= 0 || newHeight <= 0) {
            return;
        }
        
        // Create a new pixel array with the new dimensions
        vector<vector<Pixel>> newPixels(newHeight, vector<Pixel>(newWidth));
        
        // Calculate scaling factors
        double scaleX = static_cast<double>(width) / newWidth;
        double scaleY = static_cast<double>(height) / newHeight;
        
        // Resize using nearest neighbor interpolation
        for (int y = 0; y < newHeight; y++) {
            for (int x = 0; x < newWidth; x++) {
                int srcX = static_cast<int>(x * scaleX);
                int srcY = static_cast<int>(y * scaleY);
                
                srcX = clamp(srcX, 0, width - 1);
                srcY = clamp(srcY, 0, height - 1);
                
                newPixels[y][x] = pixels[srcY][srcX];
            }
        }
        
        // Update the image dimensions and pixels
        width = newWidth;
        height = newHeight;
        pixels = newPixels;
    }
    
    // Rotate the image
    void rotate(double angleDegrees) {
        // Convert angle to radians
        double angleRadians = angleDegrees * M_PI / 180.0;
        
        // Calculate the center of the image
        double centerX = width / 2.0;
        double centerY = height / 2.0;
        
        // Calculate the dimensions of the rotated image
        double cosAngle = cos(angleRadians);
        double sinAngle = sin(angleRadians);
        
        int newWidth = static_cast<int>(abs(width * cosAngle) + abs(height * sinAngle));
        int newHeight = static_cast<int>(abs(width * sinAngle) + abs(height * cosAngle));
        
        // Create a new pixel array with the new dimensions
        vector<vector<Pixel>> newPixels(newHeight, vector<Pixel>(newWidth, Pixel()));
        
        // Calculate the new center
        double newCenterX = newWidth / 2.0;
        double newCenterY = newHeight / 2.0;
        
        // Rotate the image
        for (int y = 0; y < newHeight; y++) {
            for (int x = 0; x < newWidth; x++) {
                // Calculate the source pixel coordinates
                double srcX = cosAngle * (x - newCenterX) - sinAngle * (y - newCenterY) + centerX;
                double srcY = sinAngle * (x - newCenterX) + cosAngle * (y - newCenterY) + centerY;
                
                // Check if the source pixel is within the original image
                if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height) {
                    // Use bilinear interpolation
                    int x1 = static_cast<int>(srcX);
                    int y1 = static_cast<int>(srcY);
                    int x2 = min(x1 + 1, width - 1);
                    int y2 = min(y1 + 1, height - 1);
                    
                    double dx = srcX - x1;
                    double dy = srcY - y1;
                    
                    // Get the four neighboring pixels
                    Pixel p1 = pixels[y1][x1];
                    Pixel p2 = pixels[y1][x2];
                    Pixel p3 = pixels[y2][x1];
                    Pixel p4 = pixels[y2][x2];
                    
                    // Interpolate the color channels
                    newPixels[y][x].r = static_cast<unsigned char>(
                        (1 - dx) * (1 - dy) * p1.r +
                        dx * (1 - dy) * p2.r +
                        (1 - dx) * dy * p3.r +
                        dx * dy * p4.r);
                    
                    newPixels[y][x].g = static_cast<unsigned char>(
                        (1 - dx) * (1 - dy) * p1.g +
                        dx * (1 - dy) * p2.g +
                        (1 - dx) * dy * p3.g +
                        dx * dy * p4.g);
                    
                    newPixels[y][x].b = static_cast<unsigned char>(
                        (1 - dx) * (1 - dy) * p1.b +
                        dx * (1 - dy) * p2.b +
                        (1 - dx) * dy * p3.b +
                        dx * dy * p4.b);
                }
            }
        }
        
        // Update the image dimensions and pixels
        width = newWidth;
        height = newHeight;
        pixels = newPixels;
    }
    
    // Crop the image
    void crop(int x, int y, int w, int h) {
        // Validate the crop region
        x = clamp(x, 0, width - 1);
        y = clamp(y, 0, height - 1);
        w = clamp(w, 1, width - x);
        h = clamp(h, 1, height - y);
        
        // Create a new pixel array with the cropped dimensions
        vector<vector<Pixel>> newPixels(h, vector<Pixel>(w));
        
        // Copy the pixels from the crop region
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                newPixels[j][i] = pixels[y + j][x + i];
            }
        }
        
        // Update the image dimensions and pixels
        width = w;
        height = h;
        pixels = newPixels;
    }
    
    // Apply a sepia filter
    void applySepia() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Calculate sepia values
                int tr = static_cast<int>(0.393 * pixels[y][x].r + 0.769 * pixels[y][x].g + 0.189 * pixels[y][x].b);
                int tg = static_cast<int>(0.349 * pixels[y][x].r + 0.686 * pixels[y][x].g + 0.168 * pixels[y][x].b);
                int tb = static_cast<int>(0.272 * pixels[y][x].r + 0.534 * pixels[y][x].g + 0.131 * pixels[y][x].b);
                
                // Clamp the values
                pixels[y][x].r = clamp(tr, 0, 255);
                pixels[y][x].g = clamp(tg, 0, 255);
                pixels[y][x].b = clamp(tb, 0, 255);
            }
        }
    }
    
    // Helper function to clamp a value between min and max
    template<typename T>
    static T clamp(T value, T min, T max) {
        return (value < min) ? min : ((value > max) ? max : value);
    }
};

// Function to display the main menu
void displayMenu() {
    cout << "\n===== Image Processor =====" << endl;
    cout << "1. Load Image" << endl;
    cout << "2. Save Image" << endl;
    cout << "3. Create Blank Image" << endl;
    cout << "4. Convert to Grayscale" << endl;
    cout << "5. Adjust Brightness" << endl;
    cout << "6. Adjust Contrast" << endl;
    cout << "7. Invert Colors" << endl;
    cout << "8. Apply Blur" << endl;
    cout << "9. Apply Edge Detection" << endl;
    cout << "10. Resize Image" << endl;
    cout << "11. Rotate Image" << endl;
    cout << "12. Crop Image" << endl;
    cout << "13. Apply Sepia Filter" << endl;
    cout << "0. Exit" << endl;
    cout << "=========================" << endl;
    cout << "Enter your choice: ";
}

// Main function
int main() {
    Image image;
    bool imageLoaded = false;
    int choice;
    string filename;
    
    cout << "Welcome to Image Processor!" << endl;
    cout << "Note: This program supports 24-bit BMP files." << endl;
    
    while (true) {
        displayMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }
        
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1: { // Load Image
                cout << "Enter the filename to load: ";
                getline(cin, filename);
                
                if (image.loadBMP(filename)) {
                    cout << "Image loaded successfully." << endl;
                    cout << "Dimensions: " << image.getWidth() << "x" << image.getHeight() << " pixels" << endl;
                    imageLoaded = true;
                } else {
                    cout << "Failed to load image." << endl;
                }
                break;
            }
                
            case 2: { // Save Image
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                cout << "Enter the filename to save: ";
                getline(cin, filename);
                
                if (image.saveBMP(filename)) {
                    cout << "Image saved successfully." << endl;
                } else {
                    cout << "Failed to save image." << endl;
                }
                break;
            }
                
            case 3: { // Create Blank Image
                int width, height;
                cout << "Enter the width of the image: ";
                cin >> width;
                cout << "Enter the height of the image: ";
                cin >> height;
                
                if (width <= 0 || height <= 0) {
                    cout << "Invalid dimensions. Width and height must be positive." << endl;
                } else {
                    image.createBlank(width, height);
                    cout << "Blank image created with dimensions " << width << "x" << height << " pixels." << endl;
                    imageLoaded = true;
                }
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 4: { // Convert to Grayscale
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                image.convertToGrayscale();
                cout << "Image converted to grayscale." << endl;
                break;
            }
                
            case 5: { // Adjust Brightness
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                int amount;
                cout << "Enter brightness adjustment amount (-255 to 255): ";
                cin >> amount;
                
                image.adjustBrightness(amount);
                cout << "Brightness adjusted by " << amount << "." << endl;
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 6: { // Adjust Contrast
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                double factor;
                cout << "Enter contrast factor (0.0 to 3.0, 1.0 is unchanged): ";
                cin >> factor;
                
                image.adjustContrast(factor);
                cout << "Contrast adjusted by factor " << factor << "." << endl;
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 7: { // Invert Colors
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                image.invertColors();
                cout << "Colors inverted." << endl;
                break;
            }
                
            case 8: { // Apply Blur
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                int radius;
                cout << "Enter blur radius (1-10): ";
                cin >> radius;
                
                radius = max(1, min(10, radius));
                image.applyBlur(radius);
                cout << "Blur filter applied with radius " << radius << "." << endl;
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 9: { // Apply Edge Detection
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                image.applyEdgeDetection();
                cout << "Edge detection applied." << endl;
                break;
            }
                
            case 10: { // Resize Image
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                int newWidth, newHeight;
                cout << "Enter new width: ";
                cin >> newWidth;
                cout << "Enter new height: ";
                cin >> newHeight;
                
                if (newWidth <= 0 || newHeight <= 0) {
                    cout << "Invalid dimensions. Width and height must be positive." << endl;
                } else {
                    image.resize(newWidth, newHeight);
                    cout << "Image resized to " << newWidth << "x" << newHeight << " pixels." << endl;
                }
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 11: { // Rotate Image
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                double angle;
                cout << "Enter rotation angle in degrees: ";
                cin >> angle;
                
                image.rotate(angle);
                cout << "Image rotated by " << angle << " degrees." << endl;
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 12: { // Crop Image
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                int x, y, w, h;
                cout << "Enter crop region (x y width height): ";
                cin >> x >> y >> w >> h;
                
                if (x < 0 || y < 0 || w <= 0 || h <= 0) {
                    cout << "Invalid crop region. Coordinates must be non-negative and dimensions must be positive." << endl;
                } else {
                    image.crop(x, y, w, h);
                    cout << "Image cropped to " << w << "x" << h << " pixels." << endl;
                }
                
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                break;
            }
                
            case 13: { // Apply Sepia Filter
                if (!imageLoaded) {
                    cout << "No image loaded. Please load an image first." << endl;
                    break;
                }
                
                image.applySepia();
                cout << "Sepia filter applied." << endl;
                break;
            }
                
            case 0: // Exit
                cout << "Thank you for using Image Processor!" << endl;
                return 0;
                
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    }
    
    return 0;
} 
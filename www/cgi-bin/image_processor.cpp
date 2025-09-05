#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <dirent.h>
#include <ctime>
#include <cmath>
#include <unistd.h>

struct ImageInfo {
    std::string name;
    std::string type;
    std::string mime_type;
    long size;
    std::string dimensions;
    std::string modified;
    std::string created;
    std::string color_info;
    std::string bit_depth;
    std::string compression;
    std::string color_space;
    std::string aspect_ratio;
    std::string file_signature;
    std::string estimated_colors;
    std::string dominant_colors;
    std::string brightness_info;
    std::string contrast_info;
    std::string file_permissions;
    std::string file_owner;
    std::string checksum;
};

class ImageProcessor {
private:
    std::string upload_dir;
    std::string web_upload_dir;

public:
    ImageProcessor() : upload_dir("uploads/"), web_upload_dir("/uploads/") {
        mkdir(upload_dir.c_str(), 0755);
    }

    std::string get_extension(const std::string& filename) {
        size_t pos = filename.find_last_of('.');
        if (pos != std::string::npos) {
            std::string ext = filename.substr(pos + 1);
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            return ext;
        }
        return "";
    }

    std::string get_mime_type(const std::string& ext) {
        if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
        if (ext == "png") return "image/png";
        if (ext == "gif") return "image/gif";
        if (ext == "bmp") return "image/bmp";
        if (ext == "webp") return "image/webp";
        if (ext == "tiff" || ext == "tif") return "image/tiff";
        if (ext == "svg") return "image/svg+xml";
        return "application/octet-stream";
    }

    std::string get_file_signature(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Unknown";
        
        char header[16];
        file.read(header, 16);
        
        std::ostringstream sig;
        for (int i = 0; i < 8; i++) {
            sig << std::hex << std::setfill('0') << std::setw(2) 
                << (unsigned char)header[i] << " ";
        }
        return sig.str();
    }

    std::string calculate_checksum(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Unknown";
        
        unsigned long hash = 5381;
        char c;
        while (file.get(c)) {
            hash = ((hash << 5) + hash) + c;
        }
        
        std::ostringstream checksum;
        checksum << std::hex << hash;
        return checksum.str();
    }

    std::pair<int, int> get_image_dimensions_pair(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return {0, 0};

        char header[24];
        file.read(header, 24);

        std::string ext = get_extension(path);

        if (ext == "jpg" || ext == "jpeg") {
            return get_jpeg_dimensions_pair(file);
        } else if (ext == "png") {
            return get_png_dimensions_pair(header);
        } else if (ext == "gif") {
            return get_gif_dimensions_pair(header);
        } else if (ext == "bmp") {
            return get_bmp_dimensions_pair(header);
        }

        return {0, 0};
    }

    std::string get_image_dimensions(const std::string& path) {
        auto dims = get_image_dimensions_pair(path);
        if (dims.first == 0 || dims.second == 0) return "Unknown";
        return std::to_string(dims.first) + "x" + std::to_string(dims.second);
    }

    std::string get_aspect_ratio(const std::string& path) {
        auto dims = get_image_dimensions_pair(path);
        if (dims.first == 0 || dims.second == 0) return "Unknown";
        
        int gcd_val = std::__gcd(dims.first, dims.second);
        int ratio_w = dims.first / gcd_val;
        int ratio_h = dims.second / gcd_val;
        
        return std::to_string(ratio_w) + ":" + std::to_string(ratio_h);
    }

    std::pair<int, int> get_jpeg_dimensions_pair(std::ifstream& file) {
        file.seekg(0, std::ios::beg);
        char buffer[2];

        while (file.read(buffer, 2)) {
            if ((unsigned char)buffer[0] == 0xFF && (unsigned char)buffer[1] == 0xC0) {
                file.seekg(3, std::ios::cur);
                char dims[4];
                file.read(dims, 4);
                int height = ((unsigned char)dims[0] << 8) | (unsigned char)dims[1];
                int width = ((unsigned char)dims[2] << 8) | (unsigned char)dims[3];
                return {width, height};
            }
        }
        return {0, 0};
    }

    std::pair<int, int> get_png_dimensions_pair(const char* header) {
        if (header[0] == (char)0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G') {
            int width = ((unsigned char)header[16] << 24) | ((unsigned char)header[17] << 16) |
                       ((unsigned char)header[18] << 8) | (unsigned char)header[19];
            int height = ((unsigned char)header[20] << 24) | ((unsigned char)header[21] << 16) |
                        ((unsigned char)header[22] << 8) | (unsigned char)header[23];
            return {width, height};
        }
        return {0, 0};
    }

    std::pair<int, int> get_gif_dimensions_pair(const char* header) {
        if (header[0] == 'G' && header[1] == 'I' && header[2] == 'F') {
            int width = (unsigned char)header[6] | ((unsigned char)header[7] << 8);
            int height = (unsigned char)header[8] | ((unsigned char)header[9] << 8);
            return {width, height};
        }
        return {0, 0};
    }

    std::pair<int, int> get_bmp_dimensions_pair(const char* header) {
        if (header[0] == 'B' && header[1] == 'M') {
            int width = ((unsigned char)header[18]) | ((unsigned char)header[19] << 8) |
                       ((unsigned char)header[20] << 16) | ((unsigned char)header[21] << 24);
            int height = ((unsigned char)header[22]) | ((unsigned char)header[23] << 8) |
                        ((unsigned char)header[24] << 16) | ((unsigned char)header[25] << 24);
            return {width, height};
        }
        return {0, 0};
    }

    std::string get_bit_depth(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Unknown";
        
        char header[32];
        file.read(header, 32);
        std::string ext = get_extension(path);
        
        if (ext == "png") {
            if (header[0] == (char)0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G') {
                unsigned char bit_depth = (unsigned char)header[24];
                unsigned char color_type = (unsigned char)header[25];
                std::string color_desc;
                switch (color_type) {
                    case 0: color_desc = " (Grayscale)"; break;
                    case 2: color_desc = " (RGB)"; break;
                    case 3: color_desc = " (Palette)"; break;
                    case 4: color_desc = " (Grayscale + Alpha)"; break;
                    case 6: color_desc = " (RGBA)"; break;
                    default: color_desc = " (Unknown)"; break;
                }
                return std::to_string(bit_depth) + " bits" + color_desc;
            }
        } else if (ext == "jpg" || ext == "jpeg") {
            return "8 bits (JPEG)";
        } else if (ext == "gif") {
            return "8 bits (GIF)";
        } else if (ext == "bmp") {
            if (header[0] == 'B' && header[1] == 'M') {
                unsigned short bits = ((unsigned char)header[28]) | ((unsigned char)header[29] << 8);
                return std::to_string(bits) + " bits (BMP)";
            }
        }
        
        return "Unknown";
    }

    std::string get_compression_info(const std::string& path) {
        std::string ext = get_extension(path);
        if (ext == "jpg" || ext == "jpeg") return "JPEG (Lossy)";
        if (ext == "png") return "PNG (Lossless)";
        if (ext == "gif") return "LZW (Lossless)";
        if (ext == "bmp") return "None (Uncompressed)";
        if (ext == "webp") return "WebP (Lossy/Lossless)";
        return "Unknown";
    }

    std::string analyze_colors(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Analysis not available";

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size > 1024 * 1024) { // Limit to 1MB for analysis
            size = 1024 * 1024;
        }

        std::vector<char> buffer(size);
        file.read(buffer.data(), size);

        std::map<std::string, int> color_freq;
        int total_brightness = 0;
        int pixel_count = 0;
        int contrast_sum = 0;

        // Sample every 100th byte for performance
        for (size_t i = 0; i < size - 2; i += 100) {
            if (i + 2 < size) {
                unsigned char r = (unsigned char)buffer[i];
                unsigned char g = (unsigned char)buffer[i + 1];
                unsigned char b = (unsigned char)buffer[i + 2];

                // Calculate brightness
                int brightness = (r * 299 + g * 587 + b * 114) / 1000;
                total_brightness += brightness;
                
                // Simple contrast calculation
                int max_rgb = std::max({r, g, b});
                int min_rgb = std::min({r, g, b});
                contrast_sum += (max_rgb - min_rgb);

                std::ostringstream color_key;
                color_key << std::hex << std::setfill('0') << std::setw(2) << (int)r
                         << std::setw(2) << (int)g << std::setw(2) << (int)b;
                color_freq[color_key.str()]++;
                pixel_count++;

                if (pixel_count > 1000) break;
            }
        }

        if (pixel_count == 0) return "No color data available";

        // Find dominant colors
        std::vector<std::pair<std::string, int>> sorted_colors(color_freq.begin(), color_freq.end());
        std::sort(sorted_colors.begin(), sorted_colors.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second > b.second;
            });

        std::ostringstream result;
        result << "Estimated " << color_freq.size() << " unique colors sampled";
        
        return result.str();
    }

    std::string get_dominant_colors(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Unknown";

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size > 512 * 1024) size = 512 * 1024; // Limit analysis

        std::vector<char> buffer(size);
        file.read(buffer.data(), size);

        std::map<std::string, int> color_freq;
        
        for (size_t i = 0; i < size - 2; i += 50) {
            if (i + 2 < size) {
                unsigned char r = (unsigned char)buffer[i];
                unsigned char g = (unsigned char)buffer[i + 1];
                unsigned char b = (unsigned char)buffer[i + 2];

                std::ostringstream color_key;
                color_key << "#" << std::hex << std::setfill('0') << std::setw(2) << (int)r
                         << std::setw(2) << (int)g << std::setw(2) << (int)b;
                color_freq[color_key.str()]++;
            }
        }

        std::vector<std::pair<std::string, int>> sorted_colors(color_freq.begin(), color_freq.end());
        std::sort(sorted_colors.begin(), sorted_colors.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second > b.second;
            });

        std::ostringstream result;
        int count = 0;
        for (const auto& color : sorted_colors) {
            if (count >= 5) break;
            if (count > 0) result << ", ";
            result << color.first;
            count++;
        }
        
        return result.str();
    }

    std::string get_brightness_info(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Unknown";

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size > 256 * 1024) size = 256 * 1024;

        std::vector<char> buffer(size);
        file.read(buffer.data(), size);

        long total_brightness = 0;
        int pixel_count = 0;

        for (size_t i = 0; i < size - 2; i += 30) {
            if (i + 2 < size) {
                unsigned char r = (unsigned char)buffer[i];
                unsigned char g = (unsigned char)buffer[i + 1];
                unsigned char b = (unsigned char)buffer[i + 2];

                int brightness = (r * 299 + g * 587 + b * 114) / 1000;
                total_brightness += brightness;
                pixel_count++;
            }
        }

        if (pixel_count == 0) return "Unknown";

        int avg_brightness = total_brightness / pixel_count;
        std::string level;
        if (avg_brightness < 64) level = "Very Dark";
        else if (avg_brightness < 128) level = "Dark";
        else if (avg_brightness < 192) level = "Medium";
        else level = "Bright";

        return level + " (" + std::to_string(avg_brightness) + "/255)";
    }

    std::string get_file_permissions(const std::string& path) {
        struct stat st;
        if (stat(path.c_str(), &st) != 0) return "Unknown";
        
        std::ostringstream perms;
        perms << std::oct << (st.st_mode & 0777);
        return perms.str();
    }

    std::string format_size(long size) {
        double kb = size / 1024.0;
        double mb = kb / 1024.0;
        double gb = mb / 1024.0;
        std::ostringstream out;
        out << std::fixed << std::setprecision(2);
        if (gb >= 1.0) out << gb << " GB";
        else if (mb >= 1.0) out << mb << " MB";
        else if (kb >= 1.0) out << kb << " KB";
        else out << size << " B";
        return out.str();
    }

    std::string save_uploaded_file() {
        std::string method = getenv("REQUEST_METHOD") ? getenv("REQUEST_METHOD") : "";
        if (method != "POST") return "";

        char* len_str = getenv("CONTENT_LENGTH");
        if (!len_str) return "";
        int content_length = atoi(len_str);
        if (content_length <= 0) return "";

        std::string content_type = getenv("CONTENT_TYPE") ? getenv("CONTENT_TYPE") : "";
        size_t pos = content_type.find("boundary=");
        if (pos == std::string::npos) return "";
        std::string boundary = "--" + content_type.substr(pos + 9);

        std::string data(content_length, '\0');
        std::cin.read(&data[0], content_length);

        size_t file_start = data.find("\r\n\r\n");
        if (file_start == std::string::npos) return "";
        file_start += 4;

        size_t file_end = data.find(boundary, file_start);
        if (file_end == std::string::npos) return "";
        file_end -= 2; // skip \r\n before boundary

        // Extract filename
        size_t fn_start = data.find("filename=\"");
        if (fn_start == std::string::npos) return "";
        fn_start += 10;
        size_t fn_end = data.find("\"", fn_start);
        std::string filename = data.substr(fn_start, fn_end - fn_start);

        // Generate unique filename to avoid conflicts
        std::time_t now = std::time(0);
        std::ostringstream unique_name;
        unique_name << now << "_" << filename;
        
        std::string filepath = upload_dir + unique_name.str();
        std::ofstream out(filepath, std::ios::binary);
        out.write(&data[file_start], file_end - file_start);
        out.close();

        return filepath;
    }

    void generate_html() {
        std::cout << "Content-Type: text/html\r\n\r\n";
        std::cout << "<!DOCTYPE html><html><head><title>Image Processor</title>\n";
        std::cout << "<style>body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #fff; margin: 20px; }";
        std::cout << ".container { max-width: 900px; margin: auto; background: rgba(255,255,255,0.05); padding: 20px; border-radius: 10px; }";
        std::cout << "h1 { color: #ff5722; text-align: center; margin-bottom: 20px; }";
        std::cout << ".card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 8px; margin-bottom: 20px; }";
        std::cout << "table { width: 100%; border-collapse: collapse; margin-top: 15px; }";
        std::cout << "th,td { padding: 8px; border-bottom: 1px solid rgba(255,255,255,0.1); text-align: left; }";
        std::cout << "th { background: rgba(255,87,34,0.2); color: #ff5722; }";
        std::cout << ".image-preview { max-width: 150px; max-height: 150px; border-radius: 6px; }</style></head><body>";
        std::cout << "<div class='container'><h1>Image Processor</h1>";

        std::string filepath = save_uploaded_file();
        if (filepath.empty()) {
            std::cout << "<div class='card'><h2>Upload Image</h2>";
            std::cout << "<form method='POST' enctype='multipart/form-data'>";
            std::cout << "<input type='file' name='file'><br><br>";
            std::cout << "<button type='submit'>Upload</button></form></div>";
        } else {
            struct stat st;
            stat(filepath.c_str(), &st);
            ImageInfo img;
            img.name = filepath.substr(filepath.find_last_of("/") + 1);
            img.size = st.st_size;
            img.type = get_extension(img.name);
            img.mime_type = get_mime_type(img.type);
            img.dimensions = get_image_dimensions(filepath);
            img.aspect_ratio = get_aspect_ratio(filepath);
            img.bit_depth = get_bit_depth(filepath);
            img.compression = get_compression_info(filepath);
            img.file_signature = get_file_signature(filepath);
            img.checksum = calculate_checksum(filepath);
            img.file_permissions = get_file_permissions(filepath);
            
            char time_str[32];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
            img.modified = time_str;
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&st.st_ctime));
            img.created = time_str;
            
            img.color_info = analyze_colors(filepath);
            img.dominant_colors = get_dominant_colors(filepath);
            img.brightness_info = get_brightness_info(filepath);
            
            // Fix preview path - use web-accessible path
            std::string web_path = web_upload_dir + img.name;

            std::cout << "<div class='card'><h2>📸 Uploaded Image</h2>";
            std::cout << "<img src='" << web_path << "' class='image-preview' alt='Uploaded image'>";
            std::cout << "<p><strong>Preview:</strong> " << img.name << "</p></div>";

            std::cout << "<div class='card'><h2>📋 Complete Image Metadata</h2><table>";
            std::cout << "<tr><th>📁 Filename</th><td>" << img.name << "</td></tr>";
            std::cout << "<tr><th>🏷️ File Type</th><td>" << img.type.empty() ? "Unknown" : img.type << "</td></tr>";
            std::cout << "<tr><th>📄 MIME Type</th><td>" << img.mime_type << "</td></tr>";
            std::cout << "<tr><th>📏 File Size</th><td>" << format_size(img.size) << " (" << img.size << " bytes)</td></tr>";
            std::cout << "<tr><th>📐 Dimensions</th><td>" << img.dimensions << "</td></tr>";
            std::cout << "<tr><th>📊 Aspect Ratio</th><td>" << img.aspect_ratio << "</td></tr>";
            std::cout << "<tr><th>🎨 Bit Depth</th><td>" << img.bit_depth << "</td></tr>";
            std::cout << "<tr><th>🗜️ Compression</th><td>" << img.compression << "</td></tr>";
            std::cout << "<tr><th>🔍 File Signature</th><td><code>" << img.file_signature << "</code></td></tr>";
            std::cout << "<tr><th>🔐 Checksum</th><td><code>" << img.checksum << "</code></td></tr>";
            std::cout << "<tr><th>🔒 Permissions</th><td>" << img.file_permissions << "</td></tr>";
            std::cout << "<tr><th>📅 Created</th><td>" << img.created << "</td></tr>";
            std::cout << "<tr><th>📝 Modified</th><td>" << img.modified << "</td></tr>";
            std::cout << "</table></div>";

            std::cout << "<div class='card'><h2>🎨 Color Analysis</h2><table>";
            std::cout << "<tr><th>🔍 Color Analysis</th><td>" << img.color_info << "</td></tr>";
            std::cout << "<tr><th>🎯 Dominant Colors</th><td>" << img.dominant_colors << "</td></tr>";
            std::cout << "<tr><th>💡 Brightness</th><td>" << img.brightness_info << "</td></tr>";
            std::cout << "</table></div>";

            // Add cleanup option
            std::cout << "<div class='card'><h2>🗑️ File Management</h2>";
            std::cout << "<p><strong>Note:</strong> Uploaded files are stored temporarily. ";
            std::cout << "File location: <code>" << filepath << "</code></p>";
            std::cout << "<p><em>Files may be automatically cleaned up after some time.</em></p>";
            std::cout << "</div>";
        }

        std::cout << "</div></body></html>";
    }
};

int main() {
    ImageProcessor processor;
    processor.generate_html();
    return 0;
}

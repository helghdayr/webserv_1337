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

struct ImageInfo {
    std::string name;
    std::string type;
    long size;
    std::string dimensions;
    std::string modified;
    std::string color_info;
};

class ImageProcessor {
private:
    std::string upload_dir;

public:
    ImageProcessor() : upload_dir("uploads/") {
        mkdir(upload_dir.c_str(), 0755);
    }

    std::string get_extension(const std::string& filename) {
        size_t pos = filename.find_last_of('.');
        if (pos != std::string::npos) {
            return filename.substr(pos + 1);
        }
        return "";
    }

    std::string get_image_dimensions(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Unknown";

        char header[24];
        file.read(header, 24);

        std::string ext = get_extension(path);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == "jpg" || ext == "jpeg") {
            return get_jpeg_dimensions(file);
        } else if (ext == "png") {
            return get_png_dimensions(header);
        } else if (ext == "gif") {
            return get_gif_dimensions(header);
        }

        return "Unknown";
    }

    std::string get_jpeg_dimensions(std::ifstream& file) {
        file.seekg(0, std::ios::beg);
        char buffer[2];

        while (file.read(buffer, 2)) {
            if ((unsigned char)buffer[0] == 0xFF && (unsigned char)buffer[1] == 0xC0) {
                file.seekg(3, std::ios::cur);
                char dims[4];
                file.read(dims, 4);
                int height = ((unsigned char)dims[0] << 8) | (unsigned char)dims[1];
                int width = ((unsigned char)dims[2] << 8) | (unsigned char)dims[3];
                return std::to_string(width) + "x" + std::to_string(height);
            }
        }
        return "Unknown";
    }

    std::string get_png_dimensions(const char* header) {
        if (header[0] == (char)0x89 && header[1] == 'P' && header[2] == 'N' && header[3] == 'G') {
            int width = ((unsigned char)header[16] << 24) | ((unsigned char)header[17] << 16) |
                       ((unsigned char)header[18] << 8) | (unsigned char)header[19];
            int height = ((unsigned char)header[20] << 24) | ((unsigned char)header[21] << 16) |
                        ((unsigned char)header[22] << 8) | (unsigned char)header[23];
            return std::to_string(width) + "x" + std::to_string(height);
        }
        return "Unknown";
    }

    std::string get_gif_dimensions(const char* header) {
        if (header[0] == 'G' && header[1] == 'I' && header[2] == 'F') {
            int width = (unsigned char)header[6] | ((unsigned char)header[7] << 8);
            int height = (unsigned char)header[8] | ((unsigned char)header[9] << 8);
            return std::to_string(width) + "x" + std::to_string(height);
        }
        return "Unknown";
    }

    std::string analyze_colors(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "Unknown";

        file.seekg(0, std::ios::end);
        size_t size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(size);
        file.read(buffer.data(), size);

        int color_count = 0;
        std::map<std::string, int> color_freq;

        for (size_t i = 0; i < size - 2; i += 3) {
            if (i + 2 < size) {
                unsigned char r = (unsigned char)buffer[i];
                unsigned char g = (unsigned char)buffer[i + 1];
                unsigned char b = (unsigned char)buffer[i + 2];

                std::string color = std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b);
                color_freq[color]++;
                color_count++;

                if (color_count > 1000) break;
            }
        }

        if (color_count == 0) return "No color data";

        auto max_color = std::max_element(color_freq.begin(), color_freq.end(),
            [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
                return a.second < b.second;
            });

        return "Dominant: RGB(" + max_color->first + ")";
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

        std::string filepath = upload_dir + filename;
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
            img.dimensions = get_image_dimensions(filepath);
            char time_str[32];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&st.st_mtime));
            img.modified = time_str;
            img.color_info = analyze_colors(filepath);

            std::cout << "<div class='card'><h2>Uploaded Image</h2>";
            std::cout << "<img src='" << filepath << "' class='image-preview'></div>";

            std::cout << "<div class='card'><h2>Metadata</h2><table>";
            std::cout << "<tr><th>Name</th><td>" << img.name << "</td></tr>";
            std::cout << "<tr><th>Type</th><td>" << img.type << "</td></tr>";
            std::cout << "<tr><th>Size</th><td>" << format_size(img.size) << "</td></tr>";
            std::cout << "<tr><th>Dimensions</th><td>" << img.dimensions << "</td></tr>";
            std::cout << "<tr><th>Modified</th><td>" << img.modified << "</td></tr>";
            std::cout << "<tr><th>Color Info</th><td>" << img.color_info << "</td></tr>";
            std::cout << "</table></div>";
        }

        std::cout << "</div></body></html>";
    }
};

int main() {
    ImageProcessor processor;
    processor.generate_html();
    return 0;
}

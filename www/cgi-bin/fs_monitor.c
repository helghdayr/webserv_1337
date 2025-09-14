#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define MAX_PATH 1024
#define MAX_FILES 1000

typedef struct {
    char name[256];
    long size;
    char type[16];
    char modified[32];
} file_info_t;

void html_escape(const char* input, char* output, size_t max_len) {
    size_t i, j = 0;
    for (i = 0; input[i] && j < max_len - 1; i++) {
        switch (input[i]) {
            case '&': strcpy(output + j, "&amp;"); j += 5; break;
            case '<': strcpy(output + j, "&lt;"); j += 4; break;
            case '>': strcpy(output + j, "&gt;"); j += 4; break;
            case '"': strcpy(output + j, "&quot;"); j += 6; break;
            default: output[j++] = input[i]; break;
        }
    }
    output[j] = '\0';
}

void format_size(long bytes, char* buffer, size_t buf_size) {
    if (bytes < 1024) {
        snprintf(buffer, buf_size, "%ld B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(buffer, buf_size, "%.2f KB", bytes / 1024.0);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(buffer, buf_size, "%.2f MB", bytes / (1024.0 * 1024.0));
    } else {
        snprintf(buffer, buf_size, "%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0));
    }
}

long get_directory_size(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) return 0;
    
    long total_size = 0;
    struct dirent* entry;
    struct stat statbuf;
    char full_path[MAX_PATH];
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
            
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &statbuf) == 0) {
            if (S_ISREG(statbuf.st_mode)) {
                total_size += statbuf.st_size;
            } else if (S_ISDIR(statbuf.st_mode)) {
                total_size += get_directory_size(full_path);
            }
        }
    }
    closedir(dir);
    return total_size;
}

int scan_directory(const char* path, file_info_t* files, int max_files) {
    DIR* dir = opendir(path);
    if (!dir) return 0;
    
    int count = 0;
    struct dirent* entry;
    struct stat statbuf;
    char full_path[MAX_PATH];
    struct tm* tm_info;
    
    while ((entry = readdir(dir)) != NULL && count < max_files) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
            
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &statbuf) == 0) {
            strncpy(files[count].name, entry->d_name, sizeof(files[count].name) - 1);
            files[count].name[sizeof(files[count].name) - 1] = '\0';
            
            files[count].size = statbuf.st_size;
            
            if (S_ISREG(statbuf.st_mode)) {
                strcpy(files[count].type, "File");
            } else if (S_ISDIR(statbuf.st_mode)) {
                strcpy(files[count].type, "Directory");
            } else {
                strcpy(files[count].type, "Other");
            }
            
            tm_info = localtime(&statbuf.st_mtime);
            strftime(files[count].modified, sizeof(files[count].modified), "%Y-%m-%d %H:%M:%S", tm_info);
            
            count++;
        }
    }
    closedir(dir);
    return count;
}

char* get_posted_data() {
    char* method = getenv("REQUEST_METHOD");
    if (!method || strcmp(method, "POST") != 0) return NULL;
    
    char* content_length_str = getenv("CONTENT_LENGTH");
    if (!content_length_str) return NULL;
    
    int content_length = atoi(content_length_str);
    if (content_length <= 0) return NULL;
    
    char* data = malloc(content_length + 1);
    if (!data) return NULL;
    
    fread(data, 1, content_length, stdin);
    data[content_length] = '\0';
    
    return data;
}

void parse_form_data(const char* data, char* path, char* action) {
    char* token = strtok((char*)data, "&");
    while (token) {
        if (strncmp(token, "path=", 5) == 0) {
            strncpy(path, token + 5, MAX_PATH - 1);
            path[MAX_PATH - 1] = '\0';
        } else if (strncmp(token, "action=", 7) == 0) {
            strncpy(action, token + 7, 16);
            action[15] = '\0';
        }
        token = strtok(NULL, "&");
    }
}

void url_decode(char* str) {
    char* src = str;
    char* dst = str;
    
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            int value;
            sscanf(src + 1, "%2x", &value);
            *dst++ = (char)value;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

int main() {
    char* posted_data = get_posted_data();
    char scan_path[MAX_PATH] = ".";
    char action[16] = "scan";
    char escaped_path[MAX_PATH * 2];
    
    if (posted_data) {
        parse_form_data(posted_data, scan_path, action);
        url_decode(scan_path);
        free(posted_data);
    }
    
    printf("Content-Type: text/html\r\n\r\n");
    printf("<!DOCTYPE html>\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>File System Monitor - C CGI</title>\n");
    printf("<style>\n");
    printf("body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #ffffff; margin: 20px; }\n");
    printf(".container { max-width: 1200px; margin: 0 auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }\n");
    printf("h1 { color: #ff5722; text-align: center; margin-bottom: 30px; }\n");
    printf(".section { margin-bottom: 30px; }\n");
    printf(".section h2 { color: #ff5722; margin-bottom: 15px; }\n");
    printf(".grid { display: grid; grid-template-columns: 30%% 70%%; gap: 20px; }\n");
    printf(".card { background: rgba(255,255,255,0.1); padding: 20px; border-radius: 10px; border-left: 4px solid #ff5722; }\n");
    printf(".stat-item { margin-bottom: 10px; padding: 8px; background: rgba(255,255,255,0.05); border-radius: 6px; }\n");
    printf(".stat-number { font-size: 1.1em; color: #ff5722; font-weight: bold; }\n");
    printf(".stat-label { color: #cccccc; font-size: 0.85em; margin-top: 3px; }\n");
    printf(".table-container { overflow-x: auto; }\n");
    printf(".table { width: 100%%; border-collapse: collapse; table-layout: fixed; }\n");
    printf(".table th, .table td { padding: 8px; text-align: left; border-bottom: 1px solid rgba(255,255,255,0.1); word-wrap: break-word; white-space: normal; }\n");
    printf(".table th { background: rgba(255, 87, 34, 0.2); color: #ff5722; font-weight: bold; }\n");
    printf(".table tr:hover { background: rgba(255,255,255,0.05); }\n");
    printf(".back-btn { background: #666; color: white; padding: 8px 16px; text-decoration: none; border-radius: 5px; display: inline-block; margin-top: 20px; }\n");
    printf("input[type='text'], select { width: 100%%; background: rgba(255,255,255,0.05); color: #fff; border: 1px solid rgba(255,255,255,0.1); border-radius: 6px; padding: 8px; }\n");
    printf("button { background: #ff5722; color: white; padding: 8px 16px; border: none; border-radius: 5px; cursor: pointer; }\n");
    printf("</style>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<div class='container'>\n");
    printf("<h1>File System Monitor</h1>\n");
    
    html_escape(scan_path, escaped_path, sizeof(escaped_path));
    
    printf("<div class='section'>\n");
    printf("<h2>Directory Scanner</h2>\n");
    printf("<div class='grid'>\n");
    
    // --- Scan Directory (30%) ---
    printf("<div class='card'>\n");
    printf("<h3 style='color: #ff5722; margin-bottom: 15px;'>Scan Directory</h3>\n");
    printf("<form method='POST'>\n");
    printf("<input type='text' name='path' value='%s' placeholder='Enter directory path'><br/><br/>\n", escaped_path);
    printf("<select name='action'>\n");
    printf("<option value='scan' %s>Scan Files</option>\n", strcmp(action, "scan") == 0 ? "selected" : "");
    printf("<option value='size' %s>Calculate Size</option>\n", strcmp(action, "size") == 0 ? "selected" : "");
    printf("</select><br/><br/>\n");
    printf("<button type='submit'>Execute</button>\n");
    printf("</form>\n");
    printf("</div>\n");
    
    // --- Files Section (70%) ---
    struct stat statbuf;
    if (stat(scan_path, &statbuf) == 0) {
        if (S_ISDIR(statbuf.st_mode)) {
            file_info_t files[MAX_FILES];
            int file_count = scan_directory(scan_path, files, MAX_FILES);
            long dir_size = 0;
            char size_buf[64];
            
            if (strcmp(action, "size") == 0) {
                dir_size = get_directory_size(scan_path);
                format_size(dir_size, size_buf, sizeof(size_buf));
            }
            
            printf("<div class='card'>\n");
            printf("<h3 style='color: #ff5722; margin-bottom: 15px;'>Files (%d found)</h3>\n", file_count);
            printf("<div class='stat-item'><div class='stat-number'>%s</div><div class='stat-label'>Path</div></div>\n", escaped_path);
            if (strcmp(action, "size") == 0) {
                printf("<div class='stat-item'><div class='stat-number'>%s</div><div class='stat-label'>Total Size</div></div>\n", size_buf);
            }
            
            if (file_count > 0) {
                printf("<div class='table-container'>\n");
                printf("<table class='table'>\n");
                printf("<tr><th>Name</th><th>Type</th><th>Size</th><th>Modified</th></tr>\n");
                
                for (int i = 0; i < file_count && i < 50; i++) {
                    char escaped_name[512];
                    char file_size_buf[64];
                    html_escape(files[i].name, escaped_name, sizeof(escaped_name));
                    format_size(files[i].size, file_size_buf, sizeof(file_size_buf));
                    printf("<tr><td>%s</td><td>%s</td><td>%s</td><td>%s</td></tr>\n", 
                           escaped_name, files[i].type, file_size_buf, files[i].modified);
                }
                if (file_count > 50) {
                    printf("<tr><td colspan='4' style='text-align: center; color: #ff5722;'>... and %d more files</td></tr>\n", file_count - 50);
                }
                printf("</table>\n");
                printf("</div>\n");
            }
            printf("</div>\n");
        } else {
            char size_buf[64];
            format_size(statbuf.st_size, size_buf, sizeof(size_buf));
            printf("<div class='card'>\n");
            printf("<h3 style='color: #ff5722; margin-bottom: 15px;'>File Information</h3>\n");
            printf("<div class='stat-item'><div class='stat-number'>%s</div><div class='stat-label'>File Path</div></div>\n", escaped_path);
            printf("<div class='stat-item'><div class='stat-number'>%s</div><div class='stat-label'>Size</div></div>\n", size_buf);
            printf("</div>\n");
        }
    } else {
        printf("<div class='card'>\n");
        printf("<h3 style='color: #ff5722; margin-bottom: 15px;'>Error</h3>\n");
        printf("<p style='color: #ff5722;'>Cannot access path: %s</p>\n", escaped_path);
        printf("</div>\n");
    }
    
    printf("</div>\n"); // end grid
    printf("</div>\n"); // end section
    
    printf("<a href='/cgi-bin/cgi_test.html' class='back-btn'>Back to CGI Testing</a>\n");
    printf("</div>\n");
    printf("</body>\n");
    printf("</html>\n");
    
    return 0;
}

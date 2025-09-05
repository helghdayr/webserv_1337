#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <cstdlib>
#include <cstring>
#include <cmath>

class Calculator {
private:
    std::map<std::string, std::string> params;
    std::string request_method;

    void parseQueryString(const std::string& query) {
        std::istringstream iss(query);
        std::string pair;
        
        while (std::getline(iss, pair, '&')) {
            size_t pos = pair.find('=');
            if (pos != std::string::npos) {
                std::string key = pair.substr(0, pos);
                std::string value = pair.substr(pos + 1);
                params[key] = urlDecode(value);
            }
        }
    }

    void parsePostData() {
        char* content_length_str = getenv("CONTENT_LENGTH");
        if (!content_length_str) return;
        
        int content_length = atoi(content_length_str);
        if (content_length <= 0 || content_length > 10000) return;
        
        std::string post_data(content_length, '\0');
        std::cin.read(&post_data[0], content_length);
        
        parseQueryString(post_data);
    }

    std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '+') {
                result += ' ';
            } else if (str[i] == '%' && i + 2 < str.length()) {
                int hex_value;
                std::istringstream hex_stream(str.substr(i + 1, 2));
                if (hex_stream >> std::hex >> hex_value) {
                    result += static_cast<char>(hex_value);
                    i += 2;
                } else {
                    result += str[i];
                }
            } else {
                result += str[i];
            }
        }
        return result;
    }

    double calculate(const std::string& operation, double a, double b) {
        if (operation == "add") return a + b;
        if (operation == "subtract") return a - b;
        if (operation == "multiply") return a * b;
        if (operation == "divide") return (b != 0) ? a / b : 0;
        if (operation == "power") return pow(a, b);
        if (operation == "modulo") return (b != 0) ? fmod(a, b) : 0;
        return 0;
    }

    std::string getOperationSymbol(const std::string& operation) {
        if (operation == "add") return "+";
        if (operation == "subtract") return "-";
        if (operation == "multiply") return "×";
        if (operation == "divide") return "÷";
        if (operation == "power") return "^";
        if (operation == "modulo") return "%";
        return "?";
    }

public:
    Calculator() {
        request_method = getenv("REQUEST_METHOD") ? getenv("REQUEST_METHOD") : "GET";
        
        if (request_method == "GET") {
            char* query_string = getenv("QUERY_STRING");
            if (query_string) {
                parseQueryString(query_string);
            }
        } else if (request_method == "POST") {
            parsePostData();
        }
    }

    void generateHTML() {
        std::cout << "Content-Type: text/html\r\n\r\n";
        std::cout << "<!DOCTYPE html>\n<html>\n<head>\n";
        std::cout << "<title>Calculator - GET/POST Demo</title>\n";
        std::cout << "<style>\n";
        std::cout << "body { font-family: 'Segoe UI', sans-serif; background: #1a1a1a; color: #fff; margin: 20px; }\n";
        std::cout << ".container { max-width: 600px; margin: auto; background: rgba(255,255,255,0.05); padding: 30px; border-radius: 15px; }\n";
        std::cout << "h1 { color: #4CAF50; text-align: center; margin-bottom: 30px; }\n";
        std::cout << ".method-info { background: rgba(76,175,80,0.1); padding: 15px; border-radius: 8px; margin-bottom: 20px; border-left: 4px solid #4CAF50; }\n";
        std::cout << ".form-group { margin-bottom: 20px; }\n";
        std::cout << "label { display: block; margin-bottom: 5px; color: #4CAF50; font-weight: bold; }\n";
        std::cout << "input, select { width: 100%; padding: 12px; border: 1px solid #333; background: #2a2a2a; color: #fff; border-radius: 6px; box-sizing: border-box; }\n";
        std::cout << "button { background: #4CAF50; color: white; padding: 12px 24px; border: none; border-radius: 6px; cursor: pointer; margin: 5px; }\n";
        std::cout << "button:hover { background: #45a049; }\n";
        std::cout << ".result { background: rgba(76,175,80,0.2); padding: 20px; border-radius: 8px; margin-top: 20px; text-align: center; }\n";
        std::cout << ".calculation { font-size: 1.2em; margin-bottom: 10px; }\n";
        std::cout << ".answer { font-size: 2em; font-weight: bold; color: #4CAF50; }\n";
        std::cout << ".forms { display: flex; gap: 20px; flex-wrap: wrap; }\n";
        std::cout << ".form-section { flex: 1; min-width: 250px; background: rgba(255,255,255,0.02); padding: 20px; border-radius: 10px; }\n";
        std::cout << "h3 { color: #4CAF50; margin-top: 0; }\n";
        std::cout << "</style>\n</head>\n<body>\n";
        
        std::cout << "<div class='container'>\n";
        std::cout << "<h1>🧮 Calculator - GET/POST Demo</h1>\n";
        
        // Show current request method
        std::cout << "<div class='method-info'>\n";
        std::cout << "<strong>Current Request Method:</strong> " << request_method << "\n";
        if (!params.empty()) {
            std::cout << "<br><strong>Parameters received:</strong> ";
            for (const auto& param : params) {
                std::cout << param.first << "=" << param.second << " ";
            }
        }
        std::cout << "</div>\n";

        // Show calculation result if parameters exist
        if (params.find("num1") != params.end() && 
            params.find("num2") != params.end() && 
            params.find("operation") != params.end()) {
            
            double num1 = atof(params["num1"].c_str());
            double num2 = atof(params["num2"].c_str());
            std::string operation = params["operation"];
            double result = calculate(operation, num1, num2);
            
            std::cout << "<div class='result'>\n";
            std::cout << "<div class='calculation'>" << num1 << " " << getOperationSymbol(operation) << " " << num2 << " =</div>\n";
            std::cout << "<div class='answer'>" << result << "</div>\n";
            std::cout << "<p><em>Calculated using " << request_method << " method</em></p>\n";
            std::cout << "</div>\n";
        }

        // Forms section
        std::cout << "<div class='forms'>\n";
        
        // GET form
        std::cout << "<div class='form-section'>\n";
        std::cout << "<h3>📤 GET Method</h3>\n";
        std::cout << "<p>Parameters visible in URL</p>\n";
        std::cout << "<form method='GET'>\n";
        std::cout << "<div class='form-group'>\n";
        std::cout << "<label>First Number:</label>\n";
        std::cout << "<input type='number' name='num1' step='any' value='" << (params.find("num1") != params.end() ? params["num1"] : "") << "' required>\n";
        std::cout << "</div>\n";
        std::cout << "<div class='form-group'>\n";
        std::cout << "<label>Operation:</label>\n";
        std::cout << "<select name='operation'>\n";
        std::string current_op = params.find("operation") != params.end() ? params["operation"] : "";
        std::cout << "<option value='add'" << (current_op == "add" ? " selected" : "") << ">Addition (+)</option>\n";
        std::cout << "<option value='subtract'" << (current_op == "subtract" ? " selected" : "") << ">Subtraction (-)</option>\n";
        std::cout << "<option value='multiply'" << (current_op == "multiply" ? " selected" : "") << ">Multiplication (×)</option>\n";
        std::cout << "<option value='divide'" << (current_op == "divide" ? " selected" : "") << ">Division (÷)</option>\n";
        std::cout << "<option value='power'" << (current_op == "power" ? " selected" : "") << ">Power (^)</option>\n";
        std::cout << "<option value='modulo'" << (current_op == "modulo" ? " selected" : "") << ">Modulo (%)</option>\n";
        std::cout << "</select>\n";
        std::cout << "</div>\n";
        std::cout << "<div class='form-group'>\n";
        std::cout << "<label>Second Number:</label>\n";
        std::cout << "<input type='number' name='num2' step='any' value='" << (params.find("num2") != params.end() ? params["num2"] : "") << "' required>\n";
        std::cout << "</div>\n";
        std::cout << "<button type='submit'>Calculate with GET</button>\n";
        std::cout << "</form>\n";
        std::cout << "</div>\n";
        
        // POST form
        std::cout << "<div class='form-section'>\n";
        std::cout << "<h3>📥 POST Method</h3>\n";
        std::cout << "<p>Parameters hidden in request body</p>\n";
        std::cout << "<form method='POST'>\n";
        std::cout << "<div class='form-group'>\n";
        std::cout << "<label>First Number:</label>\n";
        std::cout << "<input type='number' name='num1' step='any' value='" << (params.find("num1") != params.end() ? params["num1"] : "") << "' required>\n";
        std::cout << "</div>\n";
        std::cout << "<div class='form-group'>\n";
        std::cout << "<label>Operation:</label>\n";
        std::cout << "<select name='operation'>\n";
        std::cout << "<option value='add'" << (current_op == "add" ? " selected" : "") << ">Addition (+)</option>\n";
        std::cout << "<option value='subtract'" << (current_op == "subtract" ? " selected" : "") << ">Subtraction (-)</option>\n";
        std::cout << "<option value='multiply'" << (current_op == "multiply" ? " selected" : "") << ">Multiplication (×)</option>\n";
        std::cout << "<option value='divide'" << (current_op == "divide" ? " selected" : "") << ">Division (÷)</option>\n";
        std::cout << "<option value='power'" << (current_op == "power" ? " selected" : "") << ">Power (^)</option>\n";
        std::cout << "<option value='modulo'" << (current_op == "modulo" ? " selected" : "") << ">Modulo (%)</option>\n";
        std::cout << "</select>\n";
        std::cout << "</div>\n";
        std::cout << "<div class='form-group'>\n";
        std::cout << "<label>Second Number:</label>\n";
        std::cout << "<input type='number' name='num2' step='any' value='" << (params.find("num2") != params.end() ? params["num2"] : "") << "' required>\n";
        std::cout << "</div>\n";
        std::cout << "<button type='submit'>Calculate with POST</button>\n";
        std::cout << "</form>\n";
        std::cout << "</div>\n";
        
        std::cout << "</div>\n"; // end forms
        
        // Debug info
        std::cout << "<div style='margin-top: 30px; padding: 15px; background: rgba(255,255,255,0.02); border-radius: 8px; font-size: 0.9em;'>\n";
        std::cout << "<h4>🔧 Debug Information</h4>\n";
        std::cout << "<p><strong>Request Method:</strong> " << request_method << "</p>\n";
        
        char* query_string = getenv("QUERY_STRING");
        if (query_string && strlen(query_string) > 0) {
            std::cout << "<p><strong>Query String:</strong> " << query_string << "</p>\n";
        }
        
        char* content_type = getenv("CONTENT_TYPE");
        if (content_type) {
            std::cout << "<p><strong>Content Type:</strong> " << content_type << "</p>\n";
        }
        
        char* content_length = getenv("CONTENT_LENGTH");
        if (content_length) {
            std::cout << "<p><strong>Content Length:</strong> " << content_length << "</p>\n";
        }
        
        std::cout << "</div>\n";
        
        std::cout << "</div>\n</body>\n</html>\n";
    }
};

int main() {
    Calculator calc;
    calc.generateHTML();
    return 0;
}

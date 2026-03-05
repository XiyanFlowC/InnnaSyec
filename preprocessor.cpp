#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include <ctime>
#include <iomanip>

using namespace std;

map<string, string> macros;  // 宏定义
map<string, string> symbols; // 符号定义
vector<string> output;                // 输出结果
bool prev_is_branch = false;  // 标记上条指令是否为分支指令
string last_branch_cmd;       // 记录分支指令名（用于报错）

// 编译时间（程序启动时记录）
time_t compile_time = time(nullptr);

void process_input(istream& input, const string &filename);
void process_line(istream& input, string &line, const string &filename, int &line_no);

// 解析并计算表达式（支持简单符号和数字）
int eval_expr(const string& expr) {
    if (expr.empty()) return 0;

    try {
        return stoi(expr, nullptr, 0);  // 支持十进制、十六进制等
    } catch (const invalid_argument&) {
        if (symbols.find(expr) != symbols.end()) {
            return stoi(symbols[expr], nullptr, 0);
        }
        throw runtime_error("Unrecognized symbol: " + expr);
    }
}

// 解析逗号分隔的参数
vector<string> parse_parameters(const string& param_str) {
    vector<string> params;
    string current;
    int paren_count = 0; // 括号计数器

    for (char c : param_str) {
        if (c == '(') paren_count++;
        else if (c == ')') {
            if (paren_count > 0) paren_count--;
        }

        // 仅当括号计数为0时，逗号才作为分隔符
        if (c == ',' && paren_count == 0) {
            if (!current.empty()) {
                params.push_back(current);
                current.clear();
            }
            continue; // 跳过逗号
        }

        current += c; // 累积字符
    }

    if (!current.empty()) {
        params.push_back(current);
    }
    return params;
}

// 处理魔法宏替换
string replace_magic_macros(const string& text, const string& current_filename, int current_line) {
    string result = text;
    
    // __LINE__ 宏 - 当前行号
    size_t pos = 0;
    while ((pos = result.find("__LINE__", pos)) != string::npos) {
        result.replace(pos, 8, to_string(current_line));
        pos += to_string(current_line).length();
    }
    
    // __FILE__ 宏 - 当前文件名（带引号）
    pos = 0;
    string quoted_filename = "\"" + current_filename + "\"";
    while ((pos = result.find("__FILE__", pos)) != string::npos) {
        result.replace(pos, 8, quoted_filename);
        pos += quoted_filename.length();
    }
    
    // __DATE__ 宏 - 编译日期（格式：Oct 14 2025）
    pos = 0;
    struct tm* compile_tm = localtime(&compile_time);
    char date_buffer[12];
    strftime(date_buffer, sizeof(date_buffer), "%b %d %Y", compile_tm);
    string quoted_date = "\"" + string(date_buffer) + "\"";
    while ((pos = result.find("__DATE__", pos)) != string::npos) {
        result.replace(pos, 8, quoted_date);
        pos += quoted_date.length();
    }
    
    // __TIME__ 宏 - 编译时间（格式：HH:MM:SS）
    pos = 0;
    char time_buffer[9];
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", compile_tm);
    string quoted_time = "\"" + string(time_buffer) + "\"";
    while ((pos = result.find("__TIME__", pos)) != string::npos) {
        result.replace(pos, 8, quoted_time);
        pos += quoted_time.length();
    }

    // __DATETIME__ 宏 - 编译日期时间（格式：YYYY-MM-DD HH:MM:SS）
    pos = 0;
    char datetime_buffer[20];
    strftime(datetime_buffer, sizeof(datetime_buffer), "%Y-%m-%d %H:%M:%S", compile_tm);
    string quoted_datetime = "\"" + string(datetime_buffer) + "\"";
    while ((pos = result.find("__DATETIME__", pos)) != string::npos) {
        result.replace(pos, 12, quoted_datetime);
        pos += quoted_datetime.length();
    }
    
    return result;
}

string replace_symbols_in_expr(const string& expr) {
    string result;
    size_t pos = 0;
    while (pos < expr.size()) {
        // 识别标识符（包含字母、数字、下划线、$、@）
        if (isalpha(expr[pos]) || expr[pos] == '_' || expr[pos] == '$' || expr[pos] == '@') {
            size_t start = pos;
            // 修正：标识符应包含所有[\w_$@]字符
            while (pos < expr.size() && (isalnum(expr[pos]) || expr[pos] == '_' || expr[pos] == '$' || expr[pos] == '@')) {
                pos++;
            }
            string ident = expr.substr(start, pos - start);
            
            // 查找符号替换，但要避免循环替换
            if (symbols.find(ident) != symbols.end()) {
                string replacement = symbols[ident];
                // 防止循环替换：如果替换结果和原始标识符相同，则不替换
                if (replacement != ident) {
                    result += replacement;
                } else {
                    result += ident;
                }
            } else {
                result += ident;
            }
        } else {
            result += expr[pos++]; // 非标识符字符直接保留
        }
    }
    return result;
}

// 查找不在字符串字面量内的字符位置
size_t find_outside_strings(const string& line, char target) {
    bool in_string = false;
    char quote_char = '\0';
    
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        
        if (!in_string) {
            if (c == '"' || c == '\'') {
                in_string = true;
                quote_char = c;
            } else if (c == target) {
                return i;
            }
        } else {
            if (c == quote_char) {
                // 检查是否为转义字符
                if (i == 0 || line[i-1] != '\\') {
                    in_string = false;
                    quote_char = '\0';
                }
            }
        }
    }
    
    return string::npos;
}

// 解析指令行
void parse_instruction(const string& line, const string &filename, int &line_no) {
    string label, cmd, params, comment;
    
    // 首先应用魔法宏替换到整行
    string processed_line = replace_magic_macros(line, filename, line_no + 1);

    // 提取label和指令主体，使用新函数查找不在字符串内的冒号
    size_t colon_pos = find_outside_strings(processed_line, ':');
    size_t semicolon_pos = processed_line.find(';');
    size_t harsh_pos = processed_line.find('#');
    if (harsh_pos < semicolon_pos) semicolon_pos = harsh_pos;

    if (colon_pos != string::npos) {
        label = processed_line.substr(0, colon_pos);
        params = processed_line.substr(colon_pos + 1, semicolon_pos - colon_pos - 1);
    } else {
        params = processed_line.substr(0, semicolon_pos);
    }

    // 去掉注释部分
    if (semicolon_pos != string::npos) {
        comment = processed_line.substr(semicolon_pos);
    }

    // 分离cmd和参数
    stringstream ss(params);
    ss >> cmd;
    // 不要处理 .指令
    if (cmd[0] == '.') {
        cout << processed_line << endl;
        return;
    }
    int spaceSize = params.find_first_not_of(" \t");
    spaceSize = spaceSize < 0 ? 0 : spaceSize;
    string args = params.substr(cmd.size() + spaceSize);

    // 解析参数
    vector<string> args_list = parse_parameters(args);
    
    // 替换参数中的符号
    for (auto& arg : args_list) {
        arg = replace_symbols_in_expr(arg);
    }
    // 展开宏
    if (macros.find(cmd + " " + to_string(args_list.size())) != macros.end()) {

        string macro_body = macros[cmd + " " + to_string(args_list.size())];
        

        for (size_t i = 0; i < args_list.size(); ++i) {
            string param_placeholder = "%" + to_string(i + 1);
            size_t pos = 0;
            while ((pos = macro_body.find(param_placeholder, pos)) != string::npos) {
                macro_body.replace(pos, param_placeholder.size(), args_list[i]);
                pos += args_list[i].size();
            }
        }
        //output.push_back((label.empty() ? "" : label + ": ") + macro_body);
        cout << (label.empty() ? "" : label + ":") << macro_body;
        // 将 cmd 更新为最后一个实在的指令
        cmd = macro_body.substr(0, macro_body.find_first_of(" \t"));
        cout << ".file " << filename << " " << line_no + 1 << endl;
    } else {
        string expanded_args;
        for (const auto& arg : args_list) {
            expanded_args += arg + ", ";
        }
        if (!expanded_args.empty()) {
            expanded_args.pop_back();
            expanded_args.pop_back();
        }

        // output.push_back((label.empty() ? "" : label + ": ") + cmd + " " + expanded_args);
        cout << (label.empty() ? "" : label + ": ") << cmd << " " << expanded_args << endl;
    }

    vector<string> branch_ops = {"j", "jal", "jr", "bne", "beq", "bgtz", "blez", "bltz", "bgez", "bltzal", "bgezal"}; // 常见分支指令
    if (find(branch_ops.begin(), branch_ops.end(), cmd) != branch_ops.end()) {
        prev_is_branch = true;      // 标记当前为分支指令
        last_branch_cmd = cmd;      // 记录指令名
    } else {
        prev_is_branch = false;     // 非分支指令则重置状态
    }
}

// 处理预处理指令
void handle_directive(istream& input, const string& line, const string &filename, int &line_no) {
    // 首先应用魔法宏替换
    string processed_line = replace_magic_macros(line, filename, line_no + 1);
    
    stringstream ss(processed_line);
    string directive;
    ss >> directive;

    if (directive == "define") {
        string symbol, value;
        ss >> symbol >> value;
        symbols[symbol] = value;
        cout << ".file " << filename << " " << line_no + 1 << endl;
    } else if (directive == "macro") {
        string macro_name;
        int macro_argc;
        ss >> macro_name >> macro_argc;
        stringstream macro_body;
        string line;
        while (getline(input, line) && line != "%endmacro") {
            macro_body << line << endl;
            line_no += 1;
        }
        line_no += 1;
        cout << ".file " << filename << " " << line_no + 1 << endl;
        macros[macro_name + " " + to_string(macro_argc)] = macro_body.str();
    } else if (directive == "ifdef" || directive == "ifndef" || directive == "if") {
        string expr;
        ss >> expr;
        bool condition = directive == "if" ? eval_expr(expr) : (symbols.find(expr) != symbols.end());
        if (directive == "ifndef") condition = !condition;
        // cout << "; if " << expr << " eval to " << condition << endl;

        int depth = 1;
        string line;
        bool hasElse = false;
        bool hinted = false;
        while (depth > 0 && getline(input, line)) {
            line_no += 1;
            if (condition) {
                // condition 内，一定为最顶层
                if (!hinted) {
                    hinted = true;
                    cout << ".file " << filename << " " << line_no + 1 << endl;
                }
                if (line.find("%else") != string::npos) {
                    if (hasElse) throw runtime_error("Multiple else in if body!!!");
                    //cout << "; " << expr << " now else." << endl;
                    hasElse = true;
                    condition = false;
                    continue;
                }
                else if (line.find("%endif") != string::npos) {
                    break;
                }
                else process_line(input, line, filename, line_no);
            } else {
                // condition 不真
                if (line.find("%else") != string::npos && depth == 1) {
                    if (hasElse) throw runtime_error("Multiple else in if body!!!");
                    hasElse = true;
                    condition = true;
                    continue;
                }
                else if (line.find("%ifdef") != string::npos) ++depth;
                else if (line.find("%ifndef") != string::npos) ++depth;
                else if (line.find("%endif") != string::npos) --depth;
            }
        }
        // cout << "; if end " << expr << endl;
    } else if (directive == "include") {
        string incfilename;
        ss >> incfilename;
        ifstream file(incfilename);
        if (!file.is_open()) throw runtime_error("Unable to open file " + incfilename);
        // cout << "; File " << filename << endl;
        process_input(file, incfilename);
        cout << ".file " << filename << " " << line_no + 1 << endl;
    } else {
        throw runtime_error("Unknown directive: " + directive);
    }
}

void process_line(istream& input, string &line, const string &filename, int &line_no) {
    if (line.empty()) {
        cout << endl;
        return;
    }

    if (line[0] == '%') {
        handle_directive(input, line.substr(1), filename, line_no);  // 处理预处理指令
    } else {
        parse_instruction(line, filename, line_no);  // 处理普通指令行
    }
}

// 主流程
void process_input(istream& input, const string &filename) {
    string line;
    int line_no = 1;
    cout << ".file " << filename << " 1" << endl;
    while (getline(input, line)) {
        process_line(input, line, filename, line_no);
        line_no += 1;
    }
}

int main(int argc, char* argv[]) {
    try {
        if (argc > 1) {
            ifstream file(argv[1]);
            //cout << ".file " << argv[1] << " 1" << endl;
            if (!file.is_open()) throw runtime_error("Unable to open file");
            process_input(file, argv[1]);
        } else {
            process_input(cin, "stdin");
        }
    } catch (const runtime_error& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}

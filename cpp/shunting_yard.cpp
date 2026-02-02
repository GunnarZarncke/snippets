#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <algorithm>

int precedence(const std::string& op) {
    if (op == "+" || op == "-") {
        return 1;
    }
    if (op == "*" || op == "/") {
        return 2;
    }
    return 0;
}

std::vector<std::string> shunting_yard(const std::vector<std::string>& tokens) {
    std::vector<std::string> output;
    std::stack<std::string> operators;
    
    for (const auto& token : tokens) {
        if (std::all_of(token.begin(), token.end(), ::isdigit)) {
            output.push_back(token);
        } else if (token == "(") {
            operators.push(token);
        } else if (token == ")") {
            while (!operators.empty() && operators.top() != "(") {
                output.push_back(operators.top());
                operators.pop();
            }
            operators.pop(); // Remove the '('
        } else if (token == "+" || token == "-" || token == "*" || token == "/") {
            while (!operators.empty() && operators.top() != "(" &&
                   precedence(operators.top()) >= precedence(token)) {
                output.push_back(operators.top());
                operators.pop();
            }
            operators.push(token);
        }
    }
    
    while (!operators.empty()) {
        output.push_back(operators.top());
        operators.pop();
    }
    
    return output;
}

int evaluate_rpn(const std::vector<std::string>& tokens) {
    std::stack<int> stack;
    
    for (const auto& token : tokens) {
        if (std::all_of(token.begin(), token.end(), ::isdigit)) {
            stack.push(std::stoi(token));
        } else {
            int b = stack.top();
            stack.pop();
            int a = stack.top();
            stack.pop();
            
            if (token == "+") {
                stack.push(a + b);
            } else if (token == "-") {
                stack.push(a - b);
            } else if (token == "*") {
                stack.push(a * b);
            } else if (token == "/") {
                stack.push(a / b);
            }
        }
    }
    
    return stack.top();
}

std::vector<std::string> tokenize(const std::string& expression) {
    std::vector<std::string> tokens;
    std::string current;
    
    for (char c : expression) {
        if (c == ' ') {
            continue;
        }
        
        if (std::isdigit(c)) {
            current += c;
        } else {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(std::string(1, c));
        }
    }
    
    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& tokens, const std::string& delimiter = " ") {
    std::ostringstream oss;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) {
            oss << delimiter;
        }
        oss << tokens[i];
    }
    return oss.str();
}

int main() {
    std::vector<std::string> expressions = {
        "3 + 4 * 2",
        "(3 + 4) * 2",
        "10 - 2 * 3",
        "2 * 3 + 4"
    };
    
    for (const auto& expr : expressions) {
        auto tokens = tokenize(expr);
        auto rpn = shunting_yard(tokens);
        int result = evaluate_rpn(rpn);
        
        std::cout << std::left << std::setw(15) << expr 
                  << " -> " << std::setw(15) << join(rpn) 
                  << " = " << result << std::endl;
    }
    
    return 0;
}

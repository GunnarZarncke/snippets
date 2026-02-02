#!/usr/bin/env python3
"""
Minimal Shunting Yard algorithm implementation.
Converts infix notation to postfix (RPN) notation.
"""

def precedence(op):
    """Return precedence of operator (higher = more precedence)."""
    if op in ('+', '-'):
        return 1
    if op in ('*', '/'):
        return 2
    return 0

def shunting_yard(tokens):
    """Convert infix expression tokens to postfix (RPN) notation."""
    output = []
    operators = []
    
    for token in tokens:
        if token.isdigit():
            output.append(token)
        elif token == '(':
            operators.append(token)
        elif token == ')':
            while operators and operators[-1] != '(':
                output.append(operators.pop())
            operators.pop()
        elif token in '+-*/':
            while (operators and operators[-1] != '(' and 
                   precedence(operators[-1]) >= precedence(token)):
                output.append(operators.pop())
            operators.append(token)
    
    while operators:
        output.append(operators.pop())
    
    return output

def evaluate_rpn(tokens):
    """Evaluate postfix (RPN) expression."""
    stack = []
    for token in tokens:
        if token.isdigit():
            stack.append(int(token))
        else:
            b = stack.pop()
            a = stack.pop()
            if token == '+':
                stack.append(a + b)
            elif token == '-':
                stack.append(a - b)
            elif token == '*':
                stack.append(a * b)
            elif token == '/':
                stack.append(a // b)
    return stack[0]

def tokenize(expression):
    """Split expression into tokens (numbers and operators)."""
    tokens = []
    current = ''
    for char in expression.replace(' ', ''):
        if char.isdigit():
            current += char
        else:
            if current:
                tokens.append(current)
                current = ''
            tokens.append(char)
    if current:
        tokens.append(current)
    return tokens

def main():
    """Example usage."""
    expressions = [
        "3 + 4 * 2",
        "(3 + 4) * 2",
        "10 - 2 * 3",
        "2 * 3 + 4"
    ]
    
    for expr in expressions:
        tokens = tokenize(expr)
        rpn = shunting_yard(tokens)
        result = evaluate_rpn(rpn)
        print(f"{expr:15} -> {' '.join(rpn):15} = {result}")

if __name__ == '__main__':
    main()

# Zap спецификация

### Типы даных
- Целочисленный тип (int64)
- Числа с плавающей запятой (float64)
- Символ
- Строка
- Булевые
- Массив

### Базовый синтаксис
```
PROGRAM - FUNC_STATEMENT

EXPRESSION - (IDENTIFIER | INTEGER | FLOAT | CHAR | STRING | BOOL | ARRAY | CALL_EXPRESSION | PREFIX_EXPRESSION | INFIX_EXPRESSION | GROUP_EXPRESSION)
STATEMENT - (LET_STATEMENT | EXPRESSION_STATEMENT | IF_STATEMENT | FOR_STATEMENT | WHILE_STATEMENT | FUNC_STATEMENT | RETURN_STATEMENT | ASSIGN_STATEMENT)

DATATYPE - ('int' | 'float' | 'char' | 'string' | 'bool' | ARRAY_TYPE)
ARRAY_TYPE - LBRACKET RBRACKET DATATYPE

EXPRESSION_STATEMENT - EXPRESSION

BODY - LCURLY \{ STATEMENT_LIST \} RCURLY
STATEMENT_LIST - \{ STATEMENT \{ NEW_LINE STATEMENT \} \}
NEW_LINE - '\n'
```
### Объявление и инициализация переменной
```
LET_STATEMENT - 'let' IDENTIFIER '=' EXPRESSION
ASSIGN_STATEMENT - IDENTIFIER '=' EXPRESSION
```
### Условный оператор
```
IF_STATEMENT - 'if' CONDITION BODY \{ 'else if' CONDITION BODY \} \[ 'else' BODY \]
```
### Циклы
```
FOR_STATEMENT - 'for' VARINIT SEMICOLON CONDITION SEMICOLON STATEMENT BODY

WHILE_STATEMENT - 'while' CONDITION BODY
```
### Функции
```
ARGUMENT - DATATYPE IDENTIFIER
ARGUMENT_LIST - \[ ARGUMENT \{ COMMA ARGUMENT \} \]
FUNC_STATEMENT - 'func' IDENTIFIER LBRACE ARGUMENT_LIST RBRACE DATATYPE BODY

BUILT-IN_FUNCTION - ('len' | 'read' | 'print' | DATATYPE)
CALL_EXPRESSION - (BUILT-IN_FUNCTION | IDENTIFIER) LBRACE ARGUMENT_LIST RBRACE
RETURN_STATEMENT - 'return' EXPRESSION
```
### Операции
```
INFIX_OPERATOR - ('+' | '-' | '/' | '*' | '**' | '%' | '||' | '&&' | '==' | '#')
PREFIX_OPERATOR - ('!' | '-')

INFIX_EXPRESSION - EXPRESSION INFIX_OPERATOR EXPRESSION
PREFIX_EXPRESSION - PREFIX_OPERATOR EXPRESSION
GROUP_EXPRESSION - LBRACE EXPRESSION RBRACE
```
### Примитивы
```
DOT - '.'
COMMA - ','
SEMICOLON - ';'

DIGIT - (0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9)
NATURAL - (1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9)
SIGN - ('+' | '-')

INTEGER - \[ SIGN \] DIGIT \{ DIGIT \}
FLOAT - \[ SIGN \] INTEGER DOT INTEGER
CHAR - ('A' | 'B' | 'C' | 'D' | 'E' | 'F' | 'G' | 'H' | 'I' | 'J' | 'K' | 'L' | 'M' | 'N' | 'O' | 'P' | 'Q' | 'R' | 'S' | 'T' | 'U' | 'V' | 'W' | 'X' | 'Y' | 'Z')
STRING - CHAR \{ CHAR \}
BOOL - ('true' | 'false')
ARRAY - ARRAY_TYPE LCURLY EXPRESSION \{ COMMA EXPRESSION \} RCURLY
```

## Стандартные функции

- print - вывод на экран
- len - возвращает длину массива
- read - считывает ввод с консоли и возвращает его в виде строки
- DATATYPE - переводит значение к указанному типу данных


# Этапы компиляции

1) Lexer - Поток символов -> Поток токенов (валидация по допустимым символам)
2) Token Parser - Поток токенов -> Корень AST (валидация по синтаксису)
3) Bytecode Generator - Корень AST -> байткод (валидация по семантике типов и именам переменных)
4) JIT Compiler - исполняет байткод (оптимизация кода)

### JIT Compiler
- 

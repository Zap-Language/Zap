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
INFIX_OPERATOR - ('+' | '-' | '/' | '*' | '**' | '%' | '||' | '&&' | '==' | '#' | '>' | '<')
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

## Стандартные ошибки

ArrayIndexOutOfBoundsError - когда индекс < 0 или ≥ длины массива\
DivisionByZeroError - деление на 0\
StackUnderflowError - переполнение стека\
CallStackOverflowError - переполнение стека вызовов\
BuiltinError - ошибка внутри встроенных функций\
TypeCastException - возникает при неудачной попытке преобразования типов\
OutOfMemoryException - в куче недостаточно памяти для выделения нового объекта\
InvalidOperationException - возникает при вызове операции, которая недопустима в текущем контексте\
SyntaxError - нарушении синтаксических правил языка\
TypeError - несоответствии типов в выражениях\
UndefinedVariableError - попытка использования необъявленной переменной\
RedefinitionError - попытка повторного объявления переменной в той же области видимости

## Итоговоговый синтксиси(EBNF)

```ebnf
PROGRAM          ::= FUNC_STATEMENT

STATEMENT        ::= LET_STATEMENT 
                   | EXPRESSION_STATEMENT 
                   | IF_STATEMENT 
                   | FOR_STATEMENT 
                   | WHILE_STATEMENT 
                   | FUNC_STATEMENT 
                   | RETURN_STATEMENT 
                   | ASSIGN_STATEMENT

EXPRESSION       ::= IDENTIFIER 
                   | INTEGER 
                   | FLOAT 
                   | CHAR 
                   | STRING 
                   | BOOL 
                   | ARRAY 
                   | CALL_EXPRESSION 
                   | PREFIX_EXPRESSION 
                   | INFIX_EXPRESSION 
                   | GROUP_EXPRESSION

DATATYPE         ::= "int" | "float" | "char" | "string" | "bool" | ARRAY_TYPE
ARRAY_TYPE       ::= "[" "]" DATATYPE

LET_STATEMENT    ::= "let" IDENTIFIER "=" EXPRESSION
ASSIGN_STATEMENT ::= IDENTIFIER "=" EXPRESSION

IF_STATEMENT     ::= "if" CONDITION BODY 
                     { "else if" CONDITION BODY } 
                     [ "else" BODY ]

FOR_STATEMENT    ::= "for" VARINIT ";" CONDITION ";" STATEMENT BODY
WHILE_STATEMENT  ::= "while" CONDITION BODY

FUNC_STATEMENT   ::= "func" IDENTIFIER "(" ARGUMENT_LIST ")" DATATYPE BODY
ARGUMENT         ::= DATATYPE IDENTIFIER
ARGUMENT_LIST    ::= [ ARGUMENT { "," ARGUMENT } ]
RETURN_STATEMENT ::= "return" EXPRESSION

CALL_EXPRESSION  ::= (BUILT-IN_FUNCTION | IDENTIFIER) "(" ARGUMENT_LIST ")"
BUILT-IN_FUNCTION::= "len" | "read" | "print" | DATATYPE

INFIX_EXPRESSION ::= EXPRESSION INFIX_OPERATOR EXPRESSION
PREFIX_EXPRESSION::= PREFIX_OPERATOR EXPRESSION
GROUP_EXPRESSION ::= "(" EXPRESSION ")"

INFIX_OPERATOR   ::= "+" | "-" | "/" | "*" | "**" | "%" | "||" | "&&" | "==" | "#" | ">" | "<"
PREFIX_OPERATOR  ::= "!" | "-"

BODY             ::= "{" STATEMENT_LIST "}"
STATEMENT_LIST   ::= STATEMENT { NEW_LINE STATEMENT }
CONDITION        ::= EXPRESSION

IDENTIFIER       ::= LETTER { LETTER | DIGIT }
NEW_LINE         ::= "\n"

DOT              ::= "."
COMMA            ::= ","
SEMICOLON        ::= ";"

DIGIT            ::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
SIGN             ::= "+" | "-"

INTEGER          ::= [ SIGN ] DIGIT { DIGIT }
FLOAT            ::= [ SIGN ] INTEGER "." INTEGER
CHAR             ::= "A" | "B" | ... | "Z" | "a" | "b" | ... | "z" | "_" 
STRING           ::= CHAR { CHAR }
BOOL             ::= "true" | "false"
ARRAY            ::= DATATYPE "[" "]" "{" EXPRESSION { "," EXPRESSION } "}" 
```

# Этапы компиляции

### 1. Lexer

Лексер преобразует исходный текст программы в последовательность токенов. На этом этапе входной поток символов
считывается посимвольно, и из него формируются токены.
Также выполняет валидацию входного кода на допустимые символы и лексические шаблоны.

```zap
// Исходный код
let x = 5 + 10

// Выходные токены
(IDENTIFIER x) (INTEGER 5) (INFIX_OPERATOR +) (INTEGER 10) (SEMICOLON ;)
```

### 2. Token Parser

Получает на вход поток токенов от лекcера и строит на его основе абстрактное синтаксическое дерево AST.

Парсер проверяет корректность конструкции программы согласно правилам грамматики языка: он пытается разобрать
последовательность токенов в соответствии с
определенными синтаксическими правилами. Если токены расположены в неправильном порядке (нарушение грамматики), парсер
сообщит об ошибке синтаксиса.

Также формирует таблицу символов (symbol table) и отслеживает области видимости переменных и функций.
При использовании идентификатора парсер проверяет, объявлен ли он в доступной области видимости; если нет – выдаётся
ошибка о неизвестной переменной.

После построения дерева происходит проверка типов, например что в условии if стоит выражение логического типа,
что возвращаемое функцией значение соответствует объявленному типу функции.

Результатом работы парсера является корень AST, в дереве все типы проверенны и проверено соответствие семантике.

### 3. Преобразования и оптимизации AST

Разворачивание синтаксического сахара и простейшие оптимизации.

Например, конструкция while приводится к эквивалентной форме for, также происходит константная свёртка (вычисление
константных выражений) и
устранение недостижимых ветвей условных операторов, если условие известно на этапе компиляции.

### 4. Bytecode Generator

Генератор байт-кода обходит AST дерево программы и переводит каждую конструкцию в последовательность инструкций
стекового байт-кода.

Обходя AST, компилятор последовательно генерирует инструкции: для узла AST сначала генерируется код для его
подвыражений (например, для бинарной операции – код для левого и правого операндов),
который помещает необходимые значения на стек, а затем генерируется инструкция, выполняющая операцию над вершинами
стека.

Основные виды инструкций и формат байт-кода:

- **Загрузка констант и переменных:** PUSH_INT <value> кладёт целое число на стек, LOAD <addr> извлекает значение
  переменной из памяти.


- **Арифметические и логические операции:** ADD сложение, SUB вычитание, MUL умножение, DIV деление, MOD остаток.
  Логические: AND, OR для булевых значений. Операции сравнения: CMP_EQ сравнить на равенство, CMP_LT меньше,
  возвращающие на стек результат типа BOOL.
  При выполнении таких инструкций виртуальная машина снимает два верхних значения со стека, применяет операцию и
  результат кладёт обратно.


- **Управление переменными:** STORE <addr> берёт верхний элемент из стека и сохраняет его по адресу переменной


- **Переходы:** JMP <label> (безусловный переход) и условный, например JMP_IF <label> – проверяет верхний элемент стека
  как булево значение;
  если оно ложно (false), то переходит на указанный адрес, иначе продолжает со следующей инструкции.


- **Вызов функции:** CALL <f> сохраняет текущий адрес выполнения и передаёт управление на начало функции f.
  По завершении функции инструкция RET возвращает управление обратно.

Каждая строка байт-кода имеет вид: [АДРЕС] ОПЕРАЦИЯ [ОПЕРАНДЫ]. Например: ```10: JMP 2``` означает на адресе 10
находится команда безусловного перехода на адрес 2.

Обходим дерево и в стек.

### 5. Сохранение байт-кода и таблиц в память

После генерации байт-кода компилятор сохраняет последовательность инструкций в память, а также сопровождающие
структуры – например,
таблицу символов и информацию о функциях.

## JIT-Компилятор

### 1. Исполнение полученного байт-кода интерпретатором

Загружаем файл с программой(байт-код), таблиц символом и информации о функциях.

Интерпретатор последовательно исполняет байт-код, при этом ведет сбор статистики –  
сколько раз выполнялась каждая функция или цикл. Это реализуется с помощью счетчиков исполнения, ассоциированных с
определёнными участками кода.
Когда количество исполнений какого-либо участка превышает заданный порог, этот участок считается горячим.

### 2.Оптимизация кода

Фрагмент кода, который был идентифицирован как горячий, и переводит его в промежуточное представление.

Далее начинают применяться оптимизации:

1. Встраивание функций:
   Замена вызова функции ее непосредственным телом, тем самым убираю лишние переходы и расходы на вызов/возврат.
   
    Исходная программа:
    ```
    func square(x int) int { return x * x; }
    let a = 7;
    let b = square(a);
    return b + 1;
    ```

   Байт-код до:

    ```
    [000] PUSH_INT 7
    [001] STORE 0
    [002] LOAD 0
    [003] CALL square
    [004] STORE 1
    [005] LOAD 1
    [006] PUSH_INT 1
    [007] ADD
    [008] RET
    ```
   Функция square:
    ```
    [square:000] LOAD 0
    [square:001] LOAD 0
    [square:002] MUL
    [square:003] RET
    ```
   После оптимизации:
    ```
   [000] PUSH_INT 7
    [001] STORE 0
    ; --- inline square(a) ---
    [002] LOAD 0
    [003] LOAD 0
    [004] MUL
    ; -------------------------
    [005] STORE 1
    [006] LOAD 1
    [007] PUSH_INT 1
    [008] ADD
    [009] RET
    ```

2. Удаление мёртвого кода:
   Исключение инструкций, которые не влияют на дальнейший результат программы.
3. Свёртка констант
   Вычисление константных выражений во время компиляции.

   До:
    ```
    [000] PUSH_INT 5
    [001] PUSH_INT 10
    [002] ADD
    [003] STORE 0 ; x
    ```
   После:
    ```
    [000] PUSH_INT 15
    [001] STORE 0
    ```
4. Развёртка циклов
   Уменьшаем число проверок условия и прыжков, дублируя тело цикла на несколько итераций за раз.
    
    Исходная пограмма:
    ```
    let s = 0;
    let i = 0;
    while (i < 4) {
    s = s + i;
    i = i + 1;
    }
    return s;
    ```

   Байт-код до:

    ```
    [000] PUSH_INT 0
    [001] STORE 0            ; s
    [002] PUSH_INT 0
    [003] STORE 1            ; i
    [004] LOAD 1
    [005] PUSH_INT 4
    [006] CMP_LT
    [007] JMP_IF 018         ; if !(i<4) -> end
    [008] LOAD 0
    [009] LOAD 1
    [010] ADD
    [011] STORE 0            ; s += i
    [012] LOAD 1
    [013] PUSH_INT 1
    [014] ADD
    [015] STORE 1            ; i++
    [016] JMP 004            ; loop
    [018] LOAD 0
    [019] RET
    ```
   После оптимизации:
    ```
    [000] PUSH_INT 0
    [001] STORE 0            ; s
    [002] PUSH_INT 0
    [003] STORE 1            ; i
    
    ; основной цикл выполняет сразу 2 итерации
    [004] LOAD 1
    [005] PUSH_INT 3         ; 4 - (2-1) = 3
    [006] CMP_LT
    [007] JMP_IF 030         ; если i >= 3 -> в хвост
    
    ; итерация #1
    [008] LOAD 0
    [009] LOAD 1
    [010] ADD
    [011] STORE 0
    [012] LOAD 1
    [013] PUSH_INT 1
    [014] ADD
    [015] STORE 1
    
    ; итерация #2
    [016] LOAD 0
    [017] LOAD 1
    [018] ADD
    [019] STORE 0
    [020] LOAD 1
    [021] PUSH_INT 1
    [022] ADD
    [023] STORE 1
    
    [024] JMP 004            ; следующая пачка
    
    ; хвост — добежка оставшейся 1 итерации (если была)
    [030] LOAD 1
    [031] PUSH_INT 4
    [032] CMP_LT
    [033] JMP_IF 044         ; если i >= 4 -> end
    [034] LOAD 0
    [035] LOAD 1
    [036] ADD
    [037] STORE 0
    [038] LOAD 1
    [039] PUSH_INT 1
    [040] ADD
    [041] STORE 1
    [042] JMP 030
    
    [044] LOAD 0
    [045] RET
    ```
5. Устранение общих подвыражений
   Если одно и то же выражение вычисляется несколько раз и его операнды не менялись — вычисляем один раз и
   переиспользуем.

### 3. Подствляем оптмизированнй код в нашу программу

После оптимизации выбранного участка, выполняется генерация генерацию машинного кода, который затем помещается
в кэш кода и привязывается к системе исполнения. Например, обновить указатель на функцию: теперь вместо интерпретации
байт-кода при следующем вызове функции управление перейдет к скомпилированному высокопроизводительному коду. В случае
циклов, вставить в байт-код прыжок
в машинный код оптимизированного тела цикла.

## Gc

Подсистема управления памятью, которая работает вместе с интерпретатором и оптимизатором кода, основанная на алгоритме
пометки-и-сборки (Mark-and-Sweep).

1. Фаза пометки
    - Обход графа объектов, начиная с корневых ссылок
    - Пометка всех достижимых объектов

2. Фаза сборки
    - При достижении порога выделенной памяти, программа ставится на паузу
    - Последовательное сканирование кучи
    - Освобождение памяти от непомеченных объектов
    - Компоновка памяти при превышении порога фрагментации

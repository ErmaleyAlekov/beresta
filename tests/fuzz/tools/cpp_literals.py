"""Извлечение строковых литералов C++ из исходников модульных тестов.

Модульные тесты «Берёсты» держат примеры XML, XPath и XSD прямо в коде
строковыми литералами. Соседние литералы ("a" "b") склеиваются, как это
делает компилятор, поэтому многострочные схемы извлекаются целиком.
Сырые строки R"(...)" и префиксы u8/L не поддерживаются — в тестах их нет.

Extracting C++ string literals from the unit test sources.
The Beresta unit tests keep XML, XPath and XSD examples right in the code
as string literals. Adjacent literals ("a" "b") are joined, as the
compiler does, so multi-line schemas are extracted whole.
Raw strings R"(...)" and the u8/L prefixes are not supported — the tests have none.
"""

import re

# Литерал в двойных кавычках с экранированными символами внутри
# A literal in double quotes with escaped characters inside
LITERAL = re.compile(r'"((?:[^"\\\n]|\\.)*)"')

# Простые escape-последовательности C++ и их значения
# Simple C++ escape sequences and their values
SIMPLE_ESCAPES = {"n": "\n", "t": "\t", "r": "\r", "\\": "\\", '"': '"', "'": "'", "0": "\0"}


def decode_escapes(body):
    """Раскрывает \\n, \\t, \\", \\\\ и \\xHH; неизвестные оставляет как есть.
    Expands \\n, \\t, \\", \\\\ and \\xHH; leaves unknown ones as is."""
    result = []
    index = 0
    while index < len(body):
        symbol = body[index]
        if symbol != "\\" or index + 1 >= len(body):
            result.append(symbol)
            index += 1
            continue
        code = body[index + 1]
        if code == "x":
            digits = re.match(r"[0-9a-fA-F]{1,2}", body[index + 2:])
            if digits:
                result.append(chr(int(digits.group(0), 16)))
                index += 2 + len(digits.group(0))
                continue
        result.append(SIMPLE_ESCAPES.get(code, code))
        index += 2
    return "".join(result)


def read_literals(path):
    """Все литералы файла; подряд идущие (через пробелы и переводы строк) склеены.
    All literals of a file; consecutive ones (through spaces and line breaks) are joined."""
    with open(path, encoding="utf-8") as stream:
        text = stream.read()
    literals = []
    last_end = -1
    for match in LITERAL.finditer(text):
        value = decode_escapes(match.group(1))
        if literals and text[last_end:match.start()].strip() == "":
            literals[-1] += value
        else:
            literals.append(value)
        last_end = match.end()
    return literals

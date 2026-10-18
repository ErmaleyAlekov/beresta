"""Сборка начального корпуса для целей фаззинга «Берёсты».

Источники:
  - документы набора W3C (tests/conformance/xmlconf), если он скачан;
  - строковые литералы модульных тестов (tests/src): XML, XPath, XSD.
Формат каждого файла корпуса совпадает с форматом входа своей цели:
байт флагов, затем части через разделитель 0xF5 0xF5.

Запуск: python3 -I tests/fuzz/tools/make_corpus.py <корень проекта> <папка корпуса>

Building the initial corpus for the Beresta fuzzing targets.
Sources:
- the documents of the W3C suite (tests/conformance/xmlconf), if it is downloaded;
- the string literals of the unit tests (tests/src): XML, XPath, XSD.
The format of each corpus file matches the input format of its target:
the flag byte, then the parts separated by 0xF5 0xF5.
Run: python3 -I tests/fuzz/tools/make_corpus.py <project root> <corpus folder>
"""

import glob
import hashlib
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cpp_literals  # noqa: E402  (путь добавлен строкой выше) / the path is added on the line above

SEPARATOR = b"\xF5\xF5"
MAX_SEED = 64 * 1024
# Обёртка для фрагментов схем: тесты часто склеивают заголовок и тело в рантайме
# A wrapper for schema fragments: the tests often join the header and the body at run time
SCHEMA_HEAD = '<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">'
SCHEMA_TAIL = "</xs:schema>"
# Сочетания флагов парсера: без всего; NS+DTD+внешний DTD; всё сразу
# Combinations of parser flags: nothing; NS+DTD+external DTD; everything at once
PARSER_FLAGS = (0x00, 0x07, 0xFF)


def write_seed(folder, payload):
    """Файл корпуса с именем по SHA-1 содержимого (повторы схлопываются).
    A corpus file named by the SHA-1 of its content (duplicates collapse)."""
    if len(payload) > MAX_SEED:
        return
    name = hashlib.sha1(payload).hexdigest()
    with open(os.path.join(folder, name), "wb") as stream:
        stream.write(payload)


def collect_literals(root, pattern):
    """Литералы всех тестов, подходящих под шаблон пути (относительно tests/src).
    The literals of all tests matching the path pattern (relative to tests/src)."""
    literals = []
    for path in sorted(glob.glob(os.path.join(root, "tests", "src", pattern), recursive=True)):
        literals.extend(cpp_literals.read_literals(path))
    return literals


def write_parser_corpus(root, folder):
    """XML-документы W3C и XML-литералы тестов — с каждым набором флагов.
    W3C XML documents and the XML literals of the tests — with each set of flags."""
    documents = [text.encode("utf-8") for text in collect_literals(root, "**/*.cpp") if "<" in text]
    suite = os.path.join(root, "tests", "conformance", "xmlconf")
    for path in glob.glob(os.path.join(suite, "**", "*.xml"), recursive=True):
        with open(path, "rb") as stream:
            documents.append(stream.read())
    for document in documents:
        for flags in PARSER_FLAGS:
            write_seed(folder, bytes([flags]) + document)
    return len(documents)


def write_query_corpora(root, xpath_folder, schema_folder):
    """XPath-выражения и пары «схема + документ» из тестов XPath и XSD.
    XPath expressions and "schema + document" pairs from the XPath and XSD tests."""
    expressions = [text for text in collect_literals(root, "xpath/*.cpp") if "<" not in text]
    for expression in expressions:
        write_seed(xpath_folder, b"\x00" + expression.encode("utf-8"))
        write_seed(xpath_folder, b"\x01" + expression.encode("utf-8"))
    xsd_literals = collect_literals(root, "xsd/*.cpp")
    schemas = [text for text in xsd_literals if "<xs:schema" in text]
    schemas += [SCHEMA_HEAD + text + SCHEMA_TAIL for text in xsd_literals
                if "<xs:" in text and "<xs:schema" not in text]
    instances = [text for text in xsd_literals if "xs:" not in text and "<" in text]
    for schema in schemas:
        for instance in instances[:40]:
            write_seed(schema_folder, b"\x00" + schema.encode("utf-8") + SEPARATOR
                + instance.encode("utf-8"))
    return len(expressions), len(schemas)


def main():
    """Создаёт папки parser/, xpath/, schema/ в каталоге корпуса и заполняет их.
    Creates the parser/, xpath/, schema/ folders in the corpus directory and fills them."""
    root, corpus = sys.argv[1], sys.argv[2]
    folders = {name: os.path.join(corpus, name) for name in ("parser", "xpath", "schema")}
    for folder in folders.values():
        os.makedirs(folder, exist_ok=True)
    documents = write_parser_corpus(root, folders["parser"])
    expressions, schemas = write_query_corpora(root, folders["xpath"], folders["schema"])
    print("документов: %d, выражений XPath: %d, схем: %d" % (documents, expressions, schemas))


if __name__ == "__main__":
    main()

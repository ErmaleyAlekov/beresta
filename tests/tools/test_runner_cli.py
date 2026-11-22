"""Регрессии CLI прогонщиков и бенчмарка на локальных мини-наборах.
CLI regressions for runners and the benchmark using small local fixtures.
"""

import argparse
from pathlib import Path
import subprocess
import tempfile


# Сверяем код возврата и сохраняем оба потока в диагностике.
# Check the exit status and retain both streams for diagnostics.
def check(binary, root, expected, *arguments):
    command = [str(binary), *arguments]
    result = subprocess.run(
        command, cwd=root, capture_output=True, encoding="utf-8",
        errors="replace", timeout=30,
    )
    if result.returncode != expected:
        raise AssertionError(
            f"{command} -> {result.returncode} (expected {expected})\n"
            f"{result.stdout}\n{result.stderr}"
        )
    return result


# Каталог содержит проходящий случай и случай со сломанным эталоном.
# The catalog contains a passing case and a case with a missing reference.
def check_conformance(binary, root):
    (root / "document.xml").write_text(
        "<!DOCTYPE root [<!ELEMENT root EMPTY>]><root/>", encoding="utf-8",
    )
    (root / "xmlconf.xml").write_text(
        '<TESTCASES PROFILE="Local"><TEST ID="pass" TYPE="valid" URI="document.xml"/>'
        '<TEST ID="fail" TYPE="valid" URI="document.xml" OUTPUT="missing.xml"/>'
        '</TESTCASES>',
        encoding="utf-8",
    )
    check(binary, root, 0, ".", "--only", "pass")
    check(binary, root, 1, ".", "--only", "fail")
    check(binary, root, 2, ".", ".")
    check(binary, root, 2, ".", "--report")
    (root / "pi.xml").write_text(
        "<?pi data?><!DOCTYPE doc [<!ELEMENT doc EMPTY>]><doc/>", encoding="utf-8",
    )
    (root / "expected.xml").write_text(
        "<?pi data?>\n<!DOCTYPE doc [\n]>\n<doc></doc>", encoding="utf-8",
    )
    (root / "xmlconf.xml").write_text(
        '<TESTCASES><TEST ID="pi" TYPE="valid" URI="pi.xml" OUTPUT="expected.xml"/>'
        '</TESTCASES>', encoding="utf-8",
    )
    check(binary, root, 0, ".")


# Идентификатор случая также встречается как значение TYPE в чужом подкаталоге.
# The case ID also occurs as a TYPE value in an unrelated subcatalog.
def check_entity_base(binary, root):
    for name in ("a", "b"):
        (root / name).mkdir()
    (root / "a" / "catalog.xml").write_text(
        '<TEST TYPE="valid" ID="other" URI="first.xml"/>', encoding="utf-8",
    )
    (root / "b" / "catalog.xml").write_text(
        "<TEST TYPE='valid' ID='&caseid;' URI='second.xml'/>", encoding="utf-8",
    )
    (root / "b" / "second.xml").write_text("<!DOCTYPE doc [<!ELEMENT doc EMPTY>]><doc/>", encoding="utf-8")
    (root / "xmlconf.xml").write_text(
        '<!DOCTYPE TESTCASES [<!ENTITY caseid "valid"><!ENTITY a SYSTEM "a/catalog.xml">'
        '<!ENTITY b SYSTEM "b/catalog.xml">]><TESTCASES>&a;&b;</TESTCASES>', encoding="utf-8",
    )
    outcome = check(binary, root, 0, ".", "--only", "valid", "--verbose")
    if "отсутствует" in outcome.stdout:
        raise AssertionError(outcome.stdout)


# Ожидание XSD 1.1 стоит первым, а для XSD 1.0 схема считается валидной.
# The XSD 1.1 expectation comes first, while XSD 1.0 considers the schema valid.
def check_xsts(binary, root):
    (root / "suite.xml").write_text(
        '<suite xmlns="http://www.w3.org/XML/2004/xml-schema-test-suite/"'
        ' xmlns:xlink="http://www.w3.org/1999/xlink">'
        '<testSetRef xlink:href="cases.xml"/></suite>', encoding="utf-8",
    )
    (root / "schema.xsd").write_text(
        '<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">'
        '<xs:element name="root" type="xs:string"/></xs:schema>', encoding="utf-8",
    )
    (root / "cases.xml").write_text(
        '<testSet name="local" xmlns="http://www.w3.org/XML/2004/xml-schema-test-suite/"'
        ' xmlns:xlink="http://www.w3.org/1999/xlink">'
        '<testGroup name="pass"><schemaTest name="schema">'
        '<schemaDocument xlink:href="schema.xsd"/>'
        '<expected version="1.1" validity="invalid"/>'
        '<expected version="1.0" validity="valid"/>'
        '</schemaTest></testGroup>'
        '<testGroup name="fail"><schemaTest name="schema">'
        '<schemaDocument xlink:href="schema.xsd"/>'
        '<expected validity="invalid"/>'
        '</schemaTest></testGroup></testSet>',
        encoding="utf-8",
    )
    (root / "limitations.txt").write_text("", encoding="utf-8")
    suite = (".", "--limitations", "limitations.txt")
    check(binary, root, 0, *suite, "--only", "local/pass")
    check(binary, root, 1, *suite, "--only", "local/fail")
    check(binary, root, 2, *suite, "--report")
    metadata = root / "cases.xml"
    text = metadata.read_text(encoding="utf-8")
    metadata.write_text(text.replace('name="fail"', 'name="fail" version="1.1"'), encoding="utf-8")
    check(binary, root, 0, *suite)
    metadata.write_text(text.replace('href="schema.xsd"', 'href="absent.xsd"', 1), encoding="utf-8")
    check(binary, root, 1, *suite, "--report", "missing.md")
    report = (root / "missing.md").read_text(encoding="utf-8")
    if "local/pass/schema" not in report or "local/fail/schema" not in report:
        raise AssertionError(report)


# Список должен содержать те же два случая, что и основной прогонщик.
# The list must contain the same two cases as the main runner.
def check_regeneration(binary, root):
    check(binary, root, 0, "--regenerate", "--suite", ".",
          "--limitations", "limitations.txt", "--list", "failures.tsv")
    text = (root / "failures.tsv").read_text(encoding="utf-8")
    ids = {line.split("\t", 1)[0] for line in text.splitlines() if not line.startswith("#")}
    if ids != {"local/pass/schema", "local/fail/schema"}:
        raise AssertionError(text)


# Проверяем отказ до запуска замеров или чтения списка случаев.
# Check that the tools reject these inputs before timing or reading the case list.
def check_arguments(failing, benchmark, root):
    missing = check(failing, root, 2, "--case")
    overflow = check(failing, root, 2, "--case", "9" * 40)
    if "нет значения для --case" not in missing.stderr:
        raise AssertionError(missing.stderr)
    if "слишком большой номер случая" not in overflow.stderr:
        raise AssertionError(overflow.stderr)
    for arguments in (
        ("--runs", "0"), ("--runs", "-1"), ("--items", "10abc"),
        ("--memory", "-1", "catalog"),
    ):
        check(benchmark, root, 2, *arguments)
    check(
        benchmark, root, 2, "--runs", "1", "--items", "1",
        "--paragraphs", "1", "--report", "missing/report.md",
    )


# Двоичные файлы передаются явно, чтобы тест работал с CMake и Make.
# Pass executable paths explicitly so the script works with CMake and Make.
def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ("conformance", "xsts", "failing", "benchmark"):
        parser.add_argument("--" + name, type=Path, required=True)
    binaries = parser.parse_args()
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        check_conformance(binaries.conformance.resolve(), root)
        check_entity_base(binaries.conformance.resolve(), root)
        check_xsts(binaries.xsts.resolve(), root)
        check_regeneration(binaries.failing.resolve(), root)
        check_arguments(binaries.failing.resolve(), binaries.benchmark.resolve(), root)


if __name__ == "__main__":
    main()

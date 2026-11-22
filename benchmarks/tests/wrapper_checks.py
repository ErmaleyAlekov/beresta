"""Проверка таблиц обёрток / Check wrapper benchmark tables.

python benchmarks/tests/wrapper_checks.py dom python benchmarks/wrappers/python/bench.py
python benchmarks/tests/wrapper_checks.py stream dotnet WrapperBench.dll {data} --stream
"""

import argparse
import subprocess
import tempfile
from pathlib import Path


def check_table(output, width, expected):
    """Проверяет форму и смысл последнего столбца / Check table shape and validity."""
    rows = [line.strip("|").split("|") for line in output.splitlines() if line.startswith("|")]
    if len(rows) < 3 or any(len(row) != width for row in rows):
        raise AssertionError(output)
    supported = 0
    for row in rows[2:]:
        errors, valid = (cell.strip() for cell in row[-2:])
        if errors == valid == "—":
            continue
        supported += 1
        if valid != expected or (int(errors) == 0) != (valid == "yes"):
            raise AssertionError(row)
    if not supported:
        raise AssertionError("нет движка XSD / no XSD engine")


def check(mode, command, data):
    """Одна схема, два исхода валидации / One schema and two validation outcomes."""
    (data / "catalog.xsd").write_text(
        '<xs:schema xmlns:xs="http://www.w3.org/2001/XMLSchema">'
        '<xs:element name="r" type="xs:integer"/></xs:schema>', encoding="utf-8")
    (data / "query.txt").write_text("//r", encoding="utf-8")
    if "{data}" not in command:
        command.append("{data}")
    args = [str(data) if arg == "{data}" else arg for arg in command]
    for value, expected in (("7", "yes"), ("word", "no")):
        (data / "catalog.xml").write_text(f"<r>{value}</r>", encoding="utf-8")
        output = subprocess.check_output(args, encoding="utf-8", timeout=60)
        check_table(output, 9 if mode == "dom" else 4, expected)
    if mode == "dom":
        result = subprocess.run([*args, "--memory", "99"], capture_output=True, timeout=60)
        if not result.returncode or b"--memory" not in result.stderr:
            raise AssertionError(result)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=["dom", "stream"])
    parser.add_argument("command", nargs=argparse.REMAINDER)
    options = parser.parse_args()
    if not options.command:
        parser.error("укажите команду / missing benchmark command")
    with tempfile.TemporaryDirectory() as directory:
        check(options.mode, options.command, Path(directory))

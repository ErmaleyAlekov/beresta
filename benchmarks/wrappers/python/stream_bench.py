"""
Потоковая проверка по XSD без дерева на тех же данных, что и bench.py:
«Берёста» (Schema.validate_bytes — SAX + XSD внутри ядра) против lxml
(iterparse со схемой; элементы очищаются по ходу, дерево не копится).
Медиана из RUNS прогонов после одного прогревочного.

    python stream_bench.py data

Streaming XSD validation without a tree on the same data as bench.py:
Beresta (Schema.validate_bytes — SAX + XSD inside the core) against lxml
(iterparse with a schema; elements are cleared on the way, no tree piles up).
The median of RUNS runs after one warm-up run.
    python stream_bench.py data
"""

import io
import pathlib
import statistics
import sys
import time

import beresta

RUNS = 5


def beresta_once(schema, xml: bytes) -> int:
    """Одна проверка «Берёстой»: число нарушений. / One Beresta validation: the violation count."""
    return len(schema.validate_bytes(xml))


def lxml_once(schema, xml: bytes) -> int:
    """Потоковая проверка lxml с подсчётом нарушений схемы.
    Streaming lxml validation with a count of schema violations."""
    from lxml import etree
    parser = etree.iterparse(io.BytesIO(xml), events=("end",), schema=schema)
    try:
        for _, element in parser:
            element.clear()
            parent = element.getparent()
            while parent is not None and element.getprevious() is not None:
                del parent[0]
    except etree.XMLSyntaxError:
        errors = list(parser.error_log)
        if not errors or any(error.domain_name != "SCHEMASV" for error in errors):
            raise
        return len(errors)
    return 0


def speed(once, schema, xml: bytes) -> str:
    """Скорость в МБ/с по медиане времени. / The speed in MB/s by the median time."""
    times = []
    for run in range(RUNS + 1):
        start = time.perf_counter()
        once(schema, xml)
        if run > 0:
            times.append(time.perf_counter() - start)
    elapsed = statistics.median(times)
    if elapsed <= 0:
        return "—"
    return "%.0f" % (len(xml) / 1e6 / elapsed)


def main() -> None:
    """Таблица Markdown; lxml пропускается, если не установлена.
    A Markdown table; lxml is skipped if it is not installed."""
    data = pathlib.Path("data")
    if len(sys.argv) > 1:
        data = pathlib.Path(sys.argv[1])
    xml = (data / "catalog.xml").read_bytes()
    xsd = (data / "catalog.xsd").read_text(encoding="utf-8")
    schema = beresta.Schema.from_string(xsd)
    print("| Библиотека / Library | SAX + XSD, MB/s | Ошибки XSD / XSD errors | XSD valid |")
    print("|---|---|---|---|")
    beresta_errors = beresta_once(schema, xml)
    print("| Берёста / Beresta %s (validate_bytes) | %s | %d | %s |" % (
        beresta.version(), speed(beresta_once, schema, xml), beresta_errors, "yes" if beresta_errors == 0 else "no"))
    try:
        from lxml import etree
    except ImportError:
        return
    native = etree.XMLSchema(etree.fromstring(xsd.encode("utf-8")))
    lxml_errors = lxml_once(native, xml)
    print("| lxml %s (iterparse + schema) | %s | %d | %s |" % (
        ".".join(map(str, etree.LXML_VERSION[:3])), speed(lxml_once, native, xml), lxml_errors, "yes" if lxml_errors == 0 else "no"))


if __name__ == "__main__":
    main()

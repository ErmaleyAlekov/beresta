"""
Замер обёртки Python против lxml и xml.etree на общих данных
(benchmarks/wrappers/data, см. make_data.js). Каждая операция — медиана
из RUNS прогонов после одного прогревочного. Результат — строки Markdown.
    python bench.py data

A measurement of the Python wrapper against lxml and xml.etree on common data
(benchmarks/wrappers/data, see make_data.js). Every operation is the median
of RUNS runs after one warm-up run. The result is Markdown rows.
"""
import pathlib
import platform
import statistics
import sys
import time

from engine_etree import available_engines
from memory_probe import measure

RUNS = 5


def median_ms(operation, argument) -> float:
    """Медиана времени операции в миллисекундах; документ закрывается сразу.
    The median operation time in milliseconds; a document is closed at once."""
    times = []
    for run in range(RUNS + 1):
        start = time.perf_counter()
        result = operation(argument)
        elapsed = (time.perf_counter() - start) * 1000
        if hasattr(result, "close"):
            result.close()
        if run > 0:
            times.append(elapsed)
    return statistics.median(times)


def timed(operation, argument, megabytes: float = 0.0) -> str:
    """Время операции в мс или, если задан объём, скорость в МБ/с; «—» — операции нет.
    The operation time in ms or, if a size is given, the speed in MB/s; "—" means there is no operation."""
    if operation is None:
        return "—"
    milliseconds = median_ms(operation, argument)
    if megabytes > 0:
        return "%.0f" % (megabytes / milliseconds * 1000)
    return "%.1f" % milliseconds


def counted(operation, argument) -> str:
    """Результат операции для сверки библиотек; «—» — операции нет.
    The operation result for checking the libraries against each other; "—" means there is no operation."""
    if operation is None:
        return "—"
    return str(operation(argument))


def row(engine, xml: bytes) -> str:
    """Строка таблицы одной библиотеки.
    The table row of one library."""
    megabytes = len(xml) / 1e6
    cells = [engine.name, timed(engine.parse, xml, megabytes)]
    document = engine.parse(xml)
    cells += [timed(engine.walk, document), timed(engine.xpath, document), timed(engine.parse_validated, xml, megabytes)]
    cells += [counted(engine.walk, document), counted(engine.xpath, document), counted(engine.parse_validated, xml)]
    if hasattr(document, "close"):
        document.close()
    return "| " + " | ".join(cells) + " |"


def main() -> None:
    """Точка входа: путь к данным — первый аргумент.
    The entry point: the data path is the first argument."""
    data = pathlib.Path("data")
    if len(sys.argv) > 1:
        data = pathlib.Path(sys.argv[1])
    xml = (data / "catalog.xml").read_bytes()
    xsd = (data / "catalog.xsd").read_text(encoding="utf-8")
    query = (data / "query.txt").read_text(encoding="utf-8")
    if len(sys.argv) > 3 and sys.argv[2] == "--memory":
        # Режим памяти: одна библиотека в этом процессе / The memory mode: one library in this process
        print(measure(available_engines(xsd, query)[int(sys.argv[3])], xml))
        return
    print("### Python %s, %d MB\n" % (platform.python_version(), len(xml) // 1000000))
    print("| Библиотека / Library | Разбор / Parse, MB/s | Обход / Walk, ms | XPath, ms | Разбор + XSD / Parse + XSD, MB/s "
          "| Элементы / Elements | XPath: узлы / nodes | Ошибки XSD / XSD errors |")
    print("|---|---|---|---|---|---|---|---|")
    for engine in available_engines(xsd, query):
        print(row(engine, xml))


if __name__ == "__main__":
    main()

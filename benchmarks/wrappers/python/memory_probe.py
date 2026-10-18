"""
Память под дерево одной библиотеки: прирост резидентной памяти процесса
(VmRSS из /proc/self/status, Linux) после разбора и сборки мусора — и для
объектов Python, и для нативной памяти. Каждая библиотека меряется в
отдельном процессе.

The tree memory of one library: the growth of the process resident memory
(VmRSS from /proc/self/status, Linux) after parsing and garbage collection — both for
Python objects and for native memory. Every library is measured in a
separate process.
"""
import gc
import pathlib


def resident_bytes() -> int:
    """VmRSS процесса в байтах.
    The process VmRSS in bytes."""
    for line in pathlib.Path("/proc/self/status").read_text().splitlines():
        if line.startswith("VmRSS:"):
            return int(line.split()[1]) * 1024
    return 0


def measure(engine, xml: bytes) -> str:
    """Строка «имя | МБ | кратность к размеру файла».
    A "name | MB | multiple of the file size" line."""
    gc.collect()
    before = resident_bytes()
    document = engine.parse(xml)
    gc.collect()
    grown = resident_bytes() - before
    line = "| %s | %d MB (×%.1f) |" % (engine.name, grown // 1000000, grown / len(xml))
    engine.walk(document)
    if hasattr(document, "close"):
        document.close()
    return line

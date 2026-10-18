"""
Берёста — быстрая DOM-библиотека XML на C++17 для Python (через ctypes).

    import beresta

    with beresta.parse("<shop><item code='A1'>Чай</item></shop>") as document:
        item = document.root.first_child
        print(item.get_attribute("code"), item.text)
        print(beresta.XPath("count(//item)").number(document))

Без зависимостей: нужен только сам Python и динамическая библиотека,
которую сборка кладёт в beresta/_native/<os>-<arch>/.

Beresta is a fast XML DOM library in C++17 for Python (through ctypes).

No dependencies: only Python itself and the native library,
which the build puts into beresta/_native/<os>-<arch>/.
"""

from ._factory import new_document, parse, parse_file
from ._loader import platform_name
from ._native import LIB as _LIB
from .document import Document
from .element import Element
from .errors import BerestaError
from .flags import NodeType, ParseFlags, WriteFlags
from .languages import language, languages, set_language
from .node import Node
from .schema import Diagnostic, Schema
from .xpath import XPath

__all__ = [
    "BerestaError", "Diagnostic", "Document", "Element", "Node", "NodeType",
    "ParseFlags", "Schema", "WriteFlags", "XPath",
    "language", "languages", "new_document", "parse", "parse_file", "platform_name",
    "set_language", "version",
]


def version() -> str:
    """Версия нативной библиотеки, например "2.1.0". / The native library version, for example "2.1.0"."""
    return _LIB.beresta_version().decode("ascii")

"""
Стандартный xml.etree.ElementTree и список всех доступных библиотек.
The standard xml.etree.ElementTree and the list of all available libraries.
"""
import xml.etree.ElementTree as ElementTree

from engine_beresta import BerestaEngine


class ElementTreeEngine:
    """xml.etree.ElementTree: разбор на C (expat), без XPath 1.0 и без XSD.
    xml.etree.ElementTree: parsing in C (expat), without XPath 1.0 and without XSD."""

    def __init__(self, xsd: str, query: str):
        self.name = "xml.etree.ElementTree"
        self.xpath = None
        self.parse_validated = None

    def parse(self, xml: bytes):
        return ElementTree.fromstring(xml)

    def walk(self, document) -> int:
        count = 0
        for _ in document.iter():
            count += 1
        return count


def available_engines(xsd: str, query: str) -> list:
    """Все доступные библиотеки; lxml — если установлена.
    All available libraries; lxml — if it is installed."""
    result = [BerestaEngine(xsd, query)]
    try:
        from engine_lxml import LxmlEngine
        result.append(LxmlEngine(xsd, query))
    except ImportError:
        pass
    result.append(ElementTreeEngine(xsd, query))
    return result

"""
lxml — обёртка libxml2 на Cython, стандарт де-факто для XML в Python:
XPath 1.0 и XML Schema из libxml2. Необязательна: без неё строка пропускается.

lxml is a Cython wrapper of libxml2, the de facto standard for XML in Python:
XPath 1.0 and XML Schema from libxml2. Optional: without it the row is skipped.
"""
from lxml import etree


class LxmlEngine:
    """lxml: схема и запрос компилируются один раз.
    lxml: the schema and the query are compiled once."""

    def __init__(self, xsd: str, query: str):
        self.name = "lxml " + ".".join(str(part) for part in etree.LXML_VERSION[:3])
        self.query = etree.XPath(query)
        self.validating = etree.XMLParser(schema=etree.XMLSchema(etree.fromstring(xsd.encode())))
        self.plain = etree.XMLParser()

    def parse(self, xml: bytes):
        return etree.fromstring(xml, self.plain)

    def walk(self, document) -> int:
        count = 0
        for _ in document.iter():
            count += 1
        return count

    def xpath(self, document) -> int:
        return len(self.query(document))

    def parse_validated(self, xml: bytes) -> int:
        # Ошибка проверки — исключение XMLSyntaxError / A validation error is an XMLSyntaxError exception
        etree.fromstring(xml, self.validating)
        return 0

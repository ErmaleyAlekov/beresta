"""
«Берёста» через обёртку ctypes.
Beresta through the ctypes wrapper.
"""
import beresta


class BerestaEngine:
    """«Берёста»: каждый шаг обхода — вызов ctypes и объект-обёртка.
    Beresta: every walk step is a ctypes call and a wrapper object."""

    def __init__(self, xsd: str, query: str):
        self.name = "Берёста / Beresta " + beresta.version() + " (ctypes)"
        self.schema = beresta.Schema.from_string(xsd)
        self.query = beresta.XPath(query)

    def parse(self, xml: bytes):
        return beresta.parse(xml)

    def walk(self, document) -> int:
        # Обход явным стеком: first_child / next_sibling у каждого узла
        # A walk with an explicit stack: first_child / next_sibling of every node
        pending = [document]
        count = 0
        while pending:
            node = pending.pop()
            if node.type == beresta.NodeType.ELEMENT:
                count += 1
            child = node.first_child
            while child is not None:
                pending.append(child)
                child = child.next_sibling
        return count

    def xpath(self, document) -> int:
        return len(self.query.select(document))

    def parse_validated(self, xml: bytes) -> int:
        # Проверка идёт во время разбора; ошибка — исключение
        # Validation runs during parsing; an error is an exception
        with beresta.parse(xml, schema=self.schema):
            return 0

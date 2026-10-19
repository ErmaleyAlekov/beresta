"""Нагрузочная самопроверка: массовый перенос узлов и общие объекты в потоках.
A load self-test: bulk node moves and shared objects across threads."""

import unittest
from concurrent.futures import ThreadPoolExecutor

import beresta
from beresta import Schema, XPath

beresta.set_language("ru")

NODES = 50000

LIST_XSD = """<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>
<xs:element name='list'><xs:complexType><xs:sequence>
<xs:element name='v' type='xs:int' maxOccurs='unbounded'/>
</xs:sequence></xs:complexType></xs:element></xs:schema>"""


def _list_xml(count: int, broken: int) -> str:
    """<list> из count чисел; значение с номером broken — не число.
    A <list> of count numbers; the value number broken is not a number."""
    values = ["<v>%d</v>" % index for index in range(count)]
    values[broken] = "<v>x</v>"
    return "<list>" + "".join(values) + "</list>"


def _serve(request):
    """Один «запрос» потока: потоковая проверка и XPath общими объектами.
    One thread "request": streaming validation and XPath with shared objects."""
    schema, xpath, data = request
    problems = schema.validate_bytes(data)
    with beresta.parse(data) as document:
        return len(problems), xpath.string(document)


class LoadTests(unittest.TestCase):
    """Нагрузка через границу ctypes. / Load across the ctypes boundary."""

    def test_bulk_move_keeps_identity(self):
        """50 000 узлов переносятся из A в B с конца; A закрывается.
        Обёртки, выданные ДО переноса, — те же объекты (Identity Map),
        их документ — B, а XPath видит всё на новом месте.
        50,000 nodes are moved from A to B from the end; A is closed.
        The wrappers given out BEFORE the move are the same objects (Identity Map),
        their document is B, and XPath sees everything in the new place."""
        source = beresta.parse("<a>" + "".join("<n>%d</n>" % i for i in range(NODES)) + "</a>")
        target = beresta.parse("<b/>")
        wrappers = source.root.children
        for node in reversed(wrappers):
            node.move_to(target.root)
        source.close()
        self.assertIs(target.root.first_child, wrappers[-1])
        self.assertIs(target.root.last_child, wrappers[0])
        self.assertTrue(all(node.document is target for node in wrappers))
        self.assertEqual(wrappers[123].text, "123")
        self.assertEqual(XPath("count(/b/n)").number(target), NODES)
        self.assertEqual(XPath("/b/n[1]").select_values(target), [str(NODES - 1)])
        target.close()

    def test_threads_share_schema_and_xpath(self):
        """8 потоков x 40 запросов на одной схеме и одном XPath: ctypes
        отпускает GIL на время вызова, и ядро работает параллельно.
        8 threads x 40 requests on one schema and one XPath: ctypes releases
        the GIL for the call time, and the core works in parallel."""
        schema = Schema.from_string(LIST_XSD)
        xpath = XPath("concat(count(//v), ':', //v[2])")
        data = _list_xml(2000, 1500).encode("utf-8")
        with ThreadPoolExecutor(max_workers=8) as pool:
            results = list(pool.map(_serve, [(schema, xpath, data)] * 320))
        self.assertEqual(set(results), {(1, "2000:1")})

    def test_select_values_without_wrappers(self):
        """select_values отдаёт строки, а не узлы: и для элементов, и для атрибутов.
        select_values returns strings, not nodes: both for elements and for attributes."""
        with beresta.parse("<s><i c='A'>чай</i><i c='B'>кофе</i></s>") as document:
            self.assertEqual(XPath("//i").select_values(document), ["чай", "кофе"])
            self.assertEqual(XPath("//i/@c").select_values(document), ["A", "B"])
            self.assertEqual(XPath("//none").select_values(document), [])


if __name__ == "__main__":
    unittest.main()

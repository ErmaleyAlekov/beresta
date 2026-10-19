"""Самопроверка потоковой проверки по схеме и мест нарушений.
Self-test of streaming schema validation and the places of violations."""

import os
import tempfile
import unittest

import beresta
from beresta import BerestaError, ParseFlags, Schema

beresta.set_language("ru")

ORDER_XSD = """<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>
<xs:element name='order'><xs:complexType><xs:sequence>
<xs:element name='item' maxOccurs='unbounded'><xs:complexType><xs:simpleContent>
<xs:extension base='xs:string'>
<xs:attribute name='qty' type='xs:positiveInteger' use='required'/>
</xs:extension></xs:simpleContent></xs:complexType></xs:element>
</xs:sequence></xs:complexType></xs:element></xs:schema>"""

ORDER_XML = "<order>\n<item qty='2'>чай</item>\n<item qty='0'>кофе</item>\n<item>мёд</item>\n</order>"


class StreamingTests(unittest.TestCase):
    """Проверка без дерева. / Validation without a tree."""

    def test_stream_matches_tree(self):
        """Поток даёт те же нарушения, что и дерево с позициями узлов.
        The stream gives the same violations as a tree with node positions."""
        schema = Schema.from_string(ORDER_XSD)
        with beresta.parse(ORDER_XML, ParseFlags.TRACK_LOCATIONS) as document:
            tree = schema.validate(document)
        stream = schema.validate_bytes(ORDER_XML)
        self.assertEqual(len(tree), 2)
        self.assertEqual(stream, tree)
        self.assertEqual((stream[0].line, stream[0].column), (3, 1))
        self.assertTrue(stream[0].message.startswith("/order/item[2]"))
        self.assertEqual(schema.validate_bytes(b"<order><item qty='1'/></order>"), [])

    def test_stream_file(self):
        """Файл с кириллическим именем; синтаксическая ошибка — исключение.
        A file with a Cyrillic name; a syntax error is an exception."""
        schema = Schema.from_string(ORDER_XSD)
        with tempfile.TemporaryDirectory() as directory:
            path = os.path.join(directory, "выгрузка.xml")
            with open(path, "w", encoding="utf-8") as output:
                output.write(ORDER_XML)
            self.assertEqual(schema.validate_file(path), schema.validate_bytes(ORDER_XML))
            with self.assertRaises(BerestaError) as caught:
                schema.validate_file(os.path.join(directory, "нет.xml"))
            self.assertEqual(caught.exception.name, "IO_ERROR")
        with self.assertRaises(BerestaError) as caught:
            schema.validate_bytes("<order><item qty='1'></order>")
        self.assertEqual(caught.exception.name, "PARSE_ERROR")

    def test_tree_without_locations(self):
        """Без TRACK_LOCATIONS у узлов нет позиций: места нарушений — 0.
        Without TRACK_LOCATIONS nodes have no positions: the places of violations are 0."""
        schema = Schema.from_string(ORDER_XSD)
        with beresta.parse(ORDER_XML) as document:
            problems = schema.validate(document)
        self.assertEqual([(problem.line, problem.column) for problem in problems], [(0, 0), (0, 0)])


if __name__ == "__main__":
    unittest.main()

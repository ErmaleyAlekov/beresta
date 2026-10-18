"""Самопроверка Python-обёртки: разбор, навигация, изменение дерева, ошибки.
Self-test of the Python wrapper: parsing, navigation, tree changes, errors."""

import os
import tempfile
import unittest

import beresta
from beresta import BerestaError, NodeType, ParseFlags, WriteFlags


# Тексты ошибок сверяются по-русски — независимо от языка сборки
# Error texts are compared in Russian — regardless of the build language
beresta.set_language("ru")


class DocumentTests(unittest.TestCase):
    """Документы и узлы. / Documents and nodes."""

    def test_parse_and_navigate(self):
        with beresta.parse("<shop><item code='A1'>Чай</item><!--x--><item>Кофе</item></shop>") as document:
            root = document.root
            self.assertEqual(root.name, "shop")
            self.assertEqual(len(root), 3)
            self.assertEqual(root[0].get_attribute("code"), "A1")
            self.assertEqual(root[-1].text, "Кофе")
            self.assertEqual(root[1].type, NodeType.COMMENT)
            self.assertIs(root[0].next_sibling.next_sibling.parent.parent, document)
            self.assertEqual([child.name for child in root], ["item", "#comment", "item"])

    def test_build_and_serialize(self):
        with beresta.new_document("order") as document:
            item = document.root.append_element("item")
            item.set_attribute("qty", "2")
            item.append_text("Мёд & орехи")
            self.assertEqual(document.to_xml(), '<order><item qty="2">Мёд &amp; орехи</item></order>')
            self.assertIn("\n    <item", document.to_xml(WriteFlags.PRETTY))
            self.assertEqual(item.attributes, {"qty": "2"})
            self.assertTrue(item.remove_attribute("qty"))
            self.assertFalse(item.has_attribute("qty"))

    def test_copy_remove_and_clone(self):
        source = beresta.parse("<a><b x='1'/></a>")
        target = beresta.parse("<c/>")
        source.root.first_child.copy_to(target.root)
        source.root.first_child.remove()
        copy = target.clone()
        target.close()
        self.assertEqual(source.to_xml(), "<a/>")
        self.assertEqual(copy.to_xml(), '<c><b x="1"/></c>')
        self.assertTrue(target.closed)

    def test_errors(self):
        with self.assertRaises(BerestaError) as caught:
            beresta.parse("<a><b></a>")
        self.assertEqual(caught.exception.name, "PARSE_ERROR")
        self.assertTrue(caught.exception.message.startswith("строка 1"))
        document = beresta.parse("<a/>")
        with self.assertRaises(BerestaError):
            document.root.append_element("1bad")
        root = document.root
        document.close()
        with self.assertRaises(RuntimeError):
            root.text

    def test_files_and_encodings(self):
        with tempfile.TemporaryDirectory() as folder:
            path = os.path.join(folder, "документ.xml")
            with beresta.parse("<текст>Привет</текст>") as document:
                document.save(path, encoding="windows-1251")
            with open(path, "rb") as stream:
                self.assertIn(b"windows-1251", stream.read())
            with beresta.parse_file(path, ParseFlags.DISCARD_COMMENTS) as loaded:
                self.assertEqual(loaded.root.text, "Привет")


if __name__ == "__main__":
    unittest.main()

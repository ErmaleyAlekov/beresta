"""Регрессии ручного ревью обёртки. / Regressions from the binding review."""

import os
import unittest
from unittest.mock import patch

import beresta
from beresta import _loader
from beresta._wrapping import wrap_node


class ReviewBindingTests(unittest.TestCase):
    """Время жизни и строковые аргументы. / Lifetimes and string arguments."""

    def test_removed_subtree_invalidates_existing_wrappers(self):
        with beresta.parse("<root><a><b/></a></root>") as document:
            removed = document.root.first_child
            child = removed.first_child
            removed.remove()
            for node in (removed, child):
                with self.assertRaises(RuntimeError):
                    _ = node.name
                with self.assertRaises(RuntimeError):
                    _ = node.type
            replacement = document.root.append_element("new")
            self.assertIsNot(replacement, removed)
            self.assertIsNot(replacement, child)
            self.assertEqual(replacement.name, "new")

    def test_failed_remove_preserves_wrappers(self):
        with beresta.parse("<root><child/></root>") as document:
            root = document.root
            child = root.first_child
            with self.assertRaises(beresta.BerestaError):
                document.remove()
            self.assertIs(document.root, root)
            self.assertIs(root.first_child, child)
            self.assertEqual(child.name, "child")

    def test_attribute_rejects_embedded_nul(self):
        with beresta.new_document("root") as document:
            with self.assertRaises(ValueError):
                document.root.set_attribute("key", "a\x00b")
            self.assertFalse(document.root.has_attribute("key"))

    def test_closed_xpath_and_schema(self):
        xpath = beresta.XPath("string(/root)")
        schema = beresta.Schema.from_string(
            "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
            "<xs:element name='root' type='xs:string'/></xs:schema>"
        )
        with beresta.parse("<root>content</root>") as document:
            self.assertEqual(xpath.string(document), "content")
            self.assertEqual(schema.validate(document), [])
            for owned, read in ((xpath, xpath.string), (schema, schema.validate)):
                self.assertFalse(owned.closed)
                owned.close()
                owned.close()
                self.assertTrue(owned.closed)
                with self.assertRaises(RuntimeError):
                    read(document)
            with self.assertRaises(RuntimeError):
                beresta.parse("<root/>", schema=schema)

    def test_factory_rejects_closed_document(self):
        document = beresta.parse("<root/>")
        saved_handle = document.root.handle
        document.close()
        with self.assertRaises(RuntimeError):
            wrap_node(saved_handle, document)


class ReviewLoaderTests(unittest.TestCase):
    """Поиск библиотеки. / Library loading."""

    @patch.dict(os.environ, {"BERESTA_LIBRARY": "local-beresta-missing.dll"})
    @patch("ctypes.util.find_library", return_value=None)
    @patch("beresta._loader.os.path.exists", return_value=False)
    def test_missing_relative_library_is_listed(self, exists, find_library):
        expected = os.path.abspath("local-beresta-missing.dll")
        self.assertEqual(_loader._candidates()[0], expected)
        with self.assertRaises(OSError) as caught:
            _loader.load_library()
        self.assertIn(expected, str(caught.exception))

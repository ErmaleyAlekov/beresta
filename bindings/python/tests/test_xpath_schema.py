"""Самопроверка Python-обёртки: XPath и XML Schema. / Self-test of the Python wrapper: XPath and XML Schema."""

import math
import unittest

import beresta
from beresta import BerestaError, Schema, XPath

SHOP = ("<s:shop xmlns:s='urn:s'><s:item code='A'>5</s:item>"
        "<s:item code='B'>7</s:item></s:shop>")

SCHEMA = ("<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
          "<xs:element name='n' type='xs:int'/></xs:schema>")


# Тексты ошибок сверяются по-русски — независимо от языка сборки
# Error texts are compared in Russian — regardless of the build language
beresta.set_language("ru")


class XPathSchemaTests(unittest.TestCase):
    """XPath 1.0 и проверка по схеме. / XPath 1.0 and schema validation."""

    def test_xpath_values(self):
        with beresta.parse(SHOP) as document:
            expression = XPath("sum(//p:item[@code != $skip])", {"p": "urn:s"}, {"skip": "A"})
            self.assertEqual(expression.number(document), 7.0)
            self.assertTrue(XPath("count(//*) = 3").boolean(document))
            self.assertEqual(XPath("string(//*[local-name()='item'][2])").string(document), "7")
            self.assertTrue(math.isnan(XPath("number('abc')").number(document)))

    def test_xpath_select(self):
        with beresta.parse(SHOP) as document:
            items = XPath("//p:item", {"p": "urn:s"}).select(document)
            self.assertEqual([item.text for item in items], ["5", "7"])
            self.assertEqual(items[0].namespace_uri, "urn:s")
            self.assertEqual(items[0].local_name, "item")
            self.assertEqual(XPath("//@code").select(document), ["A", "B"])
            with self.assertRaises(BerestaError):
                XPath("//[")

    def test_schema(self):
        schema = Schema.from_string(SCHEMA)
        with beresta.parse("<n>\n abc</n>") as bad:
            problems = schema.validate(bad)
        self.assertEqual(len(problems), 1)
        with beresta.parse("<n>12</n>", schema=schema) as good:
            self.assertEqual(good.root.text, "12")
        with self.assertRaises(BerestaError) as caught:
            beresta.parse("<n>x</n>", schema=schema)
        self.assertEqual(caught.exception.name, "VALIDATION_ERROR")

    def test_version(self):
        self.assertEqual(beresta.version(), "2.1.2")
        self.assertRegex(beresta.platform_name(), r"^(windows|linux|macos)-")


if __name__ == "__main__":
    unittest.main()

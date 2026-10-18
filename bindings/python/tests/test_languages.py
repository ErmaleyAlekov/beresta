"""Самопроверка языков сообщений. / Self-test of message languages."""

import unittest

import beresta
from beresta import BerestaError


class LanguageTests(unittest.TestCase):
    """Смена языка во время работы. / Switching the language at run time."""

    def tearDown(self):
        beresta.set_language("ru")

    def test_available_languages(self):
        self.assertEqual(beresta.languages()[:2], ["ru", "en"])

    def test_english_messages(self):
        beresta.set_language("en")
        self.assertEqual(beresta.language(), "en")
        with self.assertRaises(BerestaError) as caught:
            beresta.parse("<a><b></a>")
        self.assertEqual(caught.exception.message, "line 1, column 7: expected </b>, found </a>")
        document = beresta.parse("<a/>")
        root = document.root
        document.close()
        with self.assertRaises(RuntimeError) as closed:
            root.text
        self.assertIn("the document is already closed", str(closed.exception))

    def test_unknown_language(self):
        with self.assertRaises(BerestaError) as caught:
            beresta.set_language("xx")
        self.assertEqual(caught.exception.name, "INVALID_ARGUMENT")
        self.assertIn("ru,en", caught.exception.message)
        self.assertEqual(beresta.language(), "ru")


if __name__ == "__main__":
    unittest.main()

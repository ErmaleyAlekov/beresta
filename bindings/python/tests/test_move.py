"""Самопроверка переноса узлов между документами (move_to).
Self-test of moving nodes between documents (move_to)."""

import unittest

import beresta
from beresta import BerestaError


# Тексты ошибок сверяются по-русски — независимо от языка сборки
# Error texts are compared in Russian — regardless of the build language
beresta.set_language("ru")


class MoveTests(unittest.TestCase):
    """Перенос без копирования. / Moving without copying."""

    def test_move_between_documents(self):
        """Узел переносится из A в B, A закрывается, узлом и его потомком
        (обёртка получена ДО переноса) пользуются в B.
        A node is moved from A to B, A is closed, the node and its descendant
        (the wrapper was obtained BEFORE the move) are used in B."""
        source = beresta.parse("<a><item n='1'><name>Мёд</name></item><rest/></a>")
        with beresta.parse("<b/>") as target:
            item = source.root.first_child
            name = item.first_child
            self.assertIs(item, source.root.first_child)
            item.move_to(target.root)
            self.assertEqual(source.to_xml(), "<a><rest/></a>")
            source.close()
            self.assertIs(item.document, target)
            self.assertIs(name.document, target)
            self.assertEqual(name.text, "Мёд")
            item.set_attribute("n", "2")
            self.assertIs(item.parent, target.root)
            self.assertEqual(target.to_xml(), '<b><item n="2"><name>Мёд</name></item></b>')

    def test_closed_target_document(self):
        """После закрытия B перенесённый узел недоступен, хотя A открыт.
        After B is closed the moved node is unavailable although A is open."""
        with beresta.parse("<a><item><name/></item></a>") as source:
            target = beresta.parse("<b/>")
            item = source.root.first_child
            name = item.first_child
            item.move_to(target.root)
            target.close()
            with self.assertRaises(RuntimeError):
                name.name
            self.assertEqual(source.to_xml(), "<a/>")

    def test_move_errors(self):
        """Запрещённые переносы: корень, документ, цикл; дерево не меняется.
        Forbidden moves: the root, a document, a cycle; the tree does not change."""
        with beresta.parse("<r><p><q/></p></r>") as document:
            p = document.root.first_child
            cases = [(document.root, p, "HIERARCHY_VIOLATION"),
                     (p, p.first_child, "HIERARCHY_VIOLATION"),
                     (document, p, "INVALID_ARGUMENT")]
            for node, parent, code in cases:
                with self.assertRaises(BerestaError) as caught:
                    node.move_to(parent)
                self.assertEqual(caught.exception.name, code)
            self.assertEqual(document.to_xml(), "<r><p><q/></p></r>")
            self.assertIs(p.document, document)


if __name__ == "__main__":
    unittest.main()

package org.beresta.selftest;

import org.beresta.BerestaException;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlElement;
import org.beresta.XmlNode;

/** Проверки переноса узлов между документами (moveTo). / Checks of moving nodes between documents (moveTo). */
final class MoveChecks
{
    private MoveChecks()
    {
    }

    /**
     * Главный сценарий: узел переносится из A в B, A закрывается, узлом и его
     * потомком (обёртка которого получена ДО переноса) пользуются в B.
     *
     * <p>The main scenario: a node is moved from A to B, A is closed, the node and its
     * descendant (whose wrapper was obtained BEFORE the move) are used in B.
     */
    static void moveBetweenDocuments()
    {
        try (XmlDocument target = Xml.parse("<b/>"))
        {
            XmlDocument source = Xml.parse("<a><item n='1'><name>Мёд</name></item><rest/></a>");
            XmlElement item = (XmlElement) source.root().firstChild();
            XmlNode name = item.firstChild();
            Expect.isTrue(item == source.root().firstChild(), "у узла одна обёртка");
            item.moveTo(target.root());
            Expect.equal("<a><rest/></a>", source.toXml(), "A после переноса");
            source.close();
            Expect.isTrue(item.document() == target, "владелец сменился");
            Expect.isTrue(name.document() == target, "владелец потомка сменился");
            Expect.equal("Мёд", name.text(), "потомок после закрытия A");
            item.setAttribute("n", "2");
            Expect.isTrue(item.parent() == target.root(), "родитель — корень B");
            Expect.equal("<b><item n=\"2\"><name>Мёд</name></item></b>", target.toXml(), "B после переноса");
        }
    }

    /**
     * Обратная сторона: после закрытия B перенесённый узел недоступен, хотя A
     * ещё открыт, — понятное исключение вместо падения JVM.
     *
     * <p>The other side: after B is closed the moved node is unavailable although A is
     * still open — a clear exception instead of a JVM crash.
     */
    static void closedTargetDocument()
    {
        try (XmlDocument source = Xml.parse("<a><item><name/></item></a>"))
        {
            XmlDocument target = Xml.parse("<b/>");
            XmlNode item = source.root().firstChild();
            XmlNode name = item.firstChild();
            item.moveTo(target.root());
            target.close();
            try
            {
                name.name();
                throw Expect.missingError("закрытый документ B");
            }
            catch (IllegalStateException expected)
            {
                Expect.equal("<a/>", source.toXml(), "A без перенесённого узла");
            }
        }
    }

    /**
     * Запрещённые переносы: корень, документ, цикл; дерево не меняется.
     * <p>Forbidden moves: the root, a document, a cycle; the tree does not change.
     */
    static void moveErrors()
    {
        try (XmlDocument document = Xml.parse("<r><p><q/></p></r>"))
        {
            XmlNode p = document.root().firstChild();
            expectCode("HIERARCHY_VIOLATION", document.root(), p, "перенос корня");
            expectCode("HIERARCHY_VIOLATION", p, p.firstChild(), "перенос в потомка");
            expectCode("INVALID_ARGUMENT", document, p, "перенос документа");
            Expect.equal("<r><p><q/></p></r>", document.toXml(), "дерево не изменилось");
            Expect.isTrue(p.document() == document, "владелец не изменился");
        }
    }

    /**
     * Перенос, который обязан завершиться ошибкой с кодом code.
     * <p>A move that must fail with the code code.
     */
    private static void expectCode(String code, XmlNode node, XmlNode parent, String what)
    {
        try
        {
            node.moveTo(parent);
            throw Expect.missingError(what);
        }
        catch (BerestaException error)
        {
            Expect.equal(code, error.codeName(), what);
        }
    }
}

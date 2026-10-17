package org.beresta.selftest;

import org.beresta.BerestaException;
import org.beresta.NodeType;
import org.beresta.WriteFlags;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlElement;
import org.beresta.XmlNode;

/** Проверки документов и узлов. */
final class DocumentChecks
{
    private DocumentChecks()
    {
    }

    /** Разбор и навигация: дети, соседи, родители, типы узлов. */
    static void parseAndNavigate()
    {
        try (XmlDocument document = Xml.parse("<shop><item code='A1'>Чай</item><!--x--><item>Кофе</item></shop>"))
        {
            XmlElement root = document.root();
            Expect.equal("shop", root.name(), "имя корня");
            Expect.equal(3L, root.childCount(), "число детей");
            Expect.equal("A1", ((XmlElement) root.firstChild()).getAttribute("code"), "атрибут");
            Expect.equal("Кофе", root.lastChild().text(), "текст");
            Expect.equal(NodeType.COMMENT, root.child(1).type(), "тип комментария");
            Expect.isTrue(root.firstChild().nextSibling().parent().parent() == document, "родитель корня — документ");
            Expect.equal(3, root.children().size(), "список детей");
        }
    }

    /** Построение документа с нуля и сериализация. */
    static void buildAndSerialize()
    {
        try (XmlDocument document = XmlDocument.create("order"))
        {
            XmlElement item = document.root().appendElement("item");
            item.setAttribute("qty", "2");
            item.appendText("Мёд & орехи");
            Expect.equal("<order><item qty=\"2\">Мёд &amp; орехи</item></order>", document.toXml(), "компактный вывод");
            Expect.isTrue(document.toXml(WriteFlags.PRETTY).contains("\n    <item"), "вывод с отступами");
            Expect.equal("2", item.attributes().get("qty"), "перебор атрибутов");
            Expect.isTrue(item.removeAttribute("qty") && !item.hasAttribute("qty"), "удаление атрибута");
        }
    }

    /** Копирование между документами, удаление, глубокая копия документа. */
    static void copyAndRemove()
    {
        try (XmlDocument source = Xml.parse("<a><b x='1'/></a>"); XmlDocument target = Xml.parse("<c/>"))
        {
            XmlNode b = source.root().firstChild();
            b.copyTo(target.root());
            b.remove();
            try (XmlDocument copy = target.copy())
            {
                Expect.equal("<a/>", source.toXml(), "после удаления");
                Expect.equal("<c><b x=\"1\"/></c>", copy.toXml(), "копия документа");
            }
        }
    }

    /** Ошибки: синтаксис, недопустимое имя, обращение к закрытому документу. */
    static void errors()
    {
        try
        {
            Xml.parse("<a><b></a>");
            throw Expect.missingError("синтаксическая ошибка");
        }
        catch (BerestaException error)
        {
            Expect.equal("PARSE_ERROR", error.codeName(), "код ошибки разбора");
            Expect.isTrue(error.detail().startsWith("строка 1"), "текст ошибки: " + error.detail());
        }
        XmlDocument document = Xml.parse("<a/>");
        XmlElement root = document.root();
        document.close();
        try
        {
            root.text();
            throw Expect.missingError("закрытый документ");
        }
        catch (IllegalStateException expected)
        {
            Expect.isTrue(document.isClosed(), "документ закрыт");
        }
    }
}

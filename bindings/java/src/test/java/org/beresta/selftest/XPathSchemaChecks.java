package org.beresta.selftest;

import java.util.List;

import org.beresta.BerestaException;
import org.beresta.Diagnostic;
import org.beresta.XPath;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlElement;
import org.beresta.XmlNode;
import org.beresta.XmlSchema;

/** Проверки XPath и XML Schema. */
final class XPathSchemaChecks
{
    private static final String SHOP = "<s:shop xmlns:s='urn:s'><s:item code='A'>5</s:item>"
        + "<s:item code='B'>7</s:item></s:shop>";

    private static final String SCHEMA = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
        + "<xs:element name='n' type='xs:int'/></xs:schema>";

    private XPathSchemaChecks()
    {
    }

    /** Префиксы, переменные, число / логическое / строка. */
    static void xpathValues()
    {
        try (XmlDocument document = Xml.parse(SHOP); XPath sum = new XPath("sum(//p:item[@code != $skip])"))
        {
            sum.bind("p", "urn:s").set("skip", "A");
            Expect.equal(7.0, sum.number(document), "sum()");
            try (XPath count = new XPath("count(//*) = 3"); XPath second = new XPath("string(//*[local-name()='item'][2])"))
            {
                Expect.isTrue(count.bool(document), "boolean()");
                Expect.equal("7", second.string(document), "string()");
            }
        }
    }

    /** Выборка узлов, значения атрибутов, пространства имён, синтаксическая ошибка. */
    static void xpathSelect()
    {
        try (XmlDocument document = Xml.parse(SHOP); XPath items = new XPath("//p:item"); XPath codes = new XPath("//@code"))
        {
            List<XmlNode> found = items.bind("p", "urn:s").selectNodes(document);
            Expect.equal(2, found.size(), "число узлов");
            Expect.equal("urn:s", ((XmlElement) found.get(0)).namespaceUri(), "URI пространства имён");
            Expect.equal("item", ((XmlElement) found.get(1)).localName(), "локальное имя");
            Expect.equal(List.of("A", "B"), codes.selectValues(document), "значения атрибутов");
        }
        try (XPath broken = new XPath("//["))
        {
            throw Expect.missingError("ошибка XPath");
        }
        catch (BerestaException error)
        {
            Expect.equal("XPATH_ERROR", error.codeName(), "код ошибки XPath");
        }
    }

    /** Проверка по схеме: список нарушений и проверка прямо при разборе. */
    static void schema()
    {
        try (XmlSchema schema = XmlSchema.fromString(SCHEMA); XmlDocument bad = Xml.parse("<n>abc</n>"))
        {
            List<Diagnostic> problems = schema.validate(bad);
            Expect.equal(1, problems.size(), "одно нарушение");
            try (XmlDocument good = Xml.parse("<n>12</n>", 0, schema))
            {
                Expect.equal("12", good.root().text(), "корректный документ");
            }
            try (XmlDocument invalid = Xml.parse("<n>x</n>", 0, schema))
            {
                throw Expect.missingError("ошибка валидации");
            }
            catch (BerestaException error)
            {
                Expect.equal("VALIDATION_ERROR", error.codeName(), "код ошибки валидации");
            }
        }
    }
}

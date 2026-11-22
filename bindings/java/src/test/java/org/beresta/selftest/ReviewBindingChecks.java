package org.beresta.selftest;

import org.beresta.BerestaException;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlNode;
import org.beresta.XmlSchema;
import org.beresta.internal.NativeStrings;

/** Регрессии ручного ревью. / Regressions from the binding review. */
final class ReviewBindingChecks
{
    private ReviewBindingChecks()
    {
    }

    /**
     * Проверка сообщения об ошибке при отрицательном коде.
     * <p>Check for error message with negative code.
     */
    static void messageNegativeCode()
    {
        try
        {
            NativeStrings.message(-1);
            throw Expect.missingError("ошибка с отрицательным кодом / error with negative code");
        }
        catch (BerestaException error)
        {
            Expect.equal("INVALID_ARGUMENT", error.codeName(), "код ошибки / error code");
        }
    }

    /**
     * Проверка форматирования сообщения с параметрами.
     * <p>Check for message formatting with parameters.
     */
    static void messageWithParameters()
    {
        String result = NativeStrings.message(6, "a{1}b", "X", "Z");
        Expect.isTrue(result.contains("a{1}b"), "содержит 'a{1}b' / contains 'a{1}b'");
        Expect.isTrue(result.contains("X"), "содержит 'X' / contains 'X'");
    }

    /**
     * Проверка закрытой схемы: validate и parse должны выбросить IllegalStateException.
     * <p>Check closed schema: validate and parse should throw IllegalStateException.
     */
    static void closedSchema()
    {
        try (XmlSchema schema = XmlSchema.fromString("<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'/>");
             XmlDocument document = Xml.parse("<root/>"))
        {
            schema.close();
            try
            {
                schema.validate(document);
                throw Expect.missingError("validate на закрытой схеме / validate on closed schema");
            }
            catch (IllegalStateException expected)
            {
                Expect.isTrue(expected.getMessage().contains(NativeStrings.message(3)), expected.getMessage());
            }
            try
            {
                Xml.parse("<root/>", 0, schema);
                throw Expect.missingError("parse на закрытой схеме / parse on closed schema");
            }
            catch (IllegalStateException expected)
            {
                Expect.isTrue(expected.getMessage().contains(NativeStrings.message(3)), expected.getMessage());
            }
        }
    }

    /**
     * Проверка закрытого узла: должен оставаться рефлексивным и не сравниваться с живым узлом.
     * <p>Check closed node: should remain reflexive and not compare equal to live node.
     */
    static void closedNodes()
    {
        try (XmlDocument document = Xml.parse("<root/>"))
        {
            XmlNode root = document.root();
            document.close();
            Expect.isTrue(root.equals(root), "узел равен самому себе / node equals itself");
            try (XmlDocument other = Xml.parse("<root/>"))
            {
                Expect.isTrue(!root.equals(other.root()) && !other.root().equals(root),
                    "закрытый узел отличается от живого / closed node differs from live node");
            }
        }
    }
}

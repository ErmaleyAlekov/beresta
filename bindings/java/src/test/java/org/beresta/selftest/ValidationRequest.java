package org.beresta.selftest;

import java.util.concurrent.Callable;

import org.beresta.XPath;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlSchema;

/**
 * Один «запрос» сервера для многопоточной проверки: потоковая проверка
 * общей схемой и XPath общим выражением по своему документу. Отдельный
 * класс вместо лямбды — так требует стиль проекта.
 *
 * <p>One server "request" for the multithreaded check: streaming validation with
 * the shared schema and XPath with the shared expression over an own document.
 * A separate class instead of a lambda — the project style requires it.
 */
final class ValidationRequest implements Callable<String>
{
    private final XmlSchema schema;
    private final XPath xpath;
    private final byte[] data;

    ValidationRequest(XmlSchema schema, XPath xpath, byte[] data)
    {
        this.schema = schema;
        this.xpath = xpath;
        this.data = data;
    }

    /** Ответ вида «число нарушений/результат XPath». / An answer of the form "violation count/XPath result". */
    @Override
    public String call()
    {
        int problems = schema.validate(data, 0).size();

        try (XmlDocument document = Xml.parse(data, 0, null))
        {
            return problems + "/" + xpath.string(document);
        }
    }
}

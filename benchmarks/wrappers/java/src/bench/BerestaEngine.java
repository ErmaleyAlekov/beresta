package bench;

import java.util.ArrayDeque;

import org.beresta.Beresta;
import org.beresta.NodeType;
import org.beresta.ParseFlags;
import org.beresta.XPath;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlNode;
import org.beresta.XmlSchema;

/**
 * «Берёста» через обёртку Project Panama: каждый шаг обхода и каждый узел
 * результата XPath — вызов нативной функции и объект-обёртка Java.
 *
 * Beresta through the Project Panama wrapper: every walk step and every node
 * of an XPath result is a native function call and a Java wrapper object.
 */
public final class BerestaEngine extends Engine
{
    private final XmlSchema schema;
    private final XPath query;

    /** Схема и запрос готовятся один раз. / The schema and the query are prepared once. */
    public BerestaEngine(String xsd, String expression)
    {
        super("Берёста / Beresta " + Beresta.version() + " (Panama)");
        schema = XmlSchema.fromString(xsd);
        query = new XPath(expression);
    }

    @Override
    public Object parse(byte[] xml)
    {
        return Xml.parse(xml, ParseFlags.DEFAULT, null);
    }

    /** Обход явным стеком: firstChild / nextSibling у каждого узла. / A walk with an explicit stack: firstChild / nextSibling of every node. */
    @Override
    public int walk(Object document)
    {
        ArrayDeque<XmlNode> pending = new ArrayDeque<>();
        int count = 0;

        pending.push((XmlNode) document);
        while (!pending.isEmpty())
        {
            XmlNode node = pending.pop();
            if (node.type() == NodeType.ELEMENT)
                ++count;
            for (XmlNode child = node.firstChild(); child != null; child = child.nextSibling())
                pending.push(child);
        }
        return count;
    }

    @Override
    public int xpath(Object document)
    {
        return query.selectNodes((XmlNode) document).size();
    }

    /** Проверка идёт во время разбора; ошибка проверки — исключение. / Validation runs during parsing; a validation error is an exception. */
    @Override
    public int parseValidated(byte[] xml)
    {
        try (XmlDocument document = Xml.parse(xml, ParseFlags.DEFAULT, schema))
        {
            return 0;
        }
    }
}

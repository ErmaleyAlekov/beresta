package bench;

import java.io.ByteArrayInputStream;
import java.io.StringReader;
import java.util.ArrayDeque;

import javax.xml.XMLConstants;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.SchemaFactory;
import javax.xml.xpath.XPathConstants;
import javax.xml.xpath.XPathExpression;
import javax.xml.xpath.XPathFactory;

import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Стандартный стек XML из JDK (javax.xml: встроенный Xerces-J, XPath и
 * javax.xml.validation) — то, что Java-разработчик берёт по умолчанию.
 * Построители создаются один раз и переиспользуются.
 *
 * The standard XML stack of the JDK (javax.xml: the built-in Xerces-J, XPath and
 * javax.xml.validation) — what a Java developer takes by default.
 * The builders are created once and reused.
 */
public final class JdkEngine extends Engine
{
    private final DocumentBuilder plain;
    private final DocumentBuilder validating;
    private final CountingErrorHandler errors = new CountingErrorHandler();
    private final XPathExpression query;

    /** Два построителя (без схемы и со схемой) и скомпилированный запрос. / Two builders (without and with a schema) and a compiled query. */
    public JdkEngine(String xsd, String expression) throws Exception
    {
        super("JDK " + System.getProperty("java.version") + " DOM (javax.xml)");
        DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
        factory.setNamespaceAware(true);
        plain = factory.newDocumentBuilder();
        factory.setSchema(SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI)
            .newSchema(new StreamSource(new StringReader(xsd))));
        validating = factory.newDocumentBuilder();
        validating.setErrorHandler(errors);
        query = XPathFactory.newInstance().newXPath().compile(expression);
    }

    @Override
    public Object parse(byte[] xml) throws Exception
    {
        return plain.parse(new ByteArrayInputStream(xml));
    }

    /** Тот же обход явным стеком, что и у «Берёсты». / The same explicit-stack walk as for Beresta. */
    @Override
    public int walk(Object document)
    {
        ArrayDeque<Node> pending = new ArrayDeque<>();
        int count = 0;

        pending.push((Node) document);
        while (!pending.isEmpty())
        {
            Node node = pending.pop();
            if (node.getNodeType() == Node.ELEMENT_NODE)
                ++count;
            for (Node child = node.getFirstChild(); child != null; child = child.getNextSibling())
                pending.push(child);
        }
        return count;
    }

    @Override
    public int xpath(Object document) throws Exception
    {
        return ((NodeList) query.evaluate(document, XPathConstants.NODESET)).getLength();
    }

    @Override
    public int parseValidated(byte[] xml) throws Exception
    {
        errors.reset();
        validating.parse(new ByteArrayInputStream(xml));
        return errors.count();
    }
}

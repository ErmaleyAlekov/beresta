package org.beresta.selftest;

import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.beresta.XPath;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlNode;
import org.beresta.XmlSchema;

/** Нагрузочные проверки через границу Panama. / Load checks across the Panama boundary. */
final class LoadChecks
{
    private static final int NODES = 50_000;

    private LoadChecks()
    {
    }

    /**
     * 50 000 узлов переносятся из A в B с конца, A закрывается. Обёртки,
     * выданные ДО переноса, — те же объекты (реестр обёрток), их документ —
     * B, а XPath видит всё на новом месте.
     *
     * <p>50,000 nodes are moved from A to B from the end, A is closed. The wrappers
     * given out BEFORE the move are the same objects (the wrapper registry), their
     * document is B, and XPath sees everything in the new place.
     */
    static void bulkMove()
    {
        StringBuilder xml = new StringBuilder("<a>");
        for (int index = 0; index < NODES; ++index)
            xml.append("<n>").append(index).append("</n>");
        XmlDocument source = Xml.parse(xml.append("</a>").toString());
        try (XmlDocument target = Xml.parse("<b/>"); XPath count = new XPath("count(/b/n)"))
        {
            List<XmlNode> wrappers = source.root().children();
            for (int index = NODES - 1; index >= 0; --index)
                wrappers.get(index).moveTo(target.root());
            source.close();
            Expect.isTrue(target.root().firstChild() == wrappers.get(NODES - 1), "та же обёртка первого");
            Expect.isTrue(wrappers.get(123).document() == target, "владелец сменился");
            Expect.equal("123", wrappers.get(123).text(), "текст после закрытия A");
            Expect.equal((double) NODES, count.number(target), "XPath видит все узлы");
        }
    }

    /**
     * 8 потоков x 40 запросов на одной схеме и одном XPath: ответы совпадают.
     * <p>8 threads x 40 requests on one schema and one XPath: the answers match.
     */
    static void parallelShared() throws Exception
    {
        StringBuilder xml = new StringBuilder("<order>");
        for (int index = 1; index <= 2000; ++index)
            xml.append("<item qty='").append(index % 1500).append("'>").append(index).append("</item>");
        byte[] data = xml.append("</order>").toString().getBytes(StandardCharsets.UTF_8);
        ExecutorService pool = Executors.newFixedThreadPool(8);
        try (XmlSchema schema = XmlSchema.fromString(StreamingChecks.ORDER_XSD);
            XPath xpath = new XPath("concat(count(//item), ':', //item[2])"))
        {
            List<Future<String>> answers = new ArrayList<>();
            for (int request = 0; request < 320; ++request)
                answers.add(pool.submit(new ValidationRequest(schema, xpath, data)));
            Set<String> distinct = new HashSet<>();
            for (Future<String> answer : answers)
                distinct.add(answer.get());
            Expect.equal(Set.of("1/2000:2"), distinct, "одинаковые ответы");
        }
        finally
        {
            pool.shutdown();
        }
    }
}

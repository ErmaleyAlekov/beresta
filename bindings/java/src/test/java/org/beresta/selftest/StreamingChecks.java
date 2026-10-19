package org.beresta.selftest;

import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import org.beresta.BerestaException;
import org.beresta.Diagnostic;
import org.beresta.ParseFlags;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlSchema;

/** Проверки потоковой проверки по схеме. / Checks of streaming schema validation. */
final class StreamingChecks
{
    static final String ORDER_XSD = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
        + "<xs:element name='order'><xs:complexType><xs:sequence>"
        + "<xs:element name='item' maxOccurs='unbounded'><xs:complexType><xs:simpleContent>"
        + "<xs:extension base='xs:string'><xs:attribute name='qty' type='xs:positiveInteger' use='required'/>"
        + "</xs:extension></xs:simpleContent></xs:complexType></xs:element>"
        + "</xs:sequence></xs:complexType></xs:element></xs:schema>";

    static final String ORDER_XML = "<order>\n<item qty='2'>чай</item>\n<item qty='0'>кофе</item>\n<item>мёд</item>\n</order>";

    private StreamingChecks()
    {
    }

    /**
     * Поток без дерева даёт те же нарушения, что и дерево с позициями узлов
     * (TRACK_LOCATIONS); без флага у дерева места нарушений — 0.
     *
     * <p>A stream without a tree gives the same violations as a tree with node
     * positions (TRACK_LOCATIONS); without the flag the tree's violation places are 0.
     */
    static void streamMatchesTree()
    {
        byte[] data = ORDER_XML.getBytes(StandardCharsets.UTF_8);

        try (XmlSchema schema = XmlSchema.fromString(ORDER_XSD);
            XmlDocument located = Xml.parse(ORDER_XML, ParseFlags.TRACK_LOCATIONS, null);
            XmlDocument plain = Xml.parse(ORDER_XML))
        {
            List<Diagnostic> stream = schema.validate(data, ParseFlags.DEFAULT);
            Expect.equal(schema.validate(located), stream, "поток = дерево");
            Expect.equal(2, stream.size(), "число нарушений");
            Expect.equal(3L, stream.get(0).line(), "строка нарушения");
            Expect.equal(0L, schema.validate(plain).get(0).line(), "без позиций — 0");
        }
    }

    /**
     * Файл с кириллическим именем, отсутствующий файл и синтаксическая ошибка.
     * <p>A file with a Cyrillic name, a missing file and a syntax error.
     */
    static void streamFile() throws Exception
    {
        Path directory = Files.createTempDirectory("beresta_stream");
        Path file = directory.resolve("выгрузка.xml");

        Files.writeString(file, ORDER_XML, StandardCharsets.UTF_8);
        try (XmlSchema schema = XmlSchema.fromString(ORDER_XSD))
        {
            Expect.equal(schema.validate(ORDER_XML.getBytes(StandardCharsets.UTF_8), 0), schema.validateFile(file),
                "файл = байты");
            expectCode(schema, directory.resolve("нет.xml"), "IO_ERROR");
            expectCode(schema, null, "PARSE_ERROR");
        }
        finally
        {
            Files.deleteIfExists(file);
            Files.deleteIfExists(directory);
        }
    }

    /**
     * Ошибка вызова: файл path или (null) заведомо битые байты.
     * <p>A call error: the file path or (null) deliberately broken bytes.
     */
    private static void expectCode(XmlSchema schema, Path path, String code)
    {
        try
        {
            if (path != null)
                schema.validateFile(path);
            else
                schema.validate("<order><item qty='1'></order>".getBytes(StandardCharsets.UTF_8), 0);
            throw Expect.missingError(code);
        }
        catch (BerestaException error)
        {
            Expect.equal(code, error.codeName(), "код ошибки");
        }
    }
}

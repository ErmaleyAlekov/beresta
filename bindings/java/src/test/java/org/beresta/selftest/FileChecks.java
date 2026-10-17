package org.beresta.selftest;

import java.nio.file.Files;
import java.nio.file.Path;

import org.beresta.WriteFlags;
import org.beresta.Xml;
import org.beresta.XmlDocument;

/** Проверки работы с файлами и кодировками. */
final class FileChecks
{
    private FileChecks()
    {
    }

    /** Запись в файл в другой кодировке и обратный разбор. */
    static void filesAndEncodings() throws Exception
    {
        Path folder = Files.createTempDirectory("beresta-java-");
        Path file = folder.resolve("документ.xml");

        try (XmlDocument document = Xml.parse("<текст>Привет</текст>"))
        {
            document.save(file, WriteFlags.DEFAULT, "windows-1251");
        }
        Expect.isTrue(new String(Files.readAllBytes(file), "ISO-8859-1").contains("windows-1251"), "декларация кодировки");
        try (XmlDocument loaded = Xml.parseFile(file, 0, null))
        {
            Expect.equal("Привет", loaded.root().text(), "текст после перекодирования");
        }
        Files.delete(file);
        Files.delete(folder);
    }
}

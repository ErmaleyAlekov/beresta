#include "suites/test_suites.hpp"
#include "support/dom_helpers.hpp"

#include "beresta.hpp"
#include "io/mapped_file.hpp"

#include <filesystem>
#include <string>

namespace
{
    /*
    ** Обычный файл отображается (кроме WebAssembly, где файловой системы
    ** нет): байты совпадают побайтно, включая NUL и кириллицу, а имя файла
    ** может быть кириллическим. Пустой файл отобразить нельзя — работает
    ** запасной путь с буфером; отсутствующий файл — DomException(IO_ERROR).
    **
    ** An ordinary file is mapped (except in WebAssembly, where there is no file
    ** system): the bytes match byte for byte, including NUL and Cyrillic, and the
    ** file name may be Cyrillic. An empty file cannot be mapped — the fallback
    ** path with a buffer works; a missing file is a DomException(IO_ERROR).
    */
    void testMappedFileBytes()
    {
        std::filesystem::path directory = support::makeTestDirectory("mapped_file");
        std::filesystem::path file = directory / std::filesystem::u8path("данные.bin");
        const char raw[] = "<a>берёста</a>\0\x01\xff";
        std::string content(raw, sizeof(raw) - 1);

        support::writeTestFile(file, content);
        support::writeTestFile(directory / "empty.xml", "");
        beresta::io::MappedFile mapped(file);
        TEST_ASSERT(mapped.getBytes() == content);
#if !defined(__EMSCRIPTEN__)
        TEST_ASSERT(mapped.isMapped());
#endif
        beresta::io::MappedFile empty(directory / "empty.xml");
        TEST_ASSERT(!empty.isMapped());
        TEST_ASSERT(empty.getBytes().empty());
        TEST_ASSERT_THROWS(beresta::io::MappedFile(directory / "missing.xml"), beresta::DomException);
    }

    /*
    ** Разбор файла через отображение даёт то же дерево, что и разбор
    ** строки: и для UTF-8 с LF (байты разбираются прямо из отображения),
    ** и для CRLF в windows-1251 (перекодировка и нормализация делают копию).
    **
    ** Parsing a file through the mapping gives the same tree as parsing a
    ** string: both for UTF-8 with LF (the bytes are parsed right from the mapping)
    ** and for CRLF in windows-1251 (transcoding and normalization make a copy).
    */
    void testParseMappedFile()
    {
        std::filesystem::path directory = support::makeTestDirectory("mapped_parse");
        std::string utf8 = "<?xml version='1.0'?>\n<shop>\n<item code='A1'>Чай</item>\n</shop>";
        std::string cp1251 = "<?xml version='1.0' encoding='windows-1251'?>\r\n<shop>\r\n<item code='A1'>\xD7\xE0\xE9</item>\r\n</shop>";

        support::writeTestFile(directory / "utf8.xml", utf8);
        support::writeTestFile(directory / "cp1251.xml", cp1251);
        beresta::Document fromString = beresta::XmlParser().parse(utf8);
        TEST_ASSERT_EQ(beresta::XmlParser().parseFile(directory / "utf8.xml").toString(), fromString.toString());
        TEST_ASSERT_EQ(beresta::XmlParser().parseFile(directory / "cp1251.xml").getRoot()->toString(),
            fromString.getRoot()->toString());
    }

    /*
    ** SAX по файлу из отображения: события те же, что у разбора строки
    ** (сравнивается текст всех элементов), и проверка по схеме без дерева
    ** находит нарушение в файле.
    **
    ** SAX over a file from the mapping: the events are the same as for parsing a
    ** string (the text of all elements is compared), and schema validation without
    ** a tree finds a violation in the file.
    */
    void testSaxMappedFileWithSchema()
    {
        std::filesystem::path directory = support::makeTestDirectory("mapped_sax");
        const std::string xsd = "<xs:schema xmlns:xs='http://www.w3.org/2001/XMLSchema'>"
            "<xs:element name='list'><xs:complexType><xs:sequence>"
            "<xs:element name='v' type='xs:int' maxOccurs='unbounded'/></xs:sequence></xs:complexType></xs:element></xs:schema>";
        std::string xml = "<list>";
        beresta::CollectingErrorHandler errors;
        beresta::ParserOptions options;
        beresta::ContentHandler silent;

        for (int index = 0; index < 20000; ++index)
            xml += "<v>" + std::to_string(index) + "</v>";
        xml += "<v>x</v></list>";
        support::writeTestFile(directory / "list.xml", xml);
        options.schema = beresta::xsd::Schema::fromString(xsd);
        options.errorHandler = &errors;
        beresta::SaxParser(options).parseFile(directory / "list.xml", silent);
        TEST_ASSERT_EQ(std::to_string(errors.getDiagnostics().size()), "1");
        TEST_ASSERT(errors.getDiagnostics().front().message.find("/list/v[20001]") != std::string::npos);
    }
}

namespace suites
{
    // Регистрация тестов отображения файлов в память
    // Registers the tests of mapping files into memory
    void registerMappedFileTests(test::TestRunner& runner)
    {
        runner.add("Файлы: отображение в память и запасное чтение", testMappedFileBytes);
        runner.add("Файлы: разбор отображённого файла", testParseMappedFile);
        runner.add("Файлы: SAX и XSD по отображённому файлу", testSaxMappedFileWithSchema);
    }
}

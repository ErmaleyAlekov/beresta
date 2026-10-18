#ifndef XML_SERIALIZER_HPP
#define XML_SERIALIZER_HPP

#include "core/container_node.hpp"
#include "core/document.hpp"
#include "core/element.hpp"
#include "core/node.hpp"

#include <cstddef>
#include <filesystem>
#include <iosfwd>
#include <string>
#include <string_view>
#include <vector>

namespace beresta
{
    /*
    ** Настройки вывода (аналог DOMLSSerializer + DOMConfiguration в Xerces).
    **
    ** prettyPrint по умолчанию ВЫКЛЮЧЕН: вывод монолитный, без единого
    ** лишнего пробела. Включённый prettyPrint расставляет отступы только
    ** там, где они не меняют данные: элементы с текстовым (смешанным)
    ** содержимым выводятся дословно, в одну строку. Поэтому повторный
    ** разбор красивого вывода даёт то же дерево.
    **
    ** encoding — кодировка результата. XML-декларация с правильным
    ** encoding пишется для документов, у которых она была, а также всегда,
    ** когда кодировка не UTF-8 (иначе читатель не узнает кодировку).
    **
    ** Output settings (the analog of DOMLSSerializer + DOMConfiguration in Xerces).
    ** prettyPrint is OFF by default: the output is monolithic, without a single
    ** extra space. Enabled prettyPrint adds indentation only where it does not
    ** change the data: elements with text (mixed) content are written verbatim,
    ** on one line. So parsing the pretty output again gives the same tree.
    ** encoding is the result encoding. An XML declaration with the correct
    ** encoding is written for documents that had one, and also always
    ** when the encoding is not UTF-8 (otherwise the reader would not know the encoding).
    */
    struct SerializerOptions
    {
        bool prettyPrint = false;
        std::string indent = "    ";
        std::string newline = "\n";
        std::string encoding = "UTF-8";
        bool writeDeclaration = true;
        bool forceDeclaration = false;
    };

    /*
    ** XmlSerializer — запись дерева в строку, поток или файл.
    ** Результат — байты в выбранной кодировке (std::string как контейнер
    ** байтов). Символы, которых нет в кодировке, записываются как &#N;.
    **
    ** XmlSerializer — writes a tree to a string, a stream or a file.
    ** The result is bytes in the chosen encoding (std::string as a byte
    ** container). Characters missing from the encoding are written as &#N;.
    */
    class XmlSerializer
    {
    public:
        explicit XmlSerializer(SerializerOptions options = SerializerOptions());

        // Байты XML в выбранной кодировке (для UTF-8 — обычная строка)
        // XML bytes in the chosen encoding (for UTF-8 — an ordinary string)
        std::string toString(const Node& node) const;

        // Запись в поток (поток должен быть открыт в двоичном режиме)
        // Writes to a stream (the stream must be opened in binary mode)
        void write(const Node& node, std::ostream& stream) const;

        // Перезапись файла
        // Overwrites a file
        void writeFile(const Node& node, const std::filesystem::path& path) const;

        const SerializerOptions& getOptions() const noexcept;

    private:
        // Открытый элемент красивого вывода: следующий ребёнок и глубина отступа
        // An open element of pretty output: the next child and the indentation depth
        struct PrettyFrame
        {
            const Element* element;
            const Node* next;
            std::size_t depth;
        };

        // ---- Текст в UTF-8 (xml_serializer_pretty.cpp) ----
        // ---- Text in UTF-8 (xml_serializer_pretty.cpp) ----
        std::string serializeUtf8(const Node& node) const;
        void writeNode(const Node& node, std::size_t depth, std::string& output) const;
        void writePrettyElement(const Element& element, std::size_t depth,
            std::string& output) const;
        void openPrettyElement(const Element& element, std::size_t depth, std::string& output,
            std::vector<PrettyFrame>& stack) const;
        void writePrettyDocument(const Document& document, std::string& output) const;

        // ---- Декларация и кодирование (xml_serializer_output.cpp) ----
        // ---- The declaration and encoding (xml_serializer_output.cpp) ----
        void writeDeclaration(const Node& node, std::string& output) const;
        std::string encode(std::string_view utf8) const;
        void writeIndent(std::size_t depth, std::string& output) const;

        SerializerOptions _options;
    };

    // Есть ли среди детей текст или CDATA (смешанное содержимое выводится дословно)
    // Whether there is text or CDATA among the children (mixed content is written verbatim)
    bool hasTextualChild(const ContainerNode& container);
}

#endif

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
    */
    class XmlSerializer
    {
    public:
        explicit XmlSerializer(SerializerOptions options = SerializerOptions());

        // Байты XML в выбранной кодировке (для UTF-8 — обычная строка)
        std::string toString(const Node& node) const;

        // Запись в поток (поток должен быть открыт в двоичном режиме)
        void write(const Node& node, std::ostream& stream) const;

        // Перезапись файла
        void writeFile(const Node& node, const std::filesystem::path& path) const;

        const SerializerOptions& getOptions() const noexcept;

    private:
        // ---- Текст в UTF-8 (xml_serializer_pretty.cpp) ----
        std::string serializeUtf8(const Node& node) const;
        void writeNode(const Node& node, std::size_t depth, std::string& output) const;
        void writePrettyElement(const Element& element, std::size_t depth,
            std::string& output) const;
        void writeIndent(std::size_t depth, std::string& output) const;
        void writePrettyDocument(const Document& document, std::string& output) const;

        // ---- Декларация и кодирование (xml_serializer_output.cpp) ----
        void writeDeclaration(const Node& node, std::string& output) const;
        std::string encode(std::string_view utf8) const;

        SerializerOptions _options;
    };

    // Есть ли среди детей текст или CDATA (смешанное содержимое выводится дословно)
    bool hasTextualChild(const ContainerNode& container);
}

#endif

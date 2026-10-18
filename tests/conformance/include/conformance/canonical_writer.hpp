#ifndef CONFORMANCE_CANONICAL_WRITER_HPP
#define CONFORMANCE_CANONICAL_WRITER_HPP

#include <string>
#include <string_view>

namespace beresta
{
    class Document;
    class Element;
    class Node;
}

namespace conformance
{
    /*
    ** Каноническая форма Джеймса Кларка — формат файлов OUTPUT в xmlconf:
    **   - без XML-декларации, DOCTYPE и комментариев;
    **   - атрибуты отсортированы по имени, значения в двойных кавычках;
    **   - пустой элемент всегда как <a></a>;
    **   - в тексте и атрибутах экранируются & < > " TAB LF CR
    **     (как &amp; &lt; &gt; &quot; &#9; &#10; &#13;);
    **   - PI выводится как <?цель данные?> с ровно одним пробелом.
    ** Вторая каноническая форма (с <!NOTATION> в DOCTYPE) сравнивается
    ** только по телу документа: модель DTD не хранит идентификаторы нотаций.
    **
    ** James Clark's canonical form — the format of the OUTPUT files in xmlconf:
    **  - without the XML declaration, DOCTYPE and comments;
    **  - attributes sorted by name, values in double quotes;
    **  - an empty element always as <a></a>;
    **  - in text and attributes & < > " TAB LF CR are escaped
    **    (as &amp; &lt; &gt; &quot; &#9; &#10; &#13;);
    **  - a PI is written as <?target data?> with exactly one space.
    ** The second canonical form (with <!NOTATION> in the DOCTYPE) is compared
    ** only by the document body: the DTD model does not keep notation identifiers.
    */
    std::string writeCanonical(const beresta::Document& document);

    namespace detail
    {
        // Дописывает узел (элемент — вместе со всем поддеревом)
        // Appends a node (an element — together with its whole subtree)
        void writeCanonicalNode(const beresta::Node& node, std::string& output);

        // Открывающий тег с отсортированными атрибутами
        // The start tag with sorted attributes
        void writeCanonicalStartTag(const beresta::Element& element, std::string& output);

        // Экранирует символьные данные по правилам канонической формы
        // Escapes character data by the rules of the canonical form
        void appendCanonicalEscaped(std::string_view text, std::string& output);

        // Убирает из ожидаемого вывода блок <!DOCTYPE ... ]> (вторая форма); dropDtdPis — и PI перед ним
        // Removes the <!DOCTYPE ... ]> block from the expected output (the second form); dropDtdPis — also the PIs before it
        std::string stripCanonicalDoctype(const std::string& expected, bool dropDtdPis);
    }
}

#endif

#include "serializer/xml_serializer.hpp"
#include "i18n/i18n.hpp"
#include "encoding/charset.hpp"
#include "exception/dom_exception.hpp"

namespace beresta
{
    // Поиск выбранной кодировки (неизвестная — ошибка настройки)
    // Looks up the chosen encoding (an unknown one is a settings error)
    static const encoding::Charset& requireCharset(const std::string& name)
    {
        const encoding::Charset* charset = encoding::findCharset(name);

        if (charset == nullptr)
            throw DomException(DomErrorCode::ENCODING_ERROR,
                i18n::format(i18n::Message::OUTPUT_ENCODING_UNSUPPORTED, {name}));
        return *charset;
    }

    /*
    ** Декларация пишется, если: у документа она была, её явно потребовали,
    ** или кодировка вывода не UTF-8. Атрибут encoding всегда соответствует
    ** фактической кодировке вывода, а не исходной кодировке документа.
    **
    ** The declaration is written if the document had one, if it was explicitly requested,
    ** or if the output encoding is not UTF-8. The encoding attribute always matches
    ** the actual output encoding, not the original encoding of the document.
    */
    void XmlSerializer::writeDeclaration(const Node& node, std::string& output) const
    {
        const encoding::Charset& charset = requireCharset(_options.encoding);
        bool isUtf8 = &charset == &encoding::utf8Charset();
        XmlDeclaration declaration;
        bool present = false;

        if (!_options.writeDeclaration)
            return;
        if (node.getType() == NodeType::DOCUMENT)
        {
            const std::optional<XmlDeclaration>& own = static_cast<const Document&>(node).getDeclaration();
            present = own.has_value();
            if (present)
                declaration = own.value();
        }
        if (!present && !_options.forceDeclaration && isUtf8)
            return;
        declaration.encoding = std::string(charset.getName());
        serializeXmlDeclaration(declaration, output);
        if (_options.prettyPrint && node.getType() != NodeType::DOCUMENT)
            output += _options.newline;
    }

    // BOM (если нужен кодировке) и перекодирование; непредставимые символы — &#N;
    // The BOM (if the encoding needs one) and transcoding; unrepresentable characters — &#N;
    std::string XmlSerializer::encode(std::string_view utf8) const
    {
        const encoding::Charset& charset = requireCharset(_options.encoding);
        std::string bytes(charset.getByteOrderMark());

        if (charset.encode(utf8, bytes, true) != encoding::CHARSET_OK)
            throw DomException(DomErrorCode::ENCODING_ERROR,
                i18n::format(i18n::Message::INTERNAL_INVALID_UTF8));
        return bytes;
    }

    // Текст или CDATA среди непосредственных детей
    // Text or CDATA among the direct children
    bool hasTextualChild(const ContainerNode& container)
    {
        for (const Node* child = container.getFirstChild(); child != nullptr;
            child = child->getNextSibling())
        {
            NodeType type = child->getType();
            if (type == NodeType::TEXT || type == NodeType::CDATA_SECTION)
                return true;
        }
        return false;
    }

    // Отступ — повторение строки indent depth раз
    // The indentation is the indent string repeated depth times
    void XmlSerializer::writeIndent(std::size_t depth, std::string& output) const
    {
        for (std::size_t level = 0; level < depth; ++level)
            output += _options.indent;
    }
}

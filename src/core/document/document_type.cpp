#include "core/document_type.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/xml_chars.hpp"
#include "utils/xml_escape.hpp"

#include <utility>

namespace beresta
{
    /*
    ** Имя корня обязано быть корректным XML-именем. Системный идентификатор
    ** не может одновременно содержать оба вида кавычек — его нельзя было бы
    ** записать в XML. Публичный идентификатор без системного запрещён синтаксисом.
    **
    ** The root name must be a valid XML name. The system identifier
    ** cannot contain both kinds of quotes at once — it could not be
    ** written in XML. A public identifier without a system one is forbidden by the syntax.
    */
    DocumentType::DocumentType(std::string name, std::string publicId,
        std::string systemId, std::string internalSubset)
        : Node(), _name(std::move(name)), _publicId(std::move(publicId)),
          _systemId(std::move(systemId)), _internalSubset(std::move(internalSubset)), _model()
    {
        if (!utils::isValidName(_name))
            throw DomException(DomErrorCode::INVALID_NAME,
                i18n::format(i18n::Message::DOCTYPE_NAME_INVALID, {_name}));
        if (utils::chooseQuote(_systemId) == '\0' || utils::chooseQuote(_publicId) == '\0')
            throw DomException(DomErrorCode::INVALID_CONTENT,
                i18n::format(i18n::Message::DOCTYPE_ID_BOTH_QUOTES));
        if (!_publicId.empty() && _systemId.empty())
            throw DomException(DomErrorCode::INVALID_CONTENT,
                i18n::format(i18n::Message::DOCTYPE_PUBLIC_WITHOUT_SYSTEM));
    }

    // Тип узла — объявление типа документа
    // The node type is a document type declaration
    NodeType DocumentType::getType() const noexcept
    {
        return NodeType::DOCUMENT_TYPE;
    }

    // Автономная копия (модель DTD разделяется — она неизменяема)
    // A standalone copy (the DTD model is shared — it is immutable)
    std::unique_ptr<Node> DocumentType::clone() const
    {
        return std::make_unique<DocumentType>(*this);
    }

    /*
    ** Минимальная форма без лишних пробелов: пробел между частями ставится
    ** только там, где он обязателен по грамматике. Перед '[' пробел
    ** необязателен (S?), поэтому не выводится.
    **
    ** The minimal form without extra spaces: a space between the parts is written
    ** only where the grammar requires it. A space before '[' is
    ** optional (S?), so it is not written.
    */
    void DocumentType::serialize(std::string& output) const
    {
        output += "<!DOCTYPE ";
        output += _name;
        if (!_publicId.empty())
        {
            output += " PUBLIC ";
            utils::appendQuoted(output, _publicId);
        }
        else if (!_systemId.empty())
            output += " SYSTEM";
        if (!_systemId.empty())
        {
            output += ' ';
            utils::appendQuoted(output, _systemId);
        }
        if (!_internalSubset.empty())
            output += "[" + _internalSubset + "]";
        output += '>';
    }

    // У DOCTYPE нет текстового содержимого (в W3C DOM — null)
    // A DOCTYPE has no text content (null in W3C DOM)
    std::string DocumentType::getTextContent() const
    {
        return std::string();
    }
}

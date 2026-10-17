#include "core/document_type.hpp"
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
    */
    DocumentType::DocumentType(std::string name, std::string publicId,
        std::string systemId, std::string internalSubset)
        : Node(), _name(std::move(name)), _publicId(std::move(publicId)),
          _systemId(std::move(systemId)), _internalSubset(std::move(internalSubset)), _model()
    {
        if (!utils::isValidName(_name))
            throw DomException(DomErrorCode::INVALID_NAME,
                "недопустимое имя в DOCTYPE: \"" + _name + "\"");
        if (utils::chooseQuote(_systemId) == '\0' || utils::chooseQuote(_publicId) == '\0')
            throw DomException(DomErrorCode::INVALID_CONTENT,
                "идентификатор DOCTYPE не может содержать оба вида кавычек");
        if (!_publicId.empty() && _systemId.empty())
            throw DomException(DomErrorCode::INVALID_CONTENT,
                "PUBLIC-идентификатор DOCTYPE требует системного идентификатора");
    }

    // Тип узла — объявление типа документа
    NodeType DocumentType::getType() const noexcept
    {
        return NodeType::DOCUMENT_TYPE;
    }

    // Автономная копия (модель DTD разделяется — она неизменяема)
    std::unique_ptr<Node> DocumentType::clone() const
    {
        return std::make_unique<DocumentType>(*this);
    }

    /*
    ** Минимальная форма без лишних пробелов: пробел между частями ставится
    ** только там, где он обязателен по грамматике. Перед '[' пробел
    ** необязателен (S?), поэтому не выводится.
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
    std::string DocumentType::getTextContent() const
    {
        return std::string();
    }
}

#include "parser/internal/attribute_value_decoder.hpp"
#include "i18n/i18n.hpp"
#include "parser/internal/reference_decoding.hpp"
#include "exception/dom_exception.hpp"
#include "utils/string_utils.hpp"

namespace beresta::detail
{
    // Модель может отсутствовать (документ без DOCTYPE): тогда известны только 5 сущностей
    // The model may be missing (a document without a DOCTYPE): then only the 5 entities are known
    AttributeValueDecoder::AttributeValueDecoder(const dtd::DtdModel* model,
        ExpansionBudget& budget, bool standalone) noexcept
        : _model(model), _budget(budget), _standalone(standalone)
    {
    }

    // Точка входа: результат накапливается одним буфером на весь атрибут
    // The entry point: the result is accumulated in one buffer for the whole attribute
    std::string AttributeValueDecoder::decode(std::string_view raw) const
    {
        std::string output;
        std::vector<std::string_view> active;
        Expansion expansion{output, active};

        output.reserve(raw.size());
        expandText(raw, expansion);
        return output;
    }

    /*
    ** Посимвольная нормализация. Куски без спецсимволов копируются
    ** одной операцией; КАЖДЫЙ пробельный символ заменяется пробелом.
    ** Пары \r\n здесь уже нет (концы строк нормализованы на входе), а
    ** \r\n из текста сущности (&#13;&#10;) по стандарту даёт ДВА пробела.
    **
    ** Character-by-character normalization. Pieces without special characters are copied
    ** in one operation; EVERY whitespace character is replaced with a space.
    ** There are no \r\n pairs here any more (line endings are normalized on input), and
    ** \r\n from an entity text (&#13;&#10;) gives TWO spaces by the standard.
    */
    void AttributeValueDecoder::expandText(std::string_view text, Expansion& expansion) const
    {
        std::size_t position = 0;

        while (position < text.size())
        {
            std::size_t found = utils::findAny(text, position, "&<\t\n\r");
            if (found == std::string_view::npos)
                found = text.size();
            expansion.output.append(text.substr(position, found - position));
            if (found == text.size())
                return;
            if (text[found] == '<')
                throw DomException(DomErrorCode::PARSE_ERROR,
                    i18n::format(i18n::Message::ATTRIBUTE_LT));
            position = found + 1;
            if (text[found] == '&')
                position = expandReference(text, found, expansion);
            else
                expansion.output += ' ';
        }
    }

    // &#...; — символ как есть (без нормализации); &имя; — предопределённая или из DTD
    // &#...; — the character as is (without normalization); &name; — predefined or from the DTD
    std::size_t AttributeValueDecoder::expandReference(std::string_view text, std::size_t index,
        Expansion& expansion) const
    {
        std::size_t semicolon = text.find(';', index);

        if (semicolon == std::string_view::npos)
            throw DomException(DomErrorCode::PARSE_ERROR,
                i18n::format(i18n::Message::ATTRIBUTE_REFERENCE_UNTERMINATED));
        std::string_view name = text.substr(index + 1, semicolon - index - 1);
        if (!name.empty() && name[0] == '#')
        {
            if (!decodeCharacterReference(name.substr(1), expansion.output))
                throw DomException(DomErrorCode::PARSE_ERROR,
                    i18n::format(i18n::Message::CHARACTER_REFERENCE_INVALID, {name}));
            return semicolon + 1;
        }
        std::optional<char> predefined = predefinedEntity(name);
        if (predefined.has_value())
            expansion.output += predefined.value();
        else
            expandEntity(name, expansion);
        return semicolon + 1;
    }

    /*
    ** Общая сущность в атрибуте: только внутренняя (внешние запрещены
    ** стандартом), без циклов, с оплатой из бюджета. Её текст замены
    ** нормализуется теми же правилами — рекурсивно.
    **
    ** A general entity in an attribute: only an internal one (external ones are forbidden
    ** by the standard), without cycles, paid from the budget. Its replacement text
    ** is normalized by the same rules — recursively.
    */
    void AttributeValueDecoder::expandEntity(std::string_view name, Expansion& expansion) const
    {
        const dtd::EntityDeclaration* entity = nullptr;

        if (_model != nullptr)
            entity = _model->findEntity(name);
        if (entity == nullptr)
            throw DomException(DomErrorCode::PARSE_ERROR,
                i18n::format(i18n::Message::ENTITY_UNKNOWN, {name}));
        if (entity->isExternal)
            throw DomException(DomErrorCode::PARSE_ERROR,
                i18n::format(i18n::Message::ENTITY_EXTERNAL_IN_ATTRIBUTE, {name}));
        if (_standalone && entity->externallyDeclared)
            throw DomException(DomErrorCode::PARSE_ERROR,
                i18n::format(i18n::Message::STANDALONE_EXTERNAL_ENTITY));
        for (std::string_view active : expansion.active)
        {
            if (active == name)
                throw DomException(DomErrorCode::PARSE_ERROR,
                    i18n::format(i18n::Message::ENTITY_RECURSIVE, {name}));
        }
        _budget.charge(entity->value.size(), expansion.active.size() + 1);
        expansion.active.push_back(entity->name);
        expandText(entity->value, expansion);
        expansion.active.pop_back();
    }
}

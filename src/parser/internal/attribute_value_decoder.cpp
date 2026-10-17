#include "parser/internal/attribute_value_decoder.hpp"
#include "parser/internal/reference_decoding.hpp"
#include "exception/dom_exception.hpp"

namespace beresta::detail
{
    // Модель может отсутствовать (документ без DOCTYPE): тогда известны только 5 сущностей
    AttributeValueDecoder::AttributeValueDecoder(const dtd::DtdModel* model,
        ExpansionBudget& budget) noexcept
        : _model(model), _budget(budget)
    {
    }

    // Точка входа: результат накапливается одним буфером на весь атрибут
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
    ** одной операцией; пробельные символы заменяются пробелом.
    */
    void AttributeValueDecoder::expandText(std::string_view text, Expansion& expansion) const
    {
        std::size_t position = 0;

        while (position < text.size())
        {
            std::size_t found = text.find_first_of("&<\t\n\r", position);
            if (found == std::string_view::npos)
                found = text.size();
            expansion.output.append(text.substr(position, found - position));
            if (found == text.size())
                return;
            if (text[found] == '<')
                throw DomException(DomErrorCode::PARSE_ERROR,
                    "символ '<' недопустим в значении атрибута");
            position = found + 1;
            if (text[found] == '&')
                position = expandReference(text, found, expansion);
            else
                expansion.output += ' ';
            if (text[found] == '\r' && position < text.size() && text[position] == '\n')
                ++position;
        }
    }

    // &#...; — символ как есть (без нормализации); &имя; — предопределённая или из DTD
    std::size_t AttributeValueDecoder::expandReference(std::string_view text, std::size_t index,
        Expansion& expansion) const
    {
        std::size_t semicolon = text.find(';', index);

        if (semicolon == std::string_view::npos)
            throw DomException(DomErrorCode::PARSE_ERROR,
                "ссылка на сущность в атрибуте не завершена символом ';'");
        std::string_view name = text.substr(index + 1, semicolon - index - 1);
        if (!name.empty() && name[0] == '#')
        {
            if (!decodeCharacterReference(name.substr(1), expansion.output))
                throw DomException(DomErrorCode::PARSE_ERROR,
                    "недопустимая символьная ссылка &" + std::string(name) + ";");
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
    */
    void AttributeValueDecoder::expandEntity(std::string_view name, Expansion& expansion) const
    {
        const dtd::EntityDeclaration* entity = nullptr;

        if (_model != nullptr)
            entity = _model->findEntity(name);
        if (entity == nullptr)
            throw DomException(DomErrorCode::PARSE_ERROR,
                "неизвестная сущность &" + std::string(name) + ";");
        if (entity->isExternal)
            throw DomException(DomErrorCode::PARSE_ERROR,
                "внешняя сущность &" + std::string(name) + "; недопустима в атрибуте");
        for (std::string_view active : expansion.active)
        {
            if (active == name)
                throw DomException(DomErrorCode::PARSE_ERROR,
                    "рекурсивная сущность &" + std::string(name) + ";");
        }
        _budget.charge(entity->value.size(), expansion.active.size() + 1);
        expansion.active.push_back(entity->name);
        expandText(entity->value, expansion);
        expansion.active.pop_back();
    }
}

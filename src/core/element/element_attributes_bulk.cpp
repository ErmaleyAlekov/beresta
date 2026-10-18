#include "core/element.hpp"
#include "i18n/i18n.hpp"
#include "exception/dom_exception.hpp"
#include "utils/seeded_hash.hpp"
#include "utils/xml_chars.hpp"

#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace beresta
{
    /*
    ** Все ли имена различны. Немного имён — перебор пар без выделения памяти,
    ** много — множество с секретным хешем (имена приходят из документа).
    ** Представления указывают в attributes, который здесь не меняется.
    **
    ** Whether all names are distinct. Few names — trying pairs without memory allocation,
    ** many — a set with a secret hash (the names come from the document).
    ** The views point into attributes, which is not changed here.
    */
    static bool hasUniqueNames(const std::vector<Attribute>& attributes)
    {
        if (attributes.size() <= utils::LINEAR_SEARCH_LIMIT)
        {
            for (std::size_t index = 0; index < attributes.size(); ++index)
            {
                for (std::size_t other = 0; other < index; ++other)
                {
                    if (attributes[other].name == attributes[index].name)
                        return false;
                }
            }
            return true;
        }
        std::unordered_set<std::string_view, utils::SeededHash> names;
        for (const Attribute& attribute : attributes)
        {
            if (!names.insert(attribute.name).second)
                return false;
        }
        return true;
    }

    /*
    ** Сначала проверяются все имена — неверное имя не оставит элемент
    ** изменённым наполовину. Частый случай (разбор: атрибутов ещё нет, имена
    ** различны) — вектор забирается целиком, без копирования строк.
    **
    ** First all names are checked — an invalid name will not leave the element
    ** half-changed. The common case (parsing: there are no attributes yet, the names
    ** are distinct) — the vector is taken as a whole, without copying strings.
    */
    void Element::setAttributes(std::vector<Attribute> attributes)
    {
        for (const Attribute& attribute : attributes)
        {
            if (!utils::isValidName(attribute.name))
                throw DomException(DomErrorCode::INVALID_NAME,
                    i18n::format(i18n::Message::ATTRIBUTE_NAME_INVALID, {attribute.name}));
        }
        if (_attributes.empty() && hasUniqueNames(attributes))
        {
            _attributes = std::move(attributes);
            return;
        }
        mergeAttributes(attributes);
    }

    /*
    ** Общий случай: индекс «имя → номер» по имеющимся атрибутам, затем
    ** каждая новая пара либо заменяет значение, либо добавляется в конец.
    ** Место резервируется заранее: вектор не переедет, и представления имён
    ** в индексе остаются действительными.
    **
    ** The general case: an index "name → number" over the present attributes, then
    ** every new pair either replaces the value or is appended at the end.
    ** Room is reserved in advance: the vector will not move, and the name views
    ** in the index stay valid.
    */
    void Element::mergeAttributes(std::vector<Attribute>& attributes)
    {
        std::unordered_map<std::string_view, std::size_t, utils::SeededHash> index;

        _attributes.reserve(_attributes.size() + attributes.size());
        for (std::size_t position = 0; position < _attributes.size(); ++position)
            index.emplace(_attributes[position].name, position);
        for (Attribute& attribute : attributes)
        {
            auto found = index.find(attribute.name);
            if (found != index.end())
            {
                _attributes[found->second].value = std::move(attribute.value);
                continue;
            }
            _attributes.push_back(std::move(attribute));
            index.emplace(_attributes.back().name, _attributes.size() - 1);
        }
    }
}

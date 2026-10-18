#include "dtd/attribute_defaults.hpp"
#include "parser/internal/attribute_value_decoder.hpp"

namespace beresta::dtd
{
    // Один проход: пробел пишется только между непробельными фрагментами
    // One pass: a space is written only between non-space fragments
    std::string collapseWhitespace(std::string_view value)
    {
        std::string result;
        bool pendingSpace = false;

        for (char symbol : value)
        {
            if (symbol == ' ')
            {
                pendingSpace = !result.empty();
                continue;
            }
            if (pendingSpace)
                result += ' ';
            pendingSpace = false;
            result += symbol;
        }
        return result;
    }

    // Раскрытие сущностей тем же декодером, что и для атрибутов документа
    // Entity expansion with the same decoder as for the document attributes
    std::string normalizeDefaultValue(DtdContext& context, const std::string& raw,
        AttributeType type)
    {
        std::string value = detail::AttributeValueDecoder(&context.model, context.budget).decode(raw);

        if (type == AttributeType::CDATA)
            return value;
        return collapseWhitespace(value);
    }

    // Есть ли атрибут с именем name среди уже указанных
    // Whether there is an attribute named name among those already specified
    static Attribute* findPresent(std::vector<Attribute>& attributes, std::string_view name)
    {
        for (Attribute& attribute : attributes)
        {
            if (attribute.name == name)
                return &attribute;
        }
        return nullptr;
    }

    // Нормализация указанных атрибутов и подстановка значений по умолчанию
    // Normalizes the specified attributes and substitutes default values
    void applyAttributeDeclarations(const DtdModel& model, std::string_view element,
        std::vector<Attribute>& attributes, bool addDefaults)
    {
        const std::vector<AttributeDeclaration>* declarations = model.findAttributes(element);

        if (declarations == nullptr)
            return;
        for (const AttributeDeclaration& declaration : *declarations)
        {
            Attribute* present = findPresent(attributes, declaration.name);
            if (present != nullptr && declaration.type != AttributeType::CDATA)
                present->value = collapseWhitespace(present->value);
            bool hasDefault = declaration.defaultKind == DefaultKind::VALUE
                || declaration.defaultKind == DefaultKind::FIXED;
            if (present == nullptr && addDefaults && hasDefault)
                attributes.push_back(Attribute{declaration.name, declaration.defaultValue});
        }
    }
}

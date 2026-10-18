#include "xsd/internal/stream_validator.hpp"
#include "utils/qualified_name.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Парсер без поддержки пространств имён (namespaceAware = false) не
    ** разрешает префиксы, а схема сравнивает элементы по паре «URI +
    ** локальное имя». Тогда пары вычисляет сам валидатор — по тому же
    ** стеку объявлений xmlns, что и для значений QName; результат совпадает
    ** с подъёмом к корню в дереве. При обычной работе (resolveNamespaces
    ** выключен) элемент проходит без изменений и без копий.
    **
    ** A parser without namespace support (namespaceAware = false) does not
    ** resolve prefixes, and the schema compares elements by the pair "URI +
    ** local name". Then the validator computes the pairs itself — by the same
    ** stack of xmlns declarations as for QName values; the result is the same
    ** as climbing to the root in a tree. In normal operation (resolveNamespaces
    ** is off) the element passes unchanged and without copies.
    */
    SaxElement StreamValidator::resolveNames(const SaxElement& element)
    {
        if (!_settings.resolveNamespaces)
            return element;
        utils::QualifiedNameParts parts = utils::splitQualifiedName(element.name);
        _resolvedElementUri.assign(_prefixes.lookup(parts.prefix).value_or(std::string_view()));
        _resolvedUris.resize(element.attributes.size());
        _resolvedAttributes.clear();
        for (std::size_t index = 0; index < element.attributes.size(); ++index)
        {
            const SaxAttribute& attribute = element.attributes[index];
            utils::QualifiedNameParts name = utils::splitQualifiedName(attribute.name);
            _resolvedUris[index].clear();
            if (!name.prefix.empty() && !utils::isNamespaceDeclaration(attribute.name))
                _resolvedUris[index].assign(_prefixes.lookup(name.prefix).value_or(std::string_view()));
            _resolvedAttributes.push_back(SaxAttribute{attribute.name, attribute.value, _resolvedUris[index],
                name.localName});
        }
        return SaxElement{element.name, _resolvedElementUri, parts.localName, _resolvedAttributes};
    }
}

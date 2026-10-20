#include "xsd/internal/tree_identity.hpp"
#include "xsd/internal/identity_common.hpp"

namespace beresta::xsd::detail
{
    // Дерево построено из тех же событий: можно проверять ключи прежним алгоритмом
    // The tree was built from the same events: the keys can be checked with the former algorithm
    void TreeIdentityRecorder::checkTree(const Document& document)
    {
        TreeIdentityCheck check(_psvi, _log);

        check.run(document);
    }

    // Проверка читает записанный PSVI и пишет ошибки в общий журнал
    // The check reads the recorded PSVI and writes errors into the common log
    TreeIdentityCheck::TreeIdentityCheck(const TreePsvi& psvi, DiagnosticLog& log)
        : _psvi(psvi), _log(log), _sequence(0), _nextScope(0), _scopes(), _tables(), _elementPaths(),
          _elementLocations(), _elementTypes(), _attributeTypes(), _complexElements(), _untypedAttributes(),
          _defaultAttributes(), _fieldPaths()
    {
    }

    // Привязка номеров к узлам, затем таблицы ключей и ссылки
    // Binding the numbers to the nodes, then the key tables and the references
    void TreeIdentityCheck::run(const Document& document)
    {
        const Element* root = document.getRoot();

        if (root == nullptr || _psvi.scopes.empty())
            return;
        bind(*root);
        checkConstraints();
    }

    /*
    ** Обход дерева в прямом порядке даёт элементы ровно в порядке событий
    ** startElement, из которых дерево построено, — поэтому номер элемента
    ** при обходе совпадает с номером, записанным в потоке.
    **
    ** A traversal of the tree in document order gives the elements exactly in the order of
    ** the startElement events the tree was built from — so the element number
    ** during the traversal is the same as the number recorded in the stream.
    */
    void TreeIdentityCheck::bind(const Element& root)
    {
        std::size_t ordinal = 0;

        for (const Element* element = &root; element != nullptr; element = nextElement(*element, root))
            bindElement(*element, ordinal++);
    }

    // Сведения одного элемента: шаг пути, тип значения, составной тип, атрибуты и его области
    // The information of one element: the path step, the value type, the complex type, the attributes and its scopes
    void TreeIdentityCheck::bindElement(const Element& element, std::size_t ordinal)
    {
        auto step = _psvi.steps.find(ordinal);
        auto type = _psvi.elementTypes.find(ordinal);
        auto location = _psvi.locations.find(ordinal);

        if (step != _psvi.steps.end())
            _elementPaths.emplace(&element, step->second);
        if (location != _psvi.locations.end())
            _elementLocations.emplace(&element, location->second.location);
        if (type != _psvi.elementTypes.end())
            _elementTypes.emplace(&element, type->second);
        if (_psvi.complexElements.count(ordinal) != 0)
            _complexElements.insert(&element);
        bindAttributes(element, ordinal);
        while (_nextScope < _psvi.scopes.size() && _psvi.scopes[_nextScope].ordinal == ordinal)
        {
            const ScopeRecord& scope = _psvi.scopes[_nextScope++];
            _scopes.push_back(IdentityScope{&element, scope.declaration, scope.step, scope.position.location});
        }
    }
}

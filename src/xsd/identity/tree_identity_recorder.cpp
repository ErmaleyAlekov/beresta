#include "xsd/internal/tree_identity.hpp"

namespace beresta::xsd::detail
{
    // Запись ведётся в общие пути и журнал валидатора
    // The recording goes into the common paths and log of the validator
    TreeIdentityRecorder::TreeIdentityRecorder(PathTracker& paths, DiagnosticLog& log)
        : _paths(paths), _log(log), _psvi()
    {
    }

    // Область: номер элемента, объявление и шаг пути (для сообщения об ошибке селектора)
    // A scope: the element number, the declaration and the path step (for a selector error message)
    void TreeIdentityRecorder::openScope(std::size_t level, std::size_t ordinal, const EventPosition& position,
        const ElementDecl& declaration)
    {
        _psvi.scopes.push_back(ScopeRecord{ordinal, &declaration, _paths.pin(level), position});
    }

    /*
    ** Как у обхода дерева, после первой области запоминаются шаги путей
    ** проверяемых элементов (сообщения о ключах указывают на них) и типы
    ** их корректных атрибутов, а позиции — у всех элементов: XPath может
    ** выбрать и непроверявшийся элемент. Текст элемента не нужен.
    **
    ** As in the tree traversal, after the first scope the path steps of the
    ** validated elements are remembered (key messages point to them) and the types
    ** of their valid attributes, and the positions — of all elements: XPath may
    ** select an element that was not validated too. The element text is not needed.
    */
    bool TreeIdentityRecorder::elementStarted(const IdentityStart& start)
    {
        _psvi.locations[start.ordinal] = start.position;
        if (!start.validated)
            return false;
        _psvi.steps[start.ordinal] = _paths.pin(start.level);
        for (std::size_t index = 0; index < start.attributeTypes.size(); ++index)
        {
            if (start.attributeTypes[index] != nullptr)
                _psvi.attributeTypes[AttributeSlot{start.ordinal, index}] = start.attributeTypes[index];
        }
        return false;
    }

    // Тип корректного простого значения элемента — для сравнения ключей по значению
    // The type of a valid simple value of the element — for comparing keys by value
    void TreeIdentityRecorder::elementEnded(const IdentityEnd& end)
    {
        if (end.valueType != nullptr)
            _psvi.elementTypes[end.ordinal] = end.valueType;
    }

    /*
    ** В потоке проверять нечего: всё произойдёт в checkTree, уже после
    ** разбора. Поэтому метки позиций переводятся в строки и столбцы сейчас,
    ** пока парсер жив.
    **
    ** There is nothing to check in the stream: everything happens in checkTree, already after
    ** parsing. So the position marks are converted into lines and columns now,
    ** while the parser is alive.
    */
    void TreeIdentityRecorder::finishDocument(const Locator* locator)
    {
        std::vector<EventPosition*> positions;

        if (locator == nullptr)
            return;
        for (ScopeRecord& scope : _psvi.scopes)
            positions.push_back(&scope.position);
        for (std::pair<const std::size_t, EventPosition>& entry : _psvi.locations)
            positions.push_back(&entry.second);
        resolvePositions(positions, *locator);
    }
}

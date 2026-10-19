#include "xsd/internal/stream_validator.hpp"
#include "xsd/internal/assertion_recorder.hpp"

namespace beresta::xsd::detail
{
    // Проход связан со схемой, но не владеет ею
    // The pass is bound to the schema but does not own it
    StreamValidator::StreamValidator(const Schema& schema, const ValidationSettings& settings)
        : _schema(schema), _settings(settings), _locator(nullptr), _frames(), _depth(0), _nextOrdinal(0),
          _paths(), _prefixes(), _log(), _ids(), _normalized(), _references(), _rootPosition(), _text(), _plans(),
          _lastParticle(nullptr), _lastPlan(content::CursorPlan::GREEDY_SEQUENCE),
          _identity(), _assertions(), _resolvedAttributes(), _resolvedUris(), _resolvedElementUri()
    {
    }

    // Стратегия ключей и буфер утверждений удаляются через unique_ptr — здесь их типы полностью известны
    // The key strategy and the assertion buffer are deleted through unique_ptr — their types are fully known here
    StreamValidator::~StreamValidator()
    {
    }

    // Позиции элементов берутся у парсера (или у обхода готового дерева)
    // Element positions are taken from the parser (or from the traversal of a ready tree)
    void StreamValidator::setLocator(const Locator* locator) noexcept
    {
        _locator = locator;
    }

    /*
    ** Ссылки IDREF проверяются, когда известны все ID документа. Парсер ещё
    ** жив, поэтому метки позиций переводятся в строки и столбцы сейчас.
    **
    ** IDREF references are checked when all IDs of the document are known. The parser is
    ** still alive, so the position marks are converted into lines and columns now.
    */
    void StreamValidator::endDocument()
    {
        finishReferences();
        if (_identity != nullptr)
            _identity->finishDocument(_locator);
        if (_locator != nullptr)
            _log.resolve(*_locator);
    }

    // Пути собираются только сейчас: номера одноимённых соседей уже известны
    // The paths are assembled only now: the numbers of same-named siblings are already known
    std::vector<Diagnostic> StreamValidator::takeDiagnostics()
    {
        return _log.render(_paths);
    }
}

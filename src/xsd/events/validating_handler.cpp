#include "xsd/validating_handler.hpp"
#include "xsd/internal/stream_validator.hpp"

#include <utility>

namespace beresta::xsd
{
    // Как строит дерево DomBuilder, так и считает валидатор; без пространств имён — вычисляет их сам
    // As DomBuilder builds the tree, so the validator counts; without namespaces — it computes them itself
    ValidationSettings validationSettingsFor(const ParserOptions& options, IdentityEvaluation identity)
    {
        ValidationSettings settings;

        settings.discardComments = options.discardComments;
        settings.coalesceCData = options.coalesceCData;
        settings.resolveNamespaces = !options.namespaceAware;
        settings.identity = identity;
        return settings;
    }

    // Обработчик разделяет схему (она неизменяема) и не владеет следующим обработчиком
    // The handler shares the schema (it is immutable) and does not own the next handler
    ValidatingHandler::ValidatingHandler(std::shared_ptr<const Schema> schema, ContentHandler* downstream,
        const ValidationSettings& settings)
        : _schema(std::move(schema)), _validator(), _downstream(downstream)
    {
        _validator = std::make_unique<detail::StreamValidator>(*_schema, settings);
    }

    // Валидатор удаляется здесь, где его тип полностью известен
    // The validator is deleted here, where its type is fully known
    ValidatingHandler::~ValidatingHandler()
    {
    }

    // ON_TREE: дерево построено из тех же событий — ключи проверяются по нему
    // ON_TREE: the tree was built from the same events — the keys are checked over it
    void ValidatingHandler::checkIdentity(const Document& document)
    {
        _validator->checkIdentityOnTree(document);
    }

    // Список ошибок с путями (собираются только сейчас)
    // The error list with paths (they are assembled only now)
    std::vector<Diagnostic> ValidatingHandler::takeDiagnostics()
    {
        return _validator->takeDiagnostics();
    }
}

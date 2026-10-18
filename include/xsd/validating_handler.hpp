#ifndef XSD_VALIDATING_HANDLER_HPP
#define XSD_VALIDATING_HANDLER_HPP

#include "core/document.hpp"
#include "parser/error_handler.hpp"
#include "parser/parser_options.hpp"
#include "sax/content_handler.hpp"
#include "xsd/schema.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace beresta::xsd
{
    namespace detail
    {
        class StreamValidator;
    }

    // Где проверяются xs:key / xs:unique / xs:keyref
    // Where xs:key / xs:unique / xs:keyref are checked
    enum class IdentityEvaluation
    {
        STREAMING,  // по ходу потока, для селекторов и полей из грамматики XSD (как в Xerces)
                    // along the stream, for selectors and fields of the XSD grammar (as in Xerces)
        ON_TREE     // после постройки дерева — методом checkIdentity() (любой XPath)
                    // after the tree is built — by the checkIdentity() method (any XPath)
    };

    /*
    ** Настройки потоковой проверки. Первые два поля повторяют то, как
    ** DomBuilder строит дерево (выброшенный комментарий — не ребёнок для
    ** xsi:nil, CDATA, слитая с текстом, — не «значимый текст»): так поток и
    ** готовое дерево дают одинаковые ошибки. resolveNamespaces нужен, если
    ** парсер работает без пространств имён: тогда их вычисляет валидатор.
    **
    ** Streaming validation settings. The first two fields repeat how
    ** DomBuilder builds the tree (a discarded comment is not a child for
    ** xsi:nil, CDATA merged with text is not "significant text"): this way the stream and
    ** a ready tree give the same errors. resolveNamespaces is needed if the
    ** parser works without namespaces: then the validator computes them.
    */
    struct ValidationSettings
    {
        bool discardComments = false;
        bool coalesceCData = false;
        bool resolveNamespaces = false;
        IdentityEvaluation identity = IdentityEvaluation::STREAMING;
    };

    // Настройки, согласованные с параметрами парсера
    // The settings consistent with the parser parameters
    ValidationSettings validationSettingsFor(const ParserOptions& options, IdentityEvaluation identity);

    /*
    ** ValidatingHandler — проверка по XML Schema прямо во время разбора
    ** (паттерн Decorator / Chain of Responsibility): события SAX проходят
    ** через валидатор и уходят дальше — в DomBuilder, в обработчик
    ** пользователя или никуда (downstream == nullptr). Дерево не нужно:
    ** память проверки — O(глубины документа) плюс ошибки, значения ID и
    ** таблицы ключей. Ошибки — те же, что у SchemaValidator: с путём
    ** /a/b[2]/c и позицией начала элемента. Список готов после endDocument
    ** (при ON_TREE — после checkIdentity): номера в путях известны, только
    ** когда документ дочитан.
    **
    ** ValidatingHandler — XML Schema validation right during parsing
    ** (the Decorator / Chain of Responsibility pattern): SAX events pass
    ** through the validator and go further — into DomBuilder, into the user's
    ** handler or nowhere (downstream == nullptr). No tree is needed:
    ** the validation memory is O(document depth) plus errors, ID values and
    ** key tables. The errors are the same as SchemaValidator gives: with the path
    ** /a/b[2]/c and the position of the element start. The list is ready after endDocument
    ** (with ON_TREE — after checkIdentity): the numbers in paths are known only
    ** when the document has been read to the end.
    */
    class ValidatingHandler : public ContentHandler
    {
    public:
        ValidatingHandler(std::shared_ptr<const Schema> schema, ContentHandler* downstream = nullptr,
            const ValidationSettings& settings = ValidationSettings());
        ~ValidatingHandler() override;

        ValidatingHandler(const ValidatingHandler&) = delete;
        ValidatingHandler& operator=(const ValidatingHandler&) = delete;

        // ON_TREE: ограничения идентичности по дереву, построенному из этих же событий
        // ON_TREE: the identity constraints over the tree built from these same events
        void checkIdentity(const Document& document);

        // Все найденные ошибки (пустой список — документ корректен); забирает их из валидатора
        // All errors found (an empty list — the document is valid); takes them from the validator
        std::vector<Diagnostic> takeDiagnostics();

        void setDocumentLocator(const Locator& locator) override;
        void startDocument() override;
        void endDocument() override;
        void xmlDeclaration(const XmlDeclaration& declaration) override;
        void documentType(const DocumentType& documentType) override;
        void startElement(const SaxElement& element) override;
        void endElement(const SaxElement& element) override;
        void characters(std::string_view text) override;
        void cdata(std::string_view text) override;
        void ignorableWhitespace(std::string_view text) override;
        void comment(std::string_view text) override;
        void processingInstruction(std::string_view target, std::string_view data) override;
        void dtdProcessingInstruction(std::string_view target, std::string_view data) override;

    private:
        std::shared_ptr<const Schema> _schema;
        std::unique_ptr<detail::StreamValidator> _validator;
        ContentHandler* _downstream;
    };
}

#endif

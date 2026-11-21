#ifndef XINCLUDE_PROCESSOR_HPP
#define XINCLUDE_PROCESSOR_HPP

#include "core/document.hpp"
#include "parser/parser_options.hpp"
#include "parser/resource_resolver.hpp"

#include <cstddef>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace beresta::xinclude::detail
{
    /*
    ** XIncludeProcessor — обход дерева с заменой xi:include на загруженное
    ** содержимое. Стек URI защищает от циклических включений, глубина
    ** ограничена настройкой maxEntityDepth (те же соображения, что и для сущностей).
    **
    ** XIncludeProcessor — a tree traversal replacing xi:include with the loaded
    ** content. A URI stack protects against circular inclusions, the depth
    ** is limited by the maxEntityDepth setting (the same considerations as for entities).
    */
    class XIncludeProcessor
    {
    public:
        explicit XIncludeProcessor(const ParserOptions& options);

        // Обрабатывает все включения в поддереве root; возвращает число подстановок
        // Processes all inclusions in the subtree of root; returns the number of substitutions
        std::size_t process(ContainerNode& root, const std::string& baseUri);

    private:
        // ---- Обход (xinclude_processor.cpp) ----
        // ---- Traversal (xinclude_processor.cpp) ----
        void processInclude(Element& include, const std::string& baseUri);
        void replaceWith(Element& include, std::vector<std::unique_ptr<Node>> nodes);

        // ---- Загрузка (xinclude_loading.cpp) ----
        // ---- Loading (xinclude_loading.cpp) ----
        std::vector<std::unique_ptr<Node>> loadReplacement(const Element& include,
            const std::string& baseUri);
        Resource fetch(const Element& include, const std::string& baseUri) const;
        std::unique_ptr<Node> loadXml(const Resource& resource, const Element& include);

        // Учёт повторных включений: первая загрузка каждого URI бесплатна, повторы ограничены
        // maxEntityExpansions (число) и maxExpandedSize (байты) — защита от «миллиарда смешков»
        // из цепочки файлов, где каждый включает следующий много раз.
        // Accounting of repeated inclusions: the first load of every URI is free, repeats are limited by
        // maxEntityExpansions (count) and maxExpandedSize (bytes) — protection against a "billion laughs"
        // from a chain of files where each one includes the next many times.
        void charge(const Resource& resource);
        std::unique_ptr<Node> selectPointer(Document& document, std::string_view pointer) const;

        // ---- Запасной вариант (xinclude_fallback.cpp) ----
        // ---- The fallback (xinclude_fallback.cpp) ----
        std::vector<std::unique_ptr<Node>> fallbackNodes(const Element& include,
            const std::string& baseUri);

        const ParserOptions& _options;
        std::vector<std::string> _stack;
        std::size_t _count;
        std::set<std::string> _seen;
        std::size_t _repeats;
        std::size_t _repeatedBytes;
    };

    // Ошибки самого xi:include, не зависящие от ресурса: больше одного xi:fallback,
    // нет ни href, ни xpointer (XInclude 1.0, 3.1 и 3.2). Проверяется до загрузки.
    // Errors of the xi:include itself that do not depend on the resource: more than one xi:fallback,
    // neither href nor xpointer (XInclude 1.0, 3.1 and 3.2). Checked before loading.
    void checkIncludeElement(const Element& include);

    // Все xi:include поддерева, не вложенные в другие xi:include (их обработает родитель)
    // All xi:include of the subtree not nested in other xi:include (the parent will process those)
    std::vector<Element*> collectIncludes(ContainerNode& root);

    // Является ли элемент элементом XInclude с указанным локальным именем
    // Whether the element is an XInclude element with the given local name
    bool isXIncludeElement(const Element& element, std::string_view localName);
}

#endif

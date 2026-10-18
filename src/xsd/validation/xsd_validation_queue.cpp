#include "xsd/internal/validation_run.hpp"

#include <utility>

namespace beresta::xsd::detail
{
    /*
    ** Обход документа без рекурсии. Раньше проверка элемента вызывала
    ** проверку детей, а те — своих детей, и глубокий документ переполнял
    ** стек. Теперь дети откладываются в очередь заданий _pending (стек в
    ** куче), а этот цикл разбирает её, пока она не опустеет.
    **
    ** A document traversal without recursion. Previously checking an element called
    ** the check of its children, and they called their children's, and a deep document
    ** overflowed the stack. Now the children are put into the _pending task queue (a stack in
    ** the heap), and this loop processes it until it is empty.
    */
    void ValidationRun::validateQueued()
    {
        while (!_pending.empty())
        {
            PendingElement task = std::move(_pending.back());
            _pending.pop_back();
            validateElement(*task.element, task.type, task.declaration, task.path);
        }
    }

    /*
    ** Дети кладутся в стек в обратном порядке: первый ребёнок окажется на
    ** вершине и будет проверен первым. Так порядок обхода (а значит, и
    ** порядок сообщений об ошибках) совпадает с порядком документа — тем же,
    ** что давала рекурсия.
    **
    ** The children are pushed onto the stack in reverse order: the first child ends up on
    ** top and is checked first. So the traversal order (and hence the
    ** order of error messages) matches document order — the same one
    ** that recursion gave.
    */
    void ValidationRun::enqueueChildren(std::vector<PendingElement>& children)
    {
        for (auto task = children.rbegin(); task != children.rend(); ++task)
            _pending.push_back(std::move(*task));
    }

    /*
    ** Ошибка у элемента, известного лишь по указателю (ограничения
    ** идентичности находят его через XPath). Путь берётся из записанного
    ** шага; если элемент не проверялся (например, пропущен xs:any), путь
    ** вычисляется по дереву.
    **
    ** An error at an element known only by a pointer (identity
    ** constraints find it through XPath). The path is taken from the recorded
    ** step; if the element was not validated (for example, skipped by xs:any), the path
    ** is computed from the tree.
    */
    void ValidationRun::reportElement(const Element& element, const std::string& message)
    {
        auto found = _elementPaths.find(&element);

        if (found != _elementPaths.end())
            report(element, found->second, message);
        else
            reportAt(element, pathOf(element), message);
    }

    // Ошибка с готовым путём и позицией элемента в исходнике (если она записана)
    // An error with a ready path and the element position in the source (if it was recorded)
    void ValidationRun::reportAt(const Element& element, const std::string& location, const std::string& message)
    {
        const SourceLocation& source = element.getSourceLocation();

        _diagnostics.push_back(Diagnostic{ErrorSeverity::RECOVERABLE_ERROR, location + ": " + message,
            ParsePosition{0, source.line, source.column}});
    }

    // Путь от корня: предки собираются снизу вверх, путь строится сверху вниз
    // The path from the root: the ancestors are collected bottom-up, the path is built top-down
    std::string pathOf(const Element& element)
    {
        std::vector<const Element*> chain;

        for (const Node* node = &element; node != nullptr; node = node->getParent())
        {
            if (node->getType() == NodeType::ELEMENT)
                chain.push_back(static_cast<const Element*>(node));
        }
        std::string path = "/" + chain.back()->getName();
        for (std::size_t index = chain.size() - 1; index > 0; --index)
            path = childPath(path, *chain[index - 1]);
        return path;
    }
}

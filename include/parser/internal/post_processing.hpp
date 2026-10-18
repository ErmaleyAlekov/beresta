#ifndef POST_PROCESSING_HPP
#define POST_PROCESSING_HPP

#include "core/document.hpp"
#include "parser/parser_options.hpp"

namespace beresta::detail
{
    /*
    ** Обработка готового дерева после разбора: XInclude (если
    ** processXInclude) и проверка по XML Schema (если задана schema).
    ** Ошибки схемы направляются в errorHandler как ошибки валидности.
    **
    ** Processing of the ready tree after parsing: XInclude (if
    ** processXInclude) and XML Schema validation (if schema is set).
    ** Schema errors are directed to errorHandler as validity errors.
    */
    void postProcessDocument(Document& document, const ParserOptions& options);
}

#endif

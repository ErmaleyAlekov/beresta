/* SPDX-License-Identifier: MIT — Берёста, см. файл LICENSE в корне проекта */
/* SPDX-License-Identifier: MIT — Beresta, see the LICENSE file in the project root */
#ifndef BERESTA_HPP
#define BERESTA_HPP

/*
** «Берёста» — единый заголовок библиотеки: подключает весь публичный
** интерфейс. Пользователю достаточно написать #include "beresta.hpp".
**
** Beresta — the single library header: includes the whole public
** interface. A user only needs to write #include "beresta.hpp".
*/

#include "exception/dom_exception.hpp"
#include "exception/parse_exception.hpp"

#include "core/node_type.hpp"
#include "core/source_location.hpp"
#include "core/node.hpp"
#include "core/container_node.hpp"
#include "core/attribute.hpp"
#include "core/element.hpp"
#include "core/xml_declaration.hpp"
#include "core/document_type.hpp"
#include "core/document.hpp"
#include "core/character_node.hpp"
#include "core/text.hpp"
#include "core/comment.hpp"
#include "core/cdata_section.hpp"
#include "core/processing_instruction.hpp"

#include "utils/qualified_name.hpp"

#include "encoding/charset.hpp"
#include "io/file_io.hpp"

#include "parser/parser_options.hpp"
#include "parser/error_handler.hpp"
#include "parser/resource_resolver.hpp"
#include "parser/dom_builder.hpp"
#include "parser/xml_parser.hpp"

#include "sax/locator.hpp"
#include "sax/sax_types.hpp"
#include "sax/content_handler.hpp"
#include "sax/sax_parser.hpp"

#include "dtd/dtd_model.hpp"

#include "serializer/xml_serializer.hpp"

#include "traversal/node_filter.hpp"
#include "traversal/tree_walker.hpp"

#include "xpath/xpath_value.hpp"
#include "xpath/xpath.hpp"

#include "xsd/simple_type.hpp"
#include "xsd/schema.hpp"
#include "xsd/schema_validator.hpp"

#include "xinclude/xinclude.hpp"

#endif

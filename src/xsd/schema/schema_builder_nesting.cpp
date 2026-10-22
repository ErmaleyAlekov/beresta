#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

#include <string>

namespace beresta::xsd::detail
{
    // Вход на уровень глубже; предел проверяется ДО увеличения: fail не оставит счётчик сбитым
    // Entering one level deeper; the limit is checked BEFORE increasing: fail leaves the counter intact
    SchemaBuilder::NestingGuard::NestingGuard(SchemaBuilder& builder, const Element& where)
        : _builder(builder)
    {
        if (_builder._nesting >= MAX_NESTING)
            _builder.fail(where, i18n::format(i18n::Message::SCHEMA_TOO_DEEP,
                {std::to_string(MAX_NESTING)}));
        ++_builder._nesting;
    }

    // Выход с уровня — и при обычном возврате, и при исключении
    // Leaving the level — both on a normal return and on an exception
    SchemaBuilder::NestingGuard::~NestingGuard()
    {
        --_builder._nesting;
    }
}

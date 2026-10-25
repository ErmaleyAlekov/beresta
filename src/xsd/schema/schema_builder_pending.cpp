#include "xsd/internal/schema_builder.hpp"
#include "i18n/i18n.hpp"

namespace beresta::xsd::detail
{
    /*
    ** Отложенное построение производных составных типов.
    ** Типы строятся лениво, по первой ссылке, и регистрируются до построения
    ** содержимого. Поэтому база производного типа может оказаться незавершённой
    ** по двум разным причинам:
    ** 1) база ещё строит свою модель, а модель ссылается на элемент производного
    **    типа (контейнер содержит панели, панель расширяет контейнер) — это
    **    рекурсивная структура данных, спецификация её разрешает;
    ** 2) цепочка {base type definition} замкнулась (ct-props-correct.3) — ошибка.
    ** Различить их в момент встречи нельзя: всё зависит от порядка, в котором
    ** строились компоненты. Поэтому производный тип с незавершённой базой не
    ** строится сразу, а ждёт в очереди _pendingTypes (вариант паттерна Command:
    ** задание хранит всё, что нужно для построения позже). Когда тип завершается,
    ** очередь разбирается; в конце сборки в ней могут остаться только типы,
    ** чья цепочка баз никогда не завершится, то есть настоящие циклы.
    **
    ** Deferred building of derived complex types.
    ** Types are built lazily, on the first reference, and registered before their
    ** content is built. So the base of a derived type may be incomplete for two
    ** different reasons:
    ** 1) the base is still building its model, and the model refers to an element of the
    **    derived type (a container holds panels, a panel extends the container) — this
    **    is a recursive data structure, the specification allows it;
    ** 2) the {base type definition} chain is closed (ct-props-correct.3) — an error.
    ** They cannot be told apart at the moment they are met: it all depends on the order
    ** in which the components were built. So a derived type with an incomplete base is
    ** not built at once but waits in the _pendingTypes queue (a variant of the Command
    ** pattern: the job keeps everything needed to build it later). When a type completes,
    ** the queue is processed; at the end of building it may hold only the types whose
    ** chain of bases will never complete, that is real cycles.
    */

    /*
    ** Находит xs:extension / xs:restriction и базовый тип (по умолчанию
    ** xs:anyType). Ссылка на собственное имя в xs:redefine ведёт к прежнему
    ** типу (referencedType). Возвращает true, если содержимое можно строить сразу.
    **
    ** Finds xs:extension / xs:restriction and the base type (xs:anyType by
    ** default). A reference to its own name in xs:redefine leads to the former
    ** type (referencedType). Returns true if the content can be built at once.
    */
    bool SchemaBuilder::readDerivationBase(PendingType& job)
    {
        const Element* content = firstSchemaChild(*job.definition, "simpleContent");
        bool extension = false;

        if (content == nullptr)
            content = firstSchemaChild(*job.definition, "complexContent");
        if (content == nullptr)
            return true;
        job.derivation = &derivationOf(*content, extension);
        std::optional<std::string> name = job.derivation->getAttribute("base");
        if (name.has_value())
            job.base = referencedType(*job.derivation, name.value());
        return !baseIncomplete(job);
    }

    // Ждёт ли тип своей базы: база составная и ещё не достроена
    // Whether the type waits for its base: the base is complex and not complete yet
    bool SchemaBuilder::baseIncomplete(const PendingType& job) const
    {
        return job.base.complex != nullptr && _incomplete.count(job.base.complex) != 0;
    }

    /*
    ** Строит содержимое типа, база которого уже завершена, снимает с него
    ** пометку незавершённости и достраивает типы, которые ждали именно его.
    ** Базовый тип записывается только здесь: у ждущих типов его ещё нет, поэтому
    ** цикл в цепочке не превращается в цикл указателей baseType.
    **
    ** Builds the content of a type whose base is already complete, removes the
    ** incomplete mark from it and finishes the types that were waiting for it.
    ** The base type is set only here: waiting types do not have it yet, so a
    ** cycle in the chain does not become a cycle of baseType pointers.
    */
    void SchemaBuilder::finishComplexType(const PendingType& job)
    {
        ComplexType& type = *job.type;
        const Element* simpleContent = firstSchemaChild(*job.definition, "simpleContent");
        const Element* complexContent = firstSchemaChild(*job.definition, "complexContent");
        bool mixed = booleanAttribute(*job.definition, "mixed", false);

        type.baseType = job.base;
        if (simpleContent != nullptr)
            buildSimpleContent(type, *simpleContent, *job.source);
        else if (complexContent != nullptr)
            buildComplexContent(type, *complexContent, *job.source, mixed);
        else
            buildTypeBody(type, *job.definition, *job.source, mixed);
        _incomplete.erase(&type);
        buildPendingTypes();
    }

    /*
    ** Разбор очереди до неподвижной точки: каждый готовый тип достраивается,
    ** а его завершение может освободить следующие (цепочка C → A → B). Очередь
    ** меняется внутри finishComplexType, поэтому после каждого построения поиск
    ** начинается заново. Задание копируется до удаления из очереди.
    **
    ** Processing the queue up to a fixed point: each ready type is finished,
    ** and its completion may release the next ones (a chain C → A → B). The queue
    ** changes inside finishComplexType, so after each build the search starts
    ** anew. The job is copied before it is removed from the queue.
    */
    void SchemaBuilder::buildPendingTypes()
    {
        std::size_t index = 0;

        while (index < _pendingTypes.size())
        {
            if (baseIncomplete(_pendingTypes[index]))
            {
                ++index;
                continue;
            }
            PendingType job = _pendingTypes[index];
            _pendingTypes.erase(_pendingTypes.begin() + static_cast<std::ptrdiff_t>(index));
            NestingGuard nesting(*this, *job.definition);
            finishComplexType(job);
            index = 0;
        }
    }

    /*
    ** В конце сборки все незавершённые составные типы лежат в очереди: каждый
    ** ждёт свою базу, которая тоже в очереди. Если идти от любого из них по
    ** базам столько шагов, сколько заданий в очереди, придём внутрь цикла —
    ** на его определении и сообщается об ошибке.
    **
    ** At the end of building all incomplete complex types are in the queue: each
    ** waits for its base, which is in the queue too. Walking from any of them along
    ** the bases as many steps as there are jobs in the queue leads inside a cycle —
    ** the error is reported at its definition.
    */
    void SchemaBuilder::checkPendingTypes() const
    {
        if (_pendingTypes.empty())
            return;
        const PendingType* current = &_pendingTypes.front();
        for (std::size_t step = 0; step < _pendingTypes.size(); ++step)
        {
            for (const PendingType& job : _pendingTypes)
            {
                if (job.type == current->base.complex)
                {
                    current = &job;
                    break;
                }
            }
        }
        fail(*current->derivation, i18n::format(i18n::Message::SCHEMA_CYCLIC_DERIVATION));
    }
}

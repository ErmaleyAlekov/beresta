#include "failing/case_executor.hpp"

namespace failing
{
    /*
    ** Случаи нумеруются как в Catalog::cases; группа случая берётся из
    ** TestGroup (schemaCase и instanceCases хранят индексы случаев).
    **
    ** The cases are numbered as in Catalog::cases; the group of a case is taken from
    ** TestGroup (schemaCase and instanceCases hold the case indexes).
    */
    CaseIndex indexCatalog(const xsts::Catalog& catalog)
    {
        CaseIndex index;

        index.groupOf.assign(catalog.cases.size(), 0);
        for (std::size_t position = 0; position < catalog.cases.size(); ++position)
            index.byId[catalog.cases[position].id] = position;
        for (std::size_t group = 0; group < catalog.groups.size(); ++group)
        {
            if (catalog.groups[group].hasSchema)
                index.groupOf[catalog.groups[group].schemaCase] = group;
            for (std::size_t member : catalog.groups[group].instanceCases)
                index.groupOf[member] = group;
        }
        return index;
    }
}

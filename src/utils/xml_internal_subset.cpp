#include "utils/xml_tokens.hpp"

namespace beresta::utils
{
    /*
    ** Конец литерала, комментария или инструкции, которые начинаются в position.
    ** Литералы бывают только внутри объявлений, комментарии и инструкции — только
    ** между ними. Возвращает position, если здесь ничего такого не начинается, и
    ** npos, если начавшееся не закрыто.
    **
    ** The end of a literal, a comment or an instruction starting at position.
    ** Literals occur only inside declarations, comments and instructions only
    ** between them. Returns position if nothing of the kind starts here, and
    ** npos if what has started is not closed.
    */
    static std::size_t skipDelimited(std::string_view subset, std::size_t position, bool inDeclaration) noexcept
    {
        char symbol = subset[position];
        std::string_view end;
        std::size_t from = position + 1;

        if (inDeclaration && (symbol == '"' || symbol == '\''))
            end = subset.substr(position, 1);
        else if (!inDeclaration && subset.compare(position, 4, "<!--") == 0)
        {
            end = "-->";
            from = position + 4;
        }
        else if (!inDeclaration && subset.compare(position, 2, "<?") == 0)
        {
            end = "?>";
            from = position + 2;
        }
        else
            return position;
        std::size_t close = subset.find(end, from);
        if (close == std::string_view::npos)
            return std::string_view::npos;
        return close + end.size();
    }

    /*
    ** Объявление идёт от '<' до '>' вне литералов. Кавычка вне объявления и «]»
    ** где угодно, кроме литералов, комментариев и инструкций, недопустимы.
    **
    ** A declaration runs from '<' to '>' outside literals. A quote outside a declaration and "]"
    ** anywhere except literals, comments and instructions are not allowed.
    */
    bool isClosedInternalSubset(std::string_view subset) noexcept
    {
        std::size_t position = 0;
        bool inDeclaration = false;

        while (position < subset.size())
        {
            std::size_t next = skipDelimited(subset, position, inDeclaration);
            if (next == std::string_view::npos)
                return false;
            if (next != position)
            {
                position = next;
                continue;
            }
            char symbol = subset[position];
            if (symbol == ']' || (!inDeclaration && (symbol == '"' || symbol == '\'')))
                return false;
            if (symbol == '<')
                inDeclaration = true;
            else if (symbol == '>')
                inDeclaration = false;
            ++position;
        }
        return !inDeclaration;
    }
}

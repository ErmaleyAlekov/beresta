#include "xsts/limitations.hpp"

#include <string>

namespace xsts
{
    /*
    ** Не применимы:
    **  - спорные случаи (status="queried"): рабочая группа W3C не подтвердила
    **    ожидаемый результат, по многим открыты ошибки в Bugzilla;
    **  - случаи с атрибутом version не для XSD 1.0.
    **
    ** Not applicable:
    **  - disputed cases (status="queried"): the W3C working group has not confirmed
    **    the expected result, many have open Bugzilla bugs;
    **  - cases with a version attribute not for XSD 1.0.
    */
    std::string skipReason(const TestCase& testCase)
    {
        if (testCase.status == "queried")
            return "спорный случай (status=\"queried\"): ожидание не подтверждено W3C";
        if (!testCase.version.empty() && testCase.version.find("1.0") == std::string::npos)
            return "случай не для XSD 1.0 (version=\"" + testCase.version + "\")";
        return std::string();
    }
}

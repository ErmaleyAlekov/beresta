#include "xsts/case_runner.hpp"

#include <string>

namespace xsts
{
    /*
    ** schemaTest: valid — схема загрузилась; invalid — схема отвергнута
    ** (любым исключением библиотеки). Сбой не-библиотечным исключением —
    ** всегда провал: так ведёт себя только ошибка в самой «Берёсте».
    **
    ** schemaTest: valid — the schema loaded; invalid — the schema is rejected
    ** (by any library exception). A failure with a non-library exception is
    ** always a failure: only a bug in Beresta itself behaves like this.
    */
    CaseResult judgeSchema(const TestCase& testCase, const SchemaAttempt& attempt)
    {
        if (attempt.unexpected)
            return CaseResult{&testCase, Verdict::FAILED, "сбой библиотеки: " + attempt.message};
        if (testCase.expected == Expectation::VALID)
        {
            if (attempt.rejected)
                return CaseResult{&testCase, Verdict::FAILED, "корректная схема отвергнута: " + attempt.message};
            return CaseResult{&testCase, Verdict::PASSED, "схема загружена"};
        }
        if (attempt.rejected)
            return CaseResult{&testCase, Verdict::PASSED, "схема отвергнута: " + attempt.message};
        return CaseResult{&testCase, Verdict::FAILED, "ошибочная схема принята"};
    }

    /*
    ** instanceTest: valid — разобран и ни одной ошибки проверки;
    ** invalid — хотя бы одна ошибка или экземпляр не well-formed.
    ** Схема по подсказкам, которая не загрузилась, — провал в обе стороны:
    ** без схемы невалидность экземпляра ничего не доказывает.
    **
    ** instanceTest: valid — parsed and not a single validation error;
    ** invalid — at least one error or the instance is not well-formed.
    ** A schema by hints that did not load is a failure both ways:
    ** without a schema the invalidity of the instance proves nothing.
    */
    CaseResult judgeInstance(const TestCase& testCase, const InstanceAttempt& attempt)
    {
        if (attempt.unexpected)
            return CaseResult{&testCase, Verdict::FAILED, "сбой библиотеки: " + attempt.message};
        if (attempt.schemaFailed)
            return CaseResult{&testCase, Verdict::FAILED, attempt.message};
        bool invalid = attempt.rejected || attempt.errors > 0;
        if (testCase.expected == Expectation::VALID)
        {
            if (attempt.rejected)
                return CaseResult{&testCase, Verdict::FAILED, "экземпляр не разобран: " + attempt.message};
            if (invalid)
                return CaseResult{&testCase, Verdict::FAILED, "ложная ошибка: " + attempt.message};
            return CaseResult{&testCase, Verdict::PASSED, "экземпляр валиден"};
        }
        if (invalid)
            return CaseResult{&testCase, Verdict::PASSED, "ошибка найдена: " + attempt.message};
        return CaseResult{&testCase, Verdict::FAILED, "невалидный экземпляр принят"};
    }
}

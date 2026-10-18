#include "conformance/case_runner.hpp"

#include <string>

namespace conformance
{
    namespace
    {
        // not-wf: документ ОБЯЗАН быть отвергнут фатальной ошибкой
        // not-wf: the document MUST be rejected with a fatal error
        CaseResult judgeNotWellFormed(const TestCase& testCase, const ParseAttempt& attempt)
        {
            // Причина отказа сохраняется: по ней видно, что документ отвергнут «за дело»
            // The rejection reason is kept: it shows that the document was rejected "for a reason"
            if (attempt.rejected)
                return CaseResult{&testCase, Verdict::PASSED, attempt.fatalMessage};
            return CaseResult{&testCase, Verdict::FAILED,
                "документ не well-formed, но принят парсером"};
        }

        // valid: ни фатальных ошибок, ни ошибок валидности
        // valid: neither fatal errors nor validity errors
        CaseResult judgeValid(const TestCase& testCase, const ParseAttempt& attempt)
        {
            if (attempt.rejected)
                return CaseResult{&testCase, Verdict::FAILED,
                    "корректный документ отвергнут: " + attempt.fatalMessage};
            if (attempt.validityErrors > 0)
                return CaseResult{&testCase, Verdict::FAILED,
                    "ложная ошибка валидности: " + attempt.firstValidityError};
            return CaseResult{&testCase, Verdict::PASSED, ""};
        }

        // invalid: документ well-formed, но должна быть хотя бы одна ошибка валидности
        // invalid: the document is well-formed, but there must be at least one validity error
        CaseResult judgeInvalid(const TestCase& testCase, const ParseAttempt& attempt)
        {
            if (attempt.rejected)
                return CaseResult{&testCase, Verdict::FAILED,
                    "well-formed документ отвергнут как фатальная ошибка: " + attempt.fatalMessage};
            if (attempt.validityErrors == 0)
                return CaseResult{&testCase, Verdict::FAILED,
                    "нарушение валидности не обнаружено"};
            return CaseResult{&testCase, Verdict::PASSED, ""};
        }
    }

    /*
    ** Вердикт по типу случая. Непредусмотренное исключение проваливает
    ** любой случай. Категория error по стандарту необязательна к
    ** обнаружению: случай засчитывается при любом штатном исходе, а
    ** в причину записывается, что именно сделал парсер (для отчёта).
    **
    ** The verdict by the case type. An unexpected exception fails
    ** any case. By the standard the error category is optional to
    ** detect: the case is counted as passed with any regular outcome, and
    ** the reason records what exactly the parser did (for the report).
    */
    CaseResult judgeCase(const TestCase& testCase, const ParseAttempt& attempt)
    {
        if (attempt.unexpected)
            return CaseResult{&testCase, Verdict::FAILED,
                "непредусмотренное исключение: " + attempt.fatalMessage};
        if (testCase.type == CaseType::NOT_WF)
            return judgeNotWellFormed(testCase, attempt);
        if (testCase.type == CaseType::VALID)
            return judgeValid(testCase, attempt);
        if (testCase.type == CaseType::INVALID)
            return judgeInvalid(testCase, attempt);
        if (attempt.rejected)
            return CaseResult{&testCase, Verdict::PASSED, "ошибка сообщена: " + attempt.fatalMessage};
        return CaseResult{&testCase, Verdict::PASSED, "ошибка не сообщалась (допустимо)"};
    }
}

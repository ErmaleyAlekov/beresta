#include "conformance/case_runner.hpp"

#include "beresta.hpp"

#include <exception>
#include <utility>

namespace conformance
{
    namespace
    {
        /*
        ** Настройки разбора под случай. Для сравнения с канонической формой
        ** нужен «полный» текст: пробельные узлы сохраняются, CDATA сливается
        ** с текстом, комментарии отбрасываются (в канонической форме их нет).
        ** Внешние DTD и сущности загружаются всегда: без них не обнаружить
        ** часть ошибок well-formedness во внешних подмножествах.
        **
        ** Parse settings for a case. For comparison with the canonical form
        ** the "full" text is needed: whitespace nodes are kept, CDATA is merged
        ** with the text, comments are discarded (there are none in the canonical form).
        ** External DTDs and entities are always loaded: without them some
        ** well-formedness errors in external subsets cannot be detected.
        */
        beresta::ParserOptions makeOptions(const TestCase& testCase)
        {
            beresta::ParserOptions options;

            options.preserveWhitespaceText = true;
            options.coalesceCData = true;
            options.discardComments = true;
            options.namespaceAware = testCase.namespaceAware;
            options.loadExternalDtd = true;
            // Валидность проверяется только там, где её оценивает вердикт
            // Validity is checked only where the verdict assesses it
            if (testCase.type == CaseType::VALID || testCase.type == CaseType::INVALID)
                options.validateDtd = true;
            return options;
        }

        // Переносит восстановимые ошибки из сборщика в итог попытки
        // Moves the recoverable errors from the collector into the attempt outcome
        void collectValidityErrors(const beresta::CollectingErrorHandler& collector,
            ParseAttempt& attempt)
        {
            for (const beresta::Diagnostic& diagnostic : collector.getDiagnostics())
            {
                if (diagnostic.severity != beresta::ErrorSeverity::RECOVERABLE_ERROR)
                    continue;
                if (attempt.validityErrors == 0)
                    attempt.firstValidityError = diagnostic.message;
                ++attempt.validityErrors;
            }
        }
    }

    CaseRunner::CaseRunner(std::filesystem::path suiteRoot)
        : _suiteRoot(std::move(suiteRoot))
    {
    }

    /*
    ** Порядок прогона: применимость -> разбор -> вердикт по фактам ->
    ** (для прошедших valid-случаев с OUTPUT) сравнение канонического вывода.
    **
    ** The run order: applicability -> parsing -> the verdict by the facts ->
    ** (for passed valid cases with OUTPUT) the comparison of the canonical output.
    */
    CaseResult CaseRunner::run(const TestCase& testCase) const
    {
        std::string skip = skipReason(testCase);

        if (!skip.empty())
            return CaseResult{&testCase, Verdict::SKIPPED, skip};
        ParseAttempt attempt = attemptParse(testCase);
        CaseResult result = judgeCase(testCase, attempt);
        bool needsOutput = testCase.type == CaseType::VALID && !testCase.output.empty();

        if (result.verdict == Verdict::PASSED && needsOutput && attempt.document)
        {
            std::string mismatch = compareOutput(testCase, *attempt.document);

            if (!mismatch.empty())
            {
                result.verdict = Verdict::FAILED;
                result.reason = mismatch;
            }
        }
        return result;
    }

    /*
    ** Любое исключение библиотеки (DomException и наследники) — это
    ** «документ отвергнут». Всё прочее (std::bad_alloc, out_of_range...)
    ** — непредусмотренный сбой: такой случай всегда считается проваленным.
    **
    ** Any library exception (DomException and its subclasses) means
    ** "the document is rejected". Everything else (std::bad_alloc, out_of_range...)
    ** is an unexpected failure: such a case is always counted as failed.
    */
    ParseAttempt CaseRunner::attemptParse(const TestCase& testCase) const
    {
        beresta::FileResourceResolver resolver(_suiteRoot);
        beresta::CollectingErrorHandler collector;
        beresta::ParserOptions options = makeOptions(testCase);
        ParseAttempt attempt;

        options.resolver = &resolver;
        options.errorHandler = &collector;
        try
        {
            attempt.document = beresta::XmlParser(options).parseFile(testCase.file);
        }
        catch (const beresta::DomException& problem)
        {
            attempt.rejected = true;
            attempt.fatalMessage = problem.what();
        }
        catch (const std::exception& problem)
        {
            attempt.unexpected = true;
            attempt.fatalMessage = problem.what();
        }
        collectValidityErrors(collector, attempt);
        return attempt;
    }
}

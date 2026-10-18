package org.beresta.selftest;

/**
 * Перечень проверок. Перечисление вместо лямбд: каждая константа явно
 * называет проверку и вызывает её в единственном методе run (паттерн
 * Command — проверка как объект, который можно передать и выполнить).
 *
 * <p>The list of checks. An enumeration instead of lambdas: each constant names
 * its check explicitly and calls it in the single run method (the Command
 * pattern — a check as an object that can be passed around and executed).
 */
enum Check
{
    PARSE_AND_NAVIGATE,
    BUILD_AND_SERIALIZE,
    COPY_AND_REMOVE,
    MOVE_BETWEEN_DOCUMENTS,
    MOVE_CLOSED_TARGET,
    MOVE_ERRORS,
    ERRORS,
    FILES_AND_ENCODINGS,
    XPATH_VALUES,
    XPATH_SELECT,
    SCHEMA,
    LANGUAGES,
    UNKNOWN_LANGUAGE;

    /** Выполнение проверки, соответствующей константе. / Runs the check that matches the constant. */
    void run() throws Exception
    {
        switch (this)
        {
            case PARSE_AND_NAVIGATE -> DocumentChecks.parseAndNavigate();
            case BUILD_AND_SERIALIZE -> DocumentChecks.buildAndSerialize();
            case COPY_AND_REMOVE -> DocumentChecks.copyAndRemove();
            case MOVE_BETWEEN_DOCUMENTS -> MoveChecks.moveBetweenDocuments();
            case MOVE_CLOSED_TARGET -> MoveChecks.closedTargetDocument();
            case MOVE_ERRORS -> MoveChecks.moveErrors();
            case ERRORS -> DocumentChecks.errors();
            case FILES_AND_ENCODINGS -> FileChecks.filesAndEncodings();
            case XPATH_VALUES -> XPathSchemaChecks.xpathValues();
            case XPATH_SELECT -> XPathSchemaChecks.xpathSelect();
            case SCHEMA -> XPathSchemaChecks.schema();
            case LANGUAGES -> LanguageChecks.languages();
            case UNKNOWN_LANGUAGE -> LanguageChecks.unknownLanguage();
        }
    }
}

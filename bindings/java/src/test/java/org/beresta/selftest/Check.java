package org.beresta.selftest;

/**
 * Перечень проверок. Перечисление вместо лямбд: каждая константа явно
 * называет проверку и вызывает её в единственном методе run (паттерн
 * Command — проверка как объект, который можно передать и выполнить).
 */
enum Check
{
    PARSE_AND_NAVIGATE,
    BUILD_AND_SERIALIZE,
    COPY_AND_REMOVE,
    ERRORS,
    FILES_AND_ENCODINGS,
    XPATH_VALUES,
    XPATH_SELECT,
    SCHEMA;

    /** Выполнение проверки, соответствующей константе. */
    void run() throws Exception
    {
        switch (this)
        {
            case PARSE_AND_NAVIGATE -> DocumentChecks.parseAndNavigate();
            case BUILD_AND_SERIALIZE -> DocumentChecks.buildAndSerialize();
            case COPY_AND_REMOVE -> DocumentChecks.copyAndRemove();
            case ERRORS -> DocumentChecks.errors();
            case FILES_AND_ENCODINGS -> FileChecks.filesAndEncodings();
            case XPATH_VALUES -> XPathSchemaChecks.xpathValues();
            case XPATH_SELECT -> XPathSchemaChecks.xpathSelect();
            case SCHEMA -> XPathSchemaChecks.schema();
        }
    }
}

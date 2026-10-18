package bench;

/**
 * Замеряемые операции: разбор, обход, XPath, разбор с проверкой по схеме.
 * The measured operations: parsing, walking, XPath, parsing with schema validation.
 */
public enum Operation
{
    PARSE,
    WALK,
    XPATH,
    VALIDATE
}

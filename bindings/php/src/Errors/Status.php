<?php

declare(strict_types=1);

namespace Beresta;

/**
 * Коды состояния Си-интерфейса (beresta_status).
 *
 * Значения 1..15 совпадают с beresta::DomErrorCode + 1, поэтому во всех
 * языках у ошибок одинаковые номера и имена. Имя в виде "PARSE_ERROR"
 * отдаёт BerestaException::statusName() — оно берётся у самой библиотеки.
 *
 * The status codes of the C interface (beresta_status).
 *
 * Values 1..15 match beresta::DomErrorCode + 1, so the errors have the same
 * numbers and names in all languages. The name in the "PARSE_ERROR" form is
 * returned by BerestaException::statusName() — it comes from the library itself.
 */
enum Status: int
{
    case Ok = 0;
    case InvalidName = 1;
    case InvalidContent = 2;
    case HierarchyViolation = 3;
    case IndexOutOfRange = 4;
    case NullNode = 5;
    case ParseError = 6;
    case NotFound = 7;
    case NamespaceError = 8;
    case IoError = 9;
    case EncodingError = 10;
    case ValidationError = 11;
    case XPathError = 12;
    case SchemaError = 13;
    case ResourceError = 14;
    case LimitExceeded = 15;
    case InvalidArgument = 16;
    case OutOfMemory = 17;
    case UnknownError = 18;
}

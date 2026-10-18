package org.beresta.selftest;

import org.beresta.BerestaException;
import org.beresta.Languages;
import org.beresta.Xml;
import org.beresta.XmlDocument;
import org.beresta.XmlElement;

/** Проверки языков сообщений. / Message language checks. */
final class LanguageChecks
{
    private LanguageChecks()
    {
    }

    /**
     * Английский язык во время работы: ошибки разбора и обёртки; неизвестный код отвергается.
     * English at run time: parser and binding errors; an unknown code is rejected.
     */
    static void languages()
    {
        Expect.equal("ru", Languages.available().get(0), "первый язык / first language");
        Languages.use("en");
        try
        {
            englishParseError();
        }
        catch (RuntimeException failure)
        {
            Languages.use("ru");
            throw failure;
        }
        closedDocumentMessage();
    }

    // Ошибка разбора приходит по-английски / The parse error arrives in English
    private static void englishParseError()
    {
        try
        {
            Xml.parse("<a><b></a>");
            throw Expect.missingError("ошибка разбора / parse error");
        }
        catch (BerestaException error)
        {
            Expect.equal("line 1, column 7: expected </b>, found </a>", error.detail(), "английский текст / English text");
        }
    }

    // Сообщение обёртки о закрытом документе; язык возвращается к ru
    // The binding message about a closed document; the language goes back to ru
    private static void closedDocumentMessage()
    {
        XmlDocument document = Xml.parse("<a/>");
        XmlElement root = document.root();
        document.close();
        try
        {
            root.text();
            throw Expect.missingError("закрытый документ / closed document");
        }
        catch (IllegalStateException expected)
        {
            Expect.isTrue(expected.getMessage().contains("the document is already closed"), expected.getMessage());
        }
        finally
        {
            Languages.use("ru");
        }
    }

    /** Неизвестный язык: ошибка INVALID_ARGUMENT, язык не меняется. / Unknown language: INVALID_ARGUMENT, unchanged. */
    static void unknownLanguage()
    {
        try
        {
            Languages.use("xx");
            throw Expect.missingError("неизвестный язык / unknown language");
        }
        catch (BerestaException error)
        {
            Expect.equal("INVALID_ARGUMENT", error.codeName(), "код ошибки / error code");
            Expect.equal("ru", Languages.current(), "язык не изменился / language unchanged");
        }
    }
}

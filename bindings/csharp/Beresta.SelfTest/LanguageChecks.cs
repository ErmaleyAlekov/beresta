using System;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Проверки языков сообщений. / Message language checks.</summary>
    internal static class LanguageChecks
    {
        /// <summary>
        /// Английский во время работы, неизвестный код языка отвергается.
        /// English at run time; an unknown language code is rejected.
        /// </summary>
        internal static void Languages()
        {
            Expect.Equal("ru", BerestaLibrary.Languages[0], "первый язык / first language");
            BerestaLibrary.Language = "en";
            try
            {
                Xml.Parse("<a><b></a>");
                throw Expect.MissingError("ошибка разбора / parse error");
            }
            catch (BerestaException error)
            {
                Expect.Equal("line 1, column 7: expected </b>, found </a>", error.Detail, "английский текст / English text");
            }
            finally
            {
                BerestaLibrary.Language = "ru";
            }
            try
            {
                BerestaLibrary.Language = "xx";
                throw Expect.MissingError("неизвестный язык / unknown language");
            }
            catch (BerestaException error)
            {
                Expect.Equal("INVALID_ARGUMENT", error.CodeName, "код ошибки / error code");
            }
        }
    }
}

using System;
using System.Text;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Запуск всех проверок с печатью результата каждой.
    /// Runs all checks and prints the result of each.</summary>
    internal static class Program
    {
        private static int failures;

        private static void Run(string name, Action check)
        {
            try
            {
                check();
                Console.WriteLine("[ OK ] " + name);
            }
            catch (Exception error)
            {
                ++failures;
                Console.WriteLine("[FAIL] " + name + ": " + error.Message);
            }
        }

        // Проверки передаются как группы методов (не лямбды): Action из имени метода
        // Checks are passed as method groups (not lambdas): an Action from a method name
        private static int Main()
        {
            Console.OutputEncoding = Encoding.UTF8;
            // Тексты ошибок сверяются по-русски — независимо от языка сборки
            // Error texts are compared in Russian — regardless of the build language
            BerestaLibrary.Language = "ru";
            Console.WriteLine("Берёста " + BerestaLibrary.Version + " для .NET");
            Run("разбор и навигация", DocumentChecks.ParseAndNavigate);
            Run("построение и вывод", DocumentChecks.BuildAndSerialize);
            Run("копирование и удаление", DocumentChecks.CopyAndRemove);
            Run("перенос между документами", MoveChecks.MoveBetweenDocuments);
            Run("перенос: закрытый новый документ", MoveChecks.ClosedTargetDocument);
            Run("перенос: запрещённые случаи", MoveChecks.MoveErrors);
            Run("ошибки", DocumentChecks.Errors);
            Run("файлы и кодировки", DocumentChecks.FilesAndEncodings);
            Run("XPath: значения", XPathSchemaChecks.XPathValues);
            Run("XPath: выборка", XPathSchemaChecks.XPathSelect);
            Run("XML Schema", XPathSchemaChecks.Schema);
            Run("XSD поток: совпадает с деревом", StreamingChecks.StreamMatchesTree);
            Run("XSD поток: файлы и ошибки", StreamingChecks.StreamFile);
            Run("50 000 переносов и реестр обёрток", LoadChecks.BulkMove);
            Run("8 потоков на общих схеме и XPath", LoadChecks.ParallelShared);
            Run("языки сообщений / message languages", LanguageChecks.Languages);
            Console.WriteLine("Итого упало: " + failures);
            return Math.Min(failures, 1);
        }
    }
}

using System;
using System.Text;
using Beresta;

namespace Beresta.SelfTest
{
    /// <summary>Запуск всех проверок с печатью результата каждой.</summary>
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
        private static int Main()
        {
            Console.OutputEncoding = Encoding.UTF8;
            Console.WriteLine("Берёста " + BerestaLibrary.Version + " для .NET");
            Run("разбор и навигация", DocumentChecks.ParseAndNavigate);
            Run("построение и вывод", DocumentChecks.BuildAndSerialize);
            Run("копирование и удаление", DocumentChecks.CopyAndRemove);
            Run("ошибки", DocumentChecks.Errors);
            Run("файлы и кодировки", DocumentChecks.FilesAndEncodings);
            Run("XPath: значения", XPathSchemaChecks.XPathValues);
            Run("XPath: выборка", XPathSchemaChecks.XPathSelect);
            Run("XML Schema", XPathSchemaChecks.Schema);
            Console.WriteLine("Итого упало: " + failures);
            return Math.Min(failures, 1);
        }
    }
}

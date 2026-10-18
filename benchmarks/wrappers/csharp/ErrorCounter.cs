using System.IO;
using System.Xml;
using System.Xml.Schema;

namespace WrapperBench
{
    /// <summary>
    /// Настройки проверяющего XmlReader и счётчик ошибок проверки — общие для
    /// XmlDocument и XDocument: ошибки считаются, а не бросаются, как у
    /// «Берёсты» со сбором диагностик.
    ///
    /// The settings of a validating XmlReader and a validation error counter — common for
    /// XmlDocument and XDocument: errors are counted, not thrown, as in
    /// Beresta with diagnostics collection.
    /// </summary>
    public sealed class ErrorCounter
    {
        /// <summary>Схема компилируется один раз. / The schema is compiled once.</summary>
        public ErrorCounter(string xsd)
        {
            Settings.ValidationType = ValidationType.Schema;
            Settings.Schemas.Add(null, XmlReader.Create(new StringReader(xsd)));
            Settings.ValidationEventHandler += Count;
        }

        /// <summary>Настройки проверяющего чтения. / The validating reader settings.</summary>
        public XmlReaderSettings Settings { get; } = new XmlReaderSettings();

        /// <summary>Число ошибок с последнего обнуления. / The error count since the last reset.</summary>
        public int Errors { get; set; }

        // Обработчик события проверки / The validation event handler
        private void Count(object? sender, ValidationEventArgs problem)
        {
            ++Errors;
        }
    }
}

using System.Globalization;

namespace WrapperBench
{
    internal static class BenchRate
    {
        // Скорость по измеряемому интервалу / Throughput for a measurable interval
        internal static string Format(double megabytes, double milliseconds)
        {
            if (milliseconds <= 0)
                return "—";
            return (megabytes / milliseconds * 1000).ToString("F0", CultureInfo.InvariantCulture);
        }
        // Сопоставимый итог проверки XSD / Comparable XSD validation outcome
        internal static string Validity(int errors)
        {
            if (errors == 0)
                return "yes";
            return "no";
        }
    }
}

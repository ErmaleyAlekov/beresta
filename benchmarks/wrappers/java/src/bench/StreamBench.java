package bench;

import java.io.ByteArrayInputStream;
import java.io.StringReader;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;

import javax.xml.XMLConstants;
import javax.xml.transform.stream.StreamSource;
import javax.xml.validation.Schema;
import javax.xml.validation.SchemaFactory;
import javax.xml.validation.Validator;

import org.beresta.Beresta;
import org.beresta.XmlSchema;

/**
 * Потоковая проверка по XSD без дерева на тех же данных, что и WrapperBench:
 * «Берёста» ({@code XmlSchema.validate(bytes, flags)} — SAX + XSD внутри ядра)
 * против стандартного {@code javax.xml.validation.Validator} над
 * {@code StreamSource} (тоже SAX, без DOM). Медиана из RUNS прогонов после WARMUP.
 *   java --enable-native-access=ALL-UNNAMED -cp out bench.StreamBench data
 *
 * <p>Streaming XSD validation without a tree on the same data as WrapperBench:
 * Beresta ({@code XmlSchema.validate(bytes, flags)} — SAX + XSD inside the core)
 * against the standard {@code javax.xml.validation.Validator} over a
 * {@code StreamSource} (SAX as well, no DOM). The median of RUNS runs after WARMUP.
 */
public final class StreamBench
{
    private static final int WARMUP = 3;
    private static final int RUNS = 5;

    private StreamBench()
    {
    }

    /** Точка входа: путь к данным — первый аргумент. / The entry point: the data path is the first argument. */
    public static void main(String[] args) throws Exception
    {
        try
        {
            run(args);
        }
        catch (IOException failure)
        {
            System.err.println(BenchValues.READ_FAILED + failure.getMessage());
            System.exit(2);
        }
    }

    /** Читает данные и печатает результаты. / Reads input and prints results. */
    private static void run(String[] args) throws Exception
    {
        Path data = Path.of("data");
        if (args.length > 0)
            data = Path.of(args[0]);
        byte[] xml = Files.readAllBytes(data.resolve("catalog.xml"));
        String xsd = Files.readString(data.resolve("catalog.xsd"));
        Schema schema = SchemaFactory.newInstance(XMLConstants.W3C_XML_SCHEMA_NS_URI).newSchema(new StreamSource(new StringReader(xsd)));
        JdkValidation jdk = new JdkValidation(schema);
        double megabytes = xml.length / 1e6;

        try (XmlSchema beresta = XmlSchema.fromString(xsd))
        {
            System.out.println("| Библиотека / Library | SAX + XSD, MB/s | Ошибки XSD / XSD errors | XSD valid |");
            System.out.println("|---|---|---|---|");
            int berestaErrors = once(xml, beresta, null);
            int jdkErrors = once(xml, null, jdk);
            System.out.printf("| Берёста / Beresta %s (validate(bytes)) | %s | %d | %s |%n", Beresta.version(),
                BenchValues.rate(megabytes, median(xml, beresta, null)), berestaErrors, BenchValues.validity(berestaErrors));
            System.out.printf("| JDK %s Validator (StreamSource) | %s | %d | %s |%n", System.getProperty("java.version"),
                BenchValues.rate(megabytes, median(xml, null, jdk)), jdkErrors, BenchValues.validity(jdkErrors));
        }
    }

    /** Медиана времени в мс; задана одна из двух схем. / The median time in ms; one of the two schemas is given. */
    private static double median(byte[] xml, XmlSchema beresta, JdkValidation jdk) throws Exception
    {
        double[] times = new double[RUNS];

        for (int run = -WARMUP; run < RUNS; ++run)
        {
            long start = System.nanoTime();
            once(xml, beresta, jdk);
            long elapsed = System.nanoTime() - start;
            if (run >= 0)
                times[run] = elapsed / 1e6;
        }
        Arrays.sort(times);
        return times[RUNS / 2];
    }

    /** Одна проверка; результат — число нарушений. / One validation; the result is the violation count. */
    private static int once(byte[] xml, XmlSchema beresta, JdkValidation jdk) throws Exception
    {
        if (beresta != null)
            return beresta.validate(xml, 0).size();
        return jdk.count(xml);
    }

    /** Валидатор и обработчик создаются до прогрева. / The validator and handler are created before warm-up. */
    private static final class JdkValidation
    {
        private final Validator validator;
        private final CountingErrorHandler errors = new CountingErrorHandler();

        private JdkValidation(Schema schema)
        {
            validator = schema.newValidator();
            validator.setErrorHandler(errors);
        }

        private int count(byte[] xml) throws Exception
        {
            errors.reset();
            validator.validate(new StreamSource(new ByteArrayInputStream(xml)));
            return errors.count();
        }
    }
}

package bench;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXParseException;

/**
 * Счётчик ошибок проверки для JDK: ошибки и предупреждения считаются, а
 * не бросаются, — как у «Берёсты» со сбором диагностик.
 *
 * A validation error counter for the JDK: errors and warnings are counted, not
 * thrown — as in Beresta with diagnostics collection.
 */
public final class CountingErrorHandler implements ErrorHandler
{
    private int count;

    /** Обнуление перед новым документом. / Resetting before a new document. */
    public void reset()
    {
        count = 0;
    }

    /** Число ошибок последнего документа. / The error count of the last document. */
    public int count()
    {
        return count;
    }

    @Override
    public void warning(SAXParseException problem)
    {
        ++count;
    }

    @Override
    public void error(SAXParseException problem)
    {
        ++count;
    }

    @Override
    public void fatalError(SAXParseException problem) throws SAXParseException
    {
        throw problem;
    }
}

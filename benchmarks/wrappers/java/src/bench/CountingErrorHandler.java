package bench;

import org.xml.sax.ErrorHandler;
import org.xml.sax.SAXParseException;

/**
 * Счётчик нарушений схемы JDK. Предупреждения не влияют на валидность.
 *
 * JDK schema violation counter. Warnings do not affect validity.
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

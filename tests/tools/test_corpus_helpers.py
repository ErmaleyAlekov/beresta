"""Регрессии извлечения литералов и отбора схем.
Regressions for literal extraction and schema selection.
"""

import importlib.util
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


TOOLS = Path(__file__).resolve().parents[1] / "fuzz" / "tools"
SPEC = importlib.util.spec_from_file_location("cpp_literals", TOOLS / "cpp_literals.py")
CPP_LITERALS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(CPP_LITERALS)


class CorpusHelpersTests(unittest.TestCase):
    """Проверки на миниатюрных исходниках / Checks with small source fixtures."""

    def test_character_literals_do_not_consume_following_strings(self):
        """Кавычки внутри символов и комментариев пропускаются.
        Quotes inside character literals and comments are skipped.
        """
        source = r'''
char quote = '"';
char apostrophe = '\'';
char escaped_quote = '\"';
// "ignored"
const char* document = "<r>" "text\x21" "</r>";
/* "also ignored" */
const char* query = "/r";
'''
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "example.cpp"
            path.write_text(source, encoding="utf-8")
            self.assertEqual(CPP_LITERALS.read_literals(path), ["<r>text!</r>", "/r"])

    def test_schema_namespace_with_alternate_and_default_prefixes(self):
        """Обе схемы попадают в корпус, а экземпляр служит второй частью.
        Both schemas enter the corpus, and the instance supplies the second part.
        """
        namespace = "http://www.w3.org/2001/XMLSchema"
        schemas = [f"<xsd:schema xmlns:xsd='{namespace}'/>", f"<schema xmlns='{namespace}'/>"]
        instance = "<root/>"
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            sources = root / "tests" / "src" / "xsd"
            sources.mkdir(parents=True)
            literals = "\n".join(f'const char* sample_{index} = "{sample}";'
                                 for index, sample in enumerate([*schemas, instance]))
            (sources / "example.cpp").write_text(literals, encoding="utf-8")
            corpus = root / "corpus"
            subprocess.run(
                [sys.executable, "-I", str(TOOLS / "make_corpus.py"), str(root), str(corpus)],
                capture_output=True, check=True, timeout=30,
            )
            seeds = {path.read_bytes() for path in (corpus / "schema").iterdir()}
            expected = {b"\0" + schema.encode() + b"\xf5\xf5" + instance.encode() for schema in schemas}
            self.assertEqual(seeds, expected)


if __name__ == "__main__":
    unittest.main()

"""Регрессии Markdown для генератора документов реестра.
Markdown regressions for the registry documentation generator."""

import unittest

from tools.registry.docgen.errors import DocsError
from tools.registry.docgen.md_blocks import render_markdown
from tools.registry.docgen.md_inline import render_inline


class MarkdownTests(unittest.TestCase):
    """Проверки ссылок, экранирования и блоков кода.
    Checks for links, escaping, and fenced code blocks."""

    def test_local_url_and_label_are_rendered_separately(self):
        """Адрес экранируется как атрибут, подпись форматируется отдельно.
        The URL is escaped as an attribute and the label is formatted separately."""
        self.assertEqual(
            render_inline('[`x` **имя**](page.md?q=a"b&hint=*raw*#part)'),
            '<a href="page.html?q=a&quot;b&amp;hint=*raw*#part"><code>x</code> <strong>имя</strong></a>',
        )

    def test_uri_schemes_use_allowlist(self):
        """Разрешённые схемы создают ссылку, остальные оставляют подпись.
        Supported schemes produce a link, while others leave the label."""
        for target in ("HTTPS://example.org/page.md", "mailto:user@example.org"):
            with self.subTest(target=target):
                self.assertEqual(render_inline(f"[сайт]({target})"), f'<a href="{target}">сайт</a>')
        self.assertEqual(render_inline("[**сайт**](unsupported:example)"), "<strong>сайт</strong>")
        self.assertEqual(render_inline("[сайт](path\x01name)"), "сайт")

    def test_unclosed_fence_reports_opening_line(self):
        """Незакрытый блок даёт ошибку с номером открывающей строки.
        An unclosed fence gives an error with its opening line number."""
        with self.assertRaisesRegex(DocsError, r"(?<!\d)3(?!\d)"):
            render_markdown("# Название\n\n```python\nbody\n\n## Текст")

    def test_closed_fence_keeps_following_heading(self):
        """Раздел за закрытым блоком остаётся в оглавлении.
        The section following a closed fence stays in the table of contents."""
        document = render_markdown("```text\n<x>\n```\n\n## Раздел")
        self.assertIn('<pre><code class="language-text">&lt;x&gt;</code></pre>', document["html"])
        self.assertEqual(document["toc"], [("раздел", "Раздел")])

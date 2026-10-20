# Security Policy

[Русский](SECURITY.md) | English

Beresta parses XML from untrusted sources, so security bugs matter to us most of all.

## Supported Versions

Patches are released in the latest minor branch.

| Version | Support |
|---|---|
| 2.1.x | ✅ security patches |
| < 2.1 | ❌ update to 2.1.x |

## How to Report a Vulnerability

**Do not open a public issue or post in discussions** — this will make the vulnerability known before it is fixed.

Report privately through GitHub: [Security → Report a vulnerability](https://github.com/ErmaleyAlekov/beresta/security/advisories/new). If you do not have access to GitHub — by email to [ermaley.alekov@yandex.ru](mailto:ermaley.alekov@yandex.ru) with the subject "Security: beresta".

In your message, specify:

- the version of Beresta and the connection method (C++, C interface, Java, C#, Python, Node.js, WebAssembly);
- OS, compiler, or runtime environment;
- the minimum XML / XSD / XPath on which the error occurs, and what happens (a crash, a hang, memory growth, reading a file, a network request);
- if available, an assessment of the consequences.

## What Happens Next

- Acknowledgment of receipt — within 7 days.
- Assessment and a fix plan — within 30 days; we will notify you of the timeline in advance.
- The fix is released as a patch version on GitHub and Gitverse and in npm, PyPI, NuGet, and Maven Central packages; the reporter is credited in [CHANGELOG.en.md](CHANGELOG.en.md) and in the GitHub Security Advisory (if they wish).

## What is Considered a Vulnerability

Protection in Beresta is enabled by default, and bypassing it is a vulnerability:

- loading external resources (DTD, entities, XInclude, `xs:include` / `xs:import`) without an explicitly passed `ResourceResolver` (XXE, SSRF), `FileResourceResolver` exiting the root directory;
- bypassing entity limits (entity bombs) and depth limits;
- non-linear time on XSD regular expressions, attributes, and keys (hash flooding), on XPath;
- crash, memory overflow, leak, or undefined behavior on any input document, schema, or XPath expression;
- C interface and wrapper errors leading to the same.

Not a vulnerability: behavior when loading resources explicitly allowed through your resolver, unsupported checks that are deliberate limitations (see [docs/comparison.en.md](docs/comparison.en.md#limitations)), conformance bugs with no security impact — report these in regular issues.

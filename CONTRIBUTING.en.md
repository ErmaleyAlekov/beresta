# Contributing

[Русский](CONTRIBUTING.md) | English

Thank you for wanting to help Beresta! Bug reports, tests, fixes, wrappers, and translations are equally valuable. By participating, you agree to the [Code of Conduct](CODE_OF_CONDUCT.en.md). Report vulnerabilities privately — see [SECURITY.en.md](SECURITY.en.md).

## Where

The main repository is [GitHub](https://github.com/ErmaleyAlekov/beresta), mirror is [Gitverse](https://gitverse.ru/ermaleyalekov/beresta). Issues and pull requests are accepted on both platforms; the maintainer transfers accepted changes to both.

## Bug Report

- Version (`beresta_version()` or `BERESTA_VERSION_STRING`), OS, compiler, or wrapper language.
- Minimal XML / XSD / XPath that reproduces the error, expected and actual result.
- For conformance bugs — link to the specification section (XML 1.0, Namespaces, XSD 1.0 / 1.1, XPath 1.0) or to a W3C test case.

## Code Changes

1. Fork and branch from `main`.
2. Build and run tests — instructions in [docs/building.en.md](docs/building.en.md):

```bash
make test
```

3. For XSD changes run the W3C XML Schema Test Suite, for the parser — W3C XML Conformance (see [docs/development.en.md](docs/development.en.md#tests)): the pass rate must not drop.
4. Add a test for the fixed bug or the new feature: a unit test in `tests/src/<module>/` and its registration.
5. Open a pull request into `main`: what changed, why and how it was checked. CI must be green.

## Code rules

The project follows strict coding rules. Every pull request is reviewed by the maintainer personally.

More about the project layout and how to add a node type, an XPath function or an XSD type — in [docs/development.en.md](docs/development.en.md#extending-the-library).

## Documentation

Every document exists in Russian (`*.md`) and English (`*.en.md`); when you change one, update the other. Add notable changes to the upcoming version section of [CHANGELOG.md](CHANGELOG.md) and [CHANGELOG.en.md](CHANGELOG.en.md).

## License

By submitting changes you agree that they are distributed under the project license — see [LICENSE](LICENSE).

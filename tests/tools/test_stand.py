"""Регрессии сценариев проверочного стенда.
Regression tests for the test stand scripts."""

import os
from pathlib import Path
import shutil
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[2]
SHELL = shutil.which("sh")


@unittest.skipUnless(SHELL, "POSIX sh is required")
class StandTests(unittest.TestCase):
    """Проверки диагностики и протокола выполнения шагов.
    Checks for diagnostics and step reporting."""

    def test_unknown_option_uses_diagnostic_catalog(self):
        """Неизвестный параметр даёт ошибку на языке стенда.
        An unknown option produces an error in the stand language."""
        messages = {"ru": "Неизвестный параметр: --bad", "en": "Unknown option: --bad"}
        for language, message in messages.items():
            with self.subTest(language=language):
                environment = dict(os.environ, STAND_LANG=language)
                result = subprocess.run(
                    [SHELL, "tools/stand/run_stand.sh", "--bad"],
                    cwd=ROOT, env=environment, capture_output=True, encoding="utf-8",
                )
                self.assertEqual(result.returncode, 1)
                self.assertEqual(result.stdout, "")
                self.assertEqual(result.stderr.strip(), message)

    def test_steps_do_not_need_local_and_record_failure(self):
        """Успех и ошибка шага фиксируются средствами POSIX.
        Step success and failure are recorded using POSIX facilities."""
        script = r"""
set -eu
. ./tools/stand/lib/messages.sh
. ./tools/stand/lib/report.sh
local() { exit 99; }
STAND_LANG=en
STAND_FAILED=0
STAND_LOGS=$(mktemp -d)
STAND_REPORT="$STAND_LOGS/report"
trap 'rm -f "$STAND_REPORT" "$STAND_LOGS/pass.log" "$STAND_LOGS/fail.log"; rmdir "$STAND_LOGS"' 0
pass_step() { printf '%s\n' passed; }
fail_step() { printf '%s\n' failed; return 7; }
status=caller
run_step pass pass_step step_test
if run_step fail fail_step step_test; then exit 42; fi
[ "$STAND_FAILED" = 1 ] && [ "$status" = caller ]
"""
        result = subprocess.run([SHELL, "-c", script], cwd=ROOT, capture_output=True, encoding="utf-8")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn("[OK] Unit tests", result.stdout)
        self.assertIn("[FAIL] Unit tests", result.stdout)
        self.assertIn("\nfailed\n", result.stdout)

#!/usr/bin/env python3

import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

EXPECTED = {
    "after_success": [
        ("allows successful after hooks to install overrides", "passed", []),
        ("restores overrides after a successful after hook", "passed", []),
    ],
    "before_failure": [
        (
            "reports the before failure without running the body",
            "failed",
            ["EXPECTED_BEFORE_FAILURE"],
        ),
        ("starts the next case with restored overrides", "passed", []),
    ],
    "after_failure": [
        (
            "retains the body error before both cleanup errors",
            "failed",
            [
                "EXPECTED_BODY_FAILURE",
                "EXPECTED_INNER_FAILURE",
                "EXPECTED_OUTER_FAILURE",
            ],
        ),
        (
            "fails a successful body when cleanup fails",
            "failed",
            ["EXPECTED_INNER_FAILURE", "EXPECTED_OUTER_FAILURE"],
        ),
        ("restores overrides installed by after hooks", "passed", []),
    ],
    "retry_cleanup": [
        ("retries with restored overrides and shared setup", "passed", []),
        ("keeps the next case clean after a retry", "passed", []),
    ],
    "retry_after_failure": [
        ("retries an after hook failure with restored overrides", "passed", []),
        ("continues after the successful cleanup retry", "passed", []),
    ],
    "restore_failure": [
        (
            "retains body and restoration errors while finishing cleanup",
            "failed",
            [
                "EXPECTED_RESTORE_BODY_FAILURE",
                "Restoring 'xi.test.cleanupProbe.target' failed",
                "Restoring setting 'map.EXP_RETAIN' failed",
            ],
        ),
        (
            "retries failed restorations after after hooks repair their targets",
            "passed",
            [],
        ),
    ],
}


def verify(fixture, returncode, report):
    expected = EXPECTED[fixture]
    expected_failed = sum(status == "failed" for _, status, _ in expected)
    errors = []
    if returncode != int(expected_failed > 0):
        errors.append(f"exit code {returncode}, expected {int(expected_failed > 0)}")

    results = report["results"]
    tests = results["tests"]
    summary = results["summary"]
    if summary["skipped"] != 0 or summary["tests"] != len(expected):
        errors.append(f"unexpected test counts: {summary}")
    if (
        summary["failed"] != expected_failed
        or summary["passed"] != len(expected) - expected_failed
    ):
        errors.append(f"unexpected outcomes: {summary}")
    if [test["name"] for test in tests] != [name for name, _, _ in expected]:
        errors.append("test identities/order differ from the fixture")
        return errors

    for test, (name, status, markers) in zip(tests, expected):
        if test["status"] != status:
            errors.append(f"{name}: {test['status']}, expected {status}")
        trace = test["trace"]
        position = 0
        for marker in markers:
            found = trace.find(marker, position)
            if found < 0:
                errors.append(f"{name}: missing or reordered error {marker}")
            else:
                position = found + len(marker)
        if any(
            marker in trace
            for marker in ("UNEXPECTED_BODY_EXECUTION", "NOT_RUN", "VISIBLE_TO_AFTER")
        ):
            errors.append(f"{name}: {trace}")
        if not fixture.startswith("retry_") and (test["retries"] or test["flaky"]):
            errors.append(f"{name}: unexpected retry")

    if fixture.startswith("retry_"):
        test = tests[0]
        attempts = test["retryAttempts"]
        if test["retries"] != 1 or not test["flaky"]:
            errors.append("expected exactly one successful retry")
        if [attempt["status"] for attempt in attempts] != ["failed", "passed"]:
            errors.append("expected a failed first attempt followed by a pass")
        elif (
            "EXPECTED_RETRY_AFTER_FAILURE"
            if fixture == "retry_after_failure"
            else "EXPECTED_RETRY_FAILURE"
        ) not in attempts[0]["message"]:
            errors.append("retry did not retain the original error")
        if tests[1]["retries"] or tests[1]["flaky"]:
            errors.append("the following case unexpectedly needed a retry")

    return errors


def main():
    parser = argparse.ArgumentParser(
        description="Run expected-failure xi_test cleanup fixtures against an isolated test database."
    )
    parser.add_argument(
        "--root", type=Path, default=Path(__file__).resolve().parents[3]
    )
    parser.add_argument("--binary", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--fixture", action="append", choices=EXPECTED)
    parser.add_argument("--timeout", type=int, default=180)
    args = parser.parse_args()
    root = args.root.resolve()
    binary = args.binary.resolve()
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    fixture_root = Path(__file__).resolve().parent / "fixtures"
    failures = []

    for fixture in args.fixture or EXPECTED:
        report_path = output / f"{fixture}.ctrf.json"
        log_path = output / f"{fixture}.log"
        if report_path.exists() or log_path.exists():
            parser.error(f"Refusing to overwrite evidence for {fixture} in {output}")
        with tempfile.TemporaryDirectory(
            prefix="_attempt_cleanup_", dir=root / "scripts/tests"
        ) as directory:
            staged = Path(directory) / f"{fixture}.lua"
            shutil.copyfile(fixture_root / staged.name, staged)
            command = [
                str(binary),
                "--keep-going",
                "--seed",
                "1",
                "--file",
                re.escape(staged.relative_to(root).as_posix()),
                "--output",
                str(report_path),
            ]
            if fixture.startswith("retry_"):
                command.extend(["--retry", "1"])
            with log_path.open("w", encoding="utf-8") as log:
                try:
                    completed = subprocess.run(
                        command,
                        cwd=root,
                        stdout=log,
                        stderr=subprocess.STDOUT,
                        timeout=args.timeout,
                        check=False,
                    )
                    if report_path.exists():
                        errors = verify(
                            fixture,
                            completed.returncode,
                            json.loads(report_path.read_text()),
                        )
                    else:
                        errors = [
                            f"exit code {completed.returncode}, missing CTRF report"
                        ]
                except subprocess.TimeoutExpired:
                    errors = [f"exceeded {args.timeout} seconds"]
            failures.extend(f"{fixture}: {error}" for error in errors)
            print(f"{fixture}: {'FAIL' if errors else 'PASS'}", flush=True)

    for failure in failures:
        print(failure)
    return int(bool(failures))


if __name__ == "__main__":
    raise SystemExit(main())

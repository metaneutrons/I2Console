#!/usr/bin/env python3
"""Assert that a guard job holds at least the write grants of the job it guards.

Written after v0.2.0 failed at `preflight`. That job inherited the workflow's
`contents: read` and then asked whether its own token could push. It could not,
so it aborted, and it would have aborted on every future release too. A guard
that holds less than the stage it protects can only ever produce a false alarm,
and no prerelease reaches `preflight` to reveal it.

The relation is not inferable in general, so it is declared below.
"""

from __future__ import annotations

import pathlib
import sys

import yaml

WORKFLOW = pathlib.Path(".github/workflows/release.yml")

# guard job -> job whose access it is standing in for
GUARDS = {"preflight": "stage"}


def write_grants(job: dict, workflow: dict) -> set[str]:
    block = job.get("permissions", workflow.get("permissions"))
    if block == "write-all":
        return {"*"}
    if not isinstance(block, dict):
        return set()
    return {k for k, v in block.items() if v == "write"}


def main() -> int:
    doc = yaml.safe_load(WORKFLOW.read_text())
    jobs = doc.get("jobs") or {}
    status = 0

    for guard_name, guarded_name in GUARDS.items():
        for name in (guard_name, guarded_name):
            if name not in jobs:
                print(f"::error::{WORKFLOW}: job '{name}' does not exist")
                return 1

        guard = write_grants(jobs[guard_name], doc)
        guarded = write_grants(jobs[guarded_name], doc)

        # The guard does not need every grant the guarded job has, only the
        # ones it actually verifies. `contents` is the one it asks about.
        needed = guarded & {"contents"}
        missing = needed - guard
        if missing:
            print(
                f"::error::{WORKFLOW}: job '{guard_name}' guards '{guarded_name}' "
                f"but lacks write on: {', '.join(sorted(missing))}. "
                f"It would report a failure for a release that is in fact allowed."
            )
            status = 1
        else:
            print(
                f"{guard_name} guards {guarded_name}: "
                f"holds {sorted(guard) or 'none'}, needs {sorted(needed) or 'none'} — ok"
            )

    return status


if __name__ == "__main__":
    sys.exit(main())

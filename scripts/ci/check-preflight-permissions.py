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

# Every publishing job and the secret it consumes. The preflight has to read
# the same secret, otherwise it verifies a credential nobody uses and the real
# one is first exercised in the middle of a public mutation.
PUBLISHERS = {"publish-esp-registry": "IDF_COMPONENT_API_TOKEN"}


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

    # The publisher pin must exist exactly once and must not be a literal, so
    # the preflight and the publisher cannot run different versions of the tool
    # that performs the publication. v0.3.1 died on exactly that drift.
    text = WORKFLOW.read_text()
    literal = [
        line.strip()
        for line in text.splitlines()
        if "idf-component-manager==" in line and "${" not in line
    ]
    if literal:
        for line in literal:
            print(
                f"::error::{WORKFLOW}: the component manager version is hard-coded "
                f"in a step instead of taken from the workflow-level declaration: {line}"
            )
        status = 1
    declarations = [
        line for line in text.splitlines()
        if line.strip().startswith("IDF_COMPONENT_MANAGER_VERSION:")
    ]
    if len(declarations) != 1:
        print(
            f"::error::{WORKFLOW}: expected exactly one "
            f"IDF_COMPONENT_MANAGER_VERSION declaration, found {len(declarations)}"
        )
        status = 1
    else:
        uses = text.count("idf-component-manager==${IDF_COMPONENT_MANAGER_VERSION}")
        print(
            f"component manager pinned once as "
            f"{declarations[0].split(':', 1)[1].strip()} and used {uses}x — ok"
        )

    doc_text = WORKFLOW.read_text()
    for job_name, secret in PUBLISHERS.items():
        if job_name not in jobs:
            print(f"::error::{WORKFLOW}: publishing job '{job_name}' does not exist")
            status = 1
            continue
        ref = f"secrets.{secret}"
        for who in ("preflight", job_name):
            block = yaml.dump(jobs[who])
            if ref not in block:
                print(
                    f"::error::{WORKFLOW}: job '{who}' does not read {ref}. "
                    f"The preflight and the publisher must use the same credential, "
                    f"or the preflight proves nothing about the publication."
                )
                status = 1
        if not status:
            print(f"{job_name} and preflight both read {ref} — ok")

        # A publication secret in a job without an environment would be a
        # repository secret, readable by every workflow in the repository.
        for who in ("preflight", job_name):
            if jobs[who].get("environment") is None:
                print(
                    f"::error::{WORKFLOW}: job '{who}' reads a publication secret "
                    f"but declares no environment"
                )
                status = 1

    return status


if __name__ == "__main__":
    sys.exit(main())

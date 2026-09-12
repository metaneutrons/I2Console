#!/usr/bin/env python3
"""Generate an SPDX 2.3 SBOM for one release payload.

Why this exists instead of a syft scan
--------------------------------------
`syft scan dir:.` was tried first and produced a misleading document: 98
packages, of which 53 were GitHub Actions and 35 PyPI packages picked up from
the pico-sdk submodule's own CI tooling, and not one of them was a build input
of the firmware. Syft has no cataloguer for a CMake project or for git
submodules, so it cannot see what actually determines this image.

What determines it is small enough to state exactly: this repository at a
commit, the pico-sdk submodule at its pinned commit, whatever of the SDK's
vendored libraries the link line actually pulls in, and the compiler. That is
what this writes.

The document is deterministic. The namespace is derived from repository, tag
and payload name rather than from a random UUID, and the creation timestamp
comes from SOURCE_DATE_EPOCH, so two runs over the same inputs produce
byte-identical output and the SBOMs of the three payloads differ only in the
payload name and the SPDX identifiers derived from it.
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import pathlib
import re
import subprocess
import sys

SPDX_VERSION = "SPDX-2.3"


def run(*args: str, cwd: str | None = None) -> str:
    return subprocess.run(
        args, cwd=cwd, check=True, capture_output=True, text=True
    ).stdout.strip()


def sha256(path: pathlib.Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def spdx_id(*parts: str) -> str:
    # SPDX identifiers allow letters, digits, '.' and '-' only.
    tail = "-".join(parts)
    return "SPDXRef-" + re.sub(r"[^A-Za-z0-9.\-]", "-", tail)


def linked_tinyusb(root: pathlib.Path) -> bool:
    """True when the link line actually pulls TinyUSB in.

    Read rather than assumed, so the SBOM stops claiming TinyUSB the moment
    the link line stops using it.
    """
    text = (root / "CMakeLists.txt").read_text()
    block = re.search(r"target_link_libraries\s*\(([^)]*)\)", text, re.S)
    if not block:
        raise SystemExit("no target_link_libraries block in CMakeLists.txt")
    return bool(re.search(r"\btinyusb_\w+", block.group(1)))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--payload", required=True, help="path to the payload file")
    ap.add_argument("--name", required=True, help="payload name as released")
    ap.add_argument("--version", required=True, help="release core, e.g. 0.1.0")
    ap.add_argument("--tag", required=True)
    ap.add_argument("--repository", required=True, help="owner/repo")
    ap.add_argument("--epoch", required=True, type=int, help="SOURCE_DATE_EPOCH")
    ap.add_argument("--root", default=".", help="repository root")
    ap.add_argument("--output", required=True)
    args = ap.parse_args()

    root = pathlib.Path(args.root).resolve()
    payload = pathlib.Path(args.payload)
    if not payload.is_file() or payload.stat().st_size == 0:
        raise SystemExit(f"payload {payload} is missing or empty")

    created = (
        dt.datetime.fromtimestamp(args.epoch, dt.timezone.utc)
        .strftime("%Y-%m-%dT%H:%M:%SZ")
    )
    commit = run("git", "-C", str(root), "rev-parse", "HEAD")
    sdk_dir = root / "pico-sdk"
    sdk_commit = run("git", "-C", str(sdk_dir), "rev-parse", "HEAD")
    try:
        sdk_version = run("git", "-C", str(sdk_dir), "describe", "--tags")
    except subprocess.CalledProcessError:
        sdk_version = sdk_commit[:12]
    sdk_url = run(
        "git", "-C", str(root), "config", "-f", ".gitmodules",
        "submodule.pico-sdk.url",
    )
    toolchain = run("arm-none-eabi-gcc", "-dumpversion")

    fw_id = spdx_id("Package", "I2Console", args.version)
    sdk_id = spdx_id("Package", "pico-sdk", sdk_version)
    tc_id = spdx_id("Package", "arm-none-eabi-gcc", toolchain)
    root_id = spdx_id("Payload", args.name)

    packages = [
        {
            "SPDXID": fw_id,
            "name": "I2Console",
            "versionInfo": args.version,
            "downloadLocation": (
                f"https://github.com/{args.repository}/releases/download/"
                f"{args.tag}/{args.name}"
            ),
            "filesAnalyzed": False,
            "licenseConcluded": "GPL-3.0-or-later",
            "licenseDeclared": "GPL-3.0-or-later",
            "copyrightText": "Copyright (C) 2025 Metaneutrons",
            "supplier": f"Organization: {args.repository.split('/')[0]}",
            "sourceInfo": (
                f"built from https://github.com/{args.repository} at {commit}"
            ),
            "checksums": [
                {"algorithm": "SHA256", "checksumValue": sha256(payload)}
            ],
            "externalRefs": [
                {
                    "referenceCategory": "PACKAGE-MANAGER",
                    "referenceType": "purl",
                    "referenceLocator":
                        f"pkg:github/{args.repository}@{args.tag}",
                }
            ],
        },
        {
            "SPDXID": sdk_id,
            "name": "pico-sdk",
            "versionInfo": sdk_version,
            "downloadLocation": sdk_url,
            "filesAnalyzed": False,
            "licenseConcluded": "BSD-3-Clause",
            "licenseDeclared": "BSD-3-Clause",
            "copyrightText": "Copyright (c) 2020 Raspberry Pi (Trading) Ltd.",
            "supplier": "Organization: Raspberry Pi Ltd",
            "sourceInfo": f"git submodule pinned at {sdk_commit}",
            "externalRefs": [
                {
                    "referenceCategory": "PACKAGE-MANAGER",
                    "referenceType": "purl",
                    "referenceLocator":
                        f"pkg:github/raspberrypi/pico-sdk@{sdk_commit}",
                }
            ],
        },
        {
            "SPDXID": tc_id,
            "name": "arm-none-eabi-gcc",
            "versionInfo": toolchain,
            "downloadLocation": "NOASSERTION",
            "filesAnalyzed": False,
            "licenseConcluded": "GPL-3.0-or-later WITH GCC-exception-3.1",
            "licenseDeclared": "NOASSERTION",
            "copyrightText": "NOASSERTION",
            "supplier": "NOASSERTION",
        },
    ]

    relationships = [
        {"spdxElementId": "SPDXRef-DOCUMENT",
         "relationshipType": "DESCRIBES", "relatedSpdxElement": root_id},
        {"spdxElementId": root_id,
         "relationshipType": "GENERATED_FROM", "relatedSpdxElement": fw_id},
        {"spdxElementId": fw_id,
         "relationshipType": "DEPENDS_ON", "relatedSpdxElement": sdk_id},
        {"spdxElementId": fw_id,
         "relationshipType": "BUILD_TOOL_OF", "relatedSpdxElement": tc_id},
    ]

    if linked_tinyusb(root):
        tu_dir = sdk_dir / "lib" / "tinyusb"
        tu_commit = run("git", "-C", str(tu_dir), "rev-parse", "HEAD")
        tu_id = spdx_id("Package", "tinyusb", tu_commit[:12])
        packages.append({
            "SPDXID": tu_id,
            "name": "tinyusb",
            "versionInfo": tu_commit[:12],
            "downloadLocation": "https://github.com/hathach/tinyusb.git",
            "filesAnalyzed": False,
            "licenseConcluded": "MIT",
            "licenseDeclared": "MIT",
            "copyrightText": "Copyright (c) 2018 hathach (tinyusb.org)",
            "supplier": "Person: hathach",
            "sourceInfo": (
                f"vendored in pico-sdk at lib/tinyusb, pinned at {tu_commit}"
            ),
            "externalRefs": [
                {
                    "referenceCategory": "PACKAGE-MANAGER",
                    "referenceType": "purl",
                    "referenceLocator":
                        f"pkg:github/hathach/tinyusb@{tu_commit}",
                }
            ],
        })
        relationships.append({
            "spdxElementId": fw_id,
            "relationshipType": "DEPENDS_ON",
            "relatedSpdxElement": tu_id,
        })

    document = {
        "spdxVersion": SPDX_VERSION,
        "dataLicense": "CC0-1.0",
        "SPDXID": "SPDXRef-DOCUMENT",
        "name": args.name,
        "documentNamespace": (
            f"https://github.com/{args.repository}/spdx/{args.tag}/{args.name}"
        ),
        "creationInfo": {
            "created": created,
            "creators": [
                f"Tool: I2Console-release-sbom-{SPDX_VERSION}",
                f"Organization: {args.repository.split('/')[0]}",
            ],
        },
        "files": [
            {
                "SPDXID": root_id,
                "fileName": f"./{args.name}",
                "checksums": [
                    {"algorithm": "SHA256", "checksumValue": sha256(payload)}
                ],
                "licenseConcluded": "GPL-3.0-or-later",
                "copyrightText": "Copyright (C) 2025 Metaneutrons",
            }
        ],
        "packages": packages,
        "relationships": relationships,
    }

    out = pathlib.Path(args.output)
    out.write_text(json.dumps(document, indent=2, sort_keys=True) + "\n")
    print(
        f"{out}: {len(packages)} packages, {len(relationships)} relationships, "
        f"created {created}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())

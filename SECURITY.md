# Security Policy

## Supported Versions

I2Console has not had a release yet. Only the current `main` branch is
supported. Once releases exist, this table is filled in with the versions that
receive fixes.

| Version | Supported |
| --- | --- |
| `main` | yes |

## Reporting a Vulnerability

Report privately through GitHub's private vulnerability reporting:

<https://github.com/metaneutrons/I2Console/security/advisories/new>

Please do not open a public issue for a security problem, and do not disclose it
elsewhere before a fix is available.

Include what you have: affected version or commit, the hardware and host setup,
the I2C or USB traffic that triggers it, and what an attacker gains.

## Response Time

- Acknowledgement within 7 days.
- An assessment with a planned fix or an explicit rejection within 30 days.
- Fixes ship in a normal release; the advisory is published once the fix is out.

This is a single-maintainer hobby project. The times above are targets, not a
contractual commitment.

## Scope

The firmware runs on a Waveshare RP2350-GEEK and exposes an I2C slave interface
and two USB-CDC interfaces. Anyone with physical access to the I2C bus or the
USB port can change the configured I2C address, read and write the console
buffers, and put the device into its bootloader. That is by design and is not a
vulnerability. Reports about defects reachable within that model, such as memory
corruption from a crafted I2C transfer, are in scope.

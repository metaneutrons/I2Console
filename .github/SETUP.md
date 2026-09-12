# GitHub Actions Setup

## Required Secrets

### `IDF_COMPONENT_API_TOKEN` — optional

Publishes the ESP-IDF component to the ESP Component Registry. The channel is
**optional**: without the secret the release pipeline reports
`ESP Component Registry not configured; channel skipped` and everything else
proceeds normally.

It belongs in the **`release` environment**, not in the repository secrets. A
repository secret is readable by every workflow in the repository; the
environment is restricted to `v*` tags. `scripts/ci/check-preflight-permissions.py`
fails the build if that ever stops being true.

Obtain a token at <https://components.espressif.com/settings/tokens>, then:

```bash
gh secret set IDF_COMPONENT_API_TOKEN --env release --repo metaneutrons/I2Console
```

`gh secret set` reads the value from stdin when none is given, so the token does
not end up in the shell history or in a process argument.

Nothing else needs a secret. The GitHub release itself uses the automatically
provided token, and cosign and the provenance attestation both use the Actions
OIDC identity.

## Workflows

### CI Workflow (`ci.yml`)

**Triggers:**
- Push to `main` branch
- Pull requests to `main` branch

**Jobs:**
- Build I2Console firmware (RP2350)

### Release Workflow (`release.yml`)

**Triggers:**
- Push tags with `v` prefix (e.g., `v0.1.0`)
- Manual workflow dispatch with version input

**Jobs:**
1. Create tag (manual trigger only)
2. Build firmware and create GitHub release

**Manual Release:**

1. Go to Actions → Release → Run workflow
2. Enter version (e.g., `0.1.0` without `v` prefix)
3. Click "Run workflow"

This will:
- Create tag `v0.1.0`
- Build firmware
- Create GitHub release with artifacts

**Tag-based Release:**

```bash
git tag v0.1.0
git push origin v0.1.0
```

This automatically triggers the release workflow.

## Security Notes

⚠️ **Never commit API tokens directly to the repository!**

The token is stored securely in GitHub Secrets and only accessible to workflows.

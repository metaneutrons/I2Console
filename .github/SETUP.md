# GitHub Actions Setup

## Required Secrets

None. The release workflow publishes only GitHub release assets and uses the
automatically provided token. The ESP Component Registry channel and its
`IDF_COMPONENT_API_TOKEN` secret were removed when the component was mothballed;
see issue #6.

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

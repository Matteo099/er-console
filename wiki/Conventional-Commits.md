# Conventional Commits

Use this format for new commits and pull request titles:

```text
type(optional-scope)!: short description

Optional explanation.

BREAKING CHANGE: explain the incompatible change and migration.
```

| Example | Release impact |
| --- | --- |
| `fix(audio): prevent a source from restarting` | Patch: 1.2.3 -> 1.2.4 |
| `perf(ui): reduce scene allocation` | Patch |
| `feat(settings): add language selection` | Minor: 1.2.3 -> 1.3.0 |
| `feat(api)!: replace the game schema` | Major: 1.2.3 -> 2.0.0 |
| `docs: explain Wi-Fi setup` | No release by itself |
| `ci: cache PlatformIO packages` | No release by itself |
| `build`, `test`, `refactor`, `style`, `chore` | No release by themselves |

For breaking changes, include `!` and a `BREAKING CHANGE:` footer that describes
the migration. Several commits produce one release using the highest required
increment. Before the first release, Semantic Release starts at `1.0.0`.

Keep headers within 100 characters. Use scopes such as `audio`, `ui`, `lua`,
`storage`, `network`, `portal` or `ci`. Do not place passwords or tokens in
commit messages. Conventional commit parsing uses the same preset in commitlint,
the version analyzer and the release notes generator.

## Pull requests

Give the PR a conventional title, even if the branch contains multiple commits.
Squash merge is recommended so that this title becomes the release-driving
commit. If using a merge commit, the individual feature commits must still
describe their changes conventionally; a generic merge message does not replace
them for version analysis. Changing the PR title reruns validation.

CI validates the PR title and new non-merge commits. Existing history up to
`8c3b294af7f3704f3c597e135fa0701d0bde2be4` is exempt from commitlint, so adopting
the rule does not require rewriting existing branches. This exemption concerns
linting only; Semantic Release analyzes history since the latest release tag.

## Local validation

```sh
npm ci
echo 'feat(audio): mix game music and effects' | npx commitlint
npm run check:commits
```

The final command compares the current branch to `origin/main` outside Actions.
In Actions it uses the event's push/PR base and validates the event's PR title.

## Skipping development builds

GitHub supports `[skip ci]`, `[ci skip]`, `[no ci]`, `[skip actions]` and
`[actions skip]` in commit messages for push and pull request workflows. No custom
skip parser is needed. For pull requests, the instruction must be in the HEAD
commit. Skipping a required workflow can leave its checks pending and block a
merge; push a new non-skipped commit to run those checks again. A skipped push to
`main` does not publish a release. The next non-skipped relevant push can release
the accumulated changes.

Reference: [GitHub workflow skipping](https://docs.github.com/en/actions/how-tos/manage-workflow-runs/skip-workflow-runs).

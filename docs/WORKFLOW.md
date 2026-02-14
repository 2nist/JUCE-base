# Workflow Concepts (Plain Language)

## The short loop

1. Change code in `src/`.
2. Build (`.\scripts\build.ps1`).
3. Open REAPER and load the updated plugin.
4. Validate MIDI behavior on a short repeatable test clip.
5. Run `.\scripts\test.ps1` before committing.

This tight loop is how you ship reliable audio tools quickly.

## Three levels of testing

1. **Smoke test**: does it build and load?
2. **Musical test**: does it behave correctly with real MIDI clips?
3. **Stability test**: can it run for a long session without glitches/crashes?

## Best practices for MIDI tools (like `2nist`)

- Keep timing deterministic. Avoid hidden random behavior unless it is a feature.
- Keep processing lock-free in `processBlock`.
- Treat state as a contract. Version your saved state data.
- Add one feature at a time and test each one with fixed MIDI fixtures.
- Keep latency reporting honest if you introduce look-ahead logic.

## Git workflow for plugin experiments

1. Branch from `main`: `git checkout -b exp/midi-idea-name`
2. Keep commits focused:
   - one feature
   - one bug fix
   - one refactor
3. Before push:
   - build debug
   - run tests
   - host test in REAPER
4. Merge back only when behavior is repeatable and documented.


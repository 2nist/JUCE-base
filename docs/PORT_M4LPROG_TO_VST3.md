# M4LProg -> VST3 Port Strategy (REAPER-first)

## 1) What the current repo does

After reviewing `M4LProg`, the app is split into:

- Core music/progression logic (portable):
  - `src/services/musicTheory/MusicTheoryEngine.ts`
  - `src/services/progression/ProgressionManager.ts`
  - `src/services/output/ArrangementOutput.ts`
- App/UI/state shell (not portable as-is to JUCE):
  - `src/components/*` (React UI)
  - `src/stores/*` (Zustand)
- Host bridge (Ableton/Max/Electron specific):
  - `src/services/live/OSCService.ts`
  - `src/stores/liveStore.ts`
  - `electron/main.ts`, `electron/preload.ts`
  - `m4l-helper/*`
- Tests proving core logic behavior:
  - `tests/unit/arrangement_output.test.ts`

## 2) Port goal

Create a JUCE VST3 MIDI effect plugin that:

- Generates/arranges chord and pattern MIDI internally.
- Outputs MIDI directly to host (REAPER), no OSC/Max/Ableton dependency.
- Keeps data model parity with M4LProg where useful (sections, blocks, modes, event rendering).

## 3) Keep / Replace / Drop

- Keep (port to C++):
  - Music theory engine
  - Pattern/progression manager
  - Arrangement event rendering
  - MIDI file export logic (optional but high value)
- Replace:
  - Zustand stores -> `AudioProcessorValueTreeState` + `ValueTree`
  - WebMIDI scheduling -> JUCE `processBlock` MIDI scheduling
  - Electron file dialogs -> JUCE `FileChooser`
- Drop:
  - OSC bridge and M4L helper
  - Ableton Live track/transport APIs

## 4) Target VST3 architecture

- `Engine` (pure C++, no JUCE UI dependency):
  - `MusicTheoryEngine`
  - `PatternLibrary`
  - `ProgressionModel` (sections, blocks, modes)
  - `ArrangementRenderer` (arranged events -> timestamped note on/off events)
- `PluginProcessor`:
  - Owns engine/model
  - Reads host transport via `AudioPlayHead`
  - Emits MIDI to output buffer
  - Handles deterministic playback state
- `PluginEditor`:
  - Start with minimal editor (mode/key/pattern/preview/send)
  - Expand toward timeline UI after engine parity
- `State`:
  - Serialize preset/session with versioned schema (`ValueTree`)

## 5) REAPER-first workflow

1. Insert plugin on a MIDI track.
2. Route plugin MIDI output to an instrument track (or same track instrument after plugin).
3. Enable loop and transport sync in REAPER.
4. Verify:
   - Start/stop follows host transport
   - Note timing is sample-accurate across loop boundaries
   - No hanging notes on stop/seek

## 6) Migration phases

## Phase A - Engine parity (no UI dependency)

- Port `MusicTheoryEngine.ts` and `ProgressionManager.ts` to C++.
- Port `buildArrangedChordEvents` from `ArrangementOutput.ts`.
- Build unit tests in C++ with fixtures matching TS test vectors.

Deliverable: C++ engine library + test executable.

## Phase B - MIDI event scheduler in plugin

- Implement timeline cursor and event queue in `processBlock`.
- Support:
  - gate percent
  - strum offset (ms -> samples)
  - per-block MIDI channel
- Add all-notes-off safety on stop/bypass/unload.

Deliverable: VST3 sends correct MIDI in REAPER transport.

## Phase C - Minimal editor and preset/state

- Add controls for key/mode/pattern/root/repeats/channel.
- Save/load JSON snapshot compatible with M4LProg concepts where practical.
- Add preset migration version tag.

Deliverable: usable composition workflow inside plugin.

## Phase D - Advanced parity

- Mode lanes (`harmony/drum/other`)
- Pattern detection and custom pattern save/load
- MIDI file export from arranged events

Deliverable: feature-complete replacement for the Electron generation path.

## 7) Technical risks and controls

- Real-time safety risk:
  - Keep `processBlock` lock-free; precompute arrangement/event queues off audio thread.
- Transport edge cases:
  - Explicit handling for loop wrap, seek jumps, and tempo changes.
- State drift between UI and engine:
  - Single source of truth in `ValueTree` and deterministic rebuild of event queue.
- Note-off correctness:
  - Central active-note tracker per channel/note + panic path.

## 8) First implementation slice (recommended)

Build this first, end-to-end:

1. Fixed 4-chord progression parameterized by key/mode.
2. Host-synced playback with note on/off and gate.
3. REAPER validation project with loop and tempo automation.
4. Unit tests for event timing and note-off guarantees.

This gives a stable VST3 core quickly, then you can layer full `2nist` arranger UX.


# Instructions for Claude

Standing instructions for working in this repository. Record new ones here,
not in Claude's memory: memory does not travel between computers.

## What may be committed

- Never commit anything proprietary. That includes the shareware Volkov
  Commander distribution and any commercial tool. If the work needs such a
  file, download it at run time from its official source, pinned by SHA-256
  (see `harness/deps.py`).
- Describe what is in the repository. Do not name or document tools that
  are neither stored here nor fetched by its scripts.
- The project is called Vibe Commander. The name of the checkout directory
  or of the remote may be temporary; never write either into source,
  documentation or commit messages.

## History

- The history is meant to be read: one deliberate, self-contained commit per
  step, described accurately. Get a commit right before it is pushed rather
  than fixing it afterwards.

## The original source

- The files in `src/` are CP866 text with CRLF line endings. Edit them as
  bytes; do not let an editor or tool re-encode them or change line endings.
- The first commit holds the source exactly as its author published it, so
  `git diff` against that commit shows every change made since.

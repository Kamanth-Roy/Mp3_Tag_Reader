# MP3 Tag Reader / Editor

A command-line tool written in C that reads and edits ID3v2.3 metadata
(title, artist, album, year, genre, comment) embedded in MP3 files.

## Project Overview

MP3 files don't just store audio — they can carry metadata like the
song title, artist, and album inside the file itself, in a block
called an **ID3 tag**. Media players read this block to show you
"Now Playing" details. This project implements, from scratch in C,
a tool that can:

1. **Parse** that metadata block directly from the raw bytes of an
   MP3 file (no external tag-reading library is used), and
2. **Modify** individual fields in place, by writing new bytes back
   into the same tag block.

The project specifically targets the **ID3v2.3** tag format, one of
the two unrelated metadata formats MP3 files can use (the other,
older one is ID3v1, a fixed 128-byte block at the *end* of the file).
ID3v2.3 sits at the *start* of the file and is made of a sequence of
self-describing chunks called **frames** — one frame per field
(title, artist, and so on), each with its own 4-character ID (like
`TIT2` for title) and its own byte length.

Understanding the project really comes down to understanding three
things: how the tag header is laid out, how frames are chained one
after another, and why editing a frame in place has to respect the
space that frame already has.

## Key Points to Understand the Project

- **The 10-byte tag header.** Every ID3v2.3 tag begins with 10 fixed
  bytes: the text `"ID3"`, a version byte (`3` for this project), a
  revision byte, a flags byte, and 4 bytes that encode the total size
  of everything that follows. `view_mp3()` and `edit_tag()` both
  start by reading exactly these 10 bytes and checking the first
  three are `"ID3"` and the version byte is `3` — if either check
  fails, the file is rejected before anything else is touched.

- **The size bytes are "synchsafe", not a normal number.** The ID3
  spec stores the tag's total size using only 7 bits of each of the
  4 size bytes (the 8th bit of each byte is always kept 0). This
  project decodes it with
  `((b6 & 0x7F) << 21) | ((b7 & 0x7F) << 14) | ((b8 & 0x7F) << 7) | (b9 & 0x7F)`.
  Treating those 4 bytes as a plain 32-bit big-endian number instead
  would read the wrong size for almost every real file.

- **Frames are read one after another, not by name lookup.**
  There's no index or table of contents inside the tag — to find the
  `TIT2` (title) frame, the code has to walk from the start of the
  tag, reading a 10-byte frame header (4-byte ID + 4-byte size + 2
  flag bytes) at a time, and either match the ID or skip over
  `frame_size` bytes to reach the next one. This is why both
  `view_mp3()` and `edit_tag()` contain the same kind of loop: they
  are both doing this same walk, just doing something different
  (print vs. overwrite) when they find the frame they're looking for.

- **A frame header's first byte of `0` means "padding, stop here".**
  Real encoders often leave some empty space after the last frame so
  future edits have somewhere to grow without rewriting the whole
  file. The loop treats a frame ID starting with a null byte as the
  end of real data, not an error.

- **The tag size is cross-checked against the actual file size.**
  A tag can *claim* any size in its header, even one larger than the
  file actually is (either from corruption or a truncated download).
  Before reading or editing any frame, this project checks that the
  declared tag size would actually fit inside the file, and refuses
  to continue if it wouldn't — this avoids reading garbage or writing
  past the real end of the file.

- **Editing never resizes a frame — only overwrites it.** This is
  the most important design decision in `edit.c`. Each text frame
  (`TIT2`, `TPE1`, etc.) has a fixed size already set in the file.
  When you edit a field, the new value is written starting at byte
  `1` of that frame's data (byte `0` is a text-encoding flag, kept
  as `0` for plain text), and the rest of the frame is zero-padded.
  If the new value is *longer* than the frame's existing size, the
  edit is rejected outright rather than attempted — growing a frame
  would mean shifting every byte after it, including the audio data,
  which this tool intentionally does not do. This keeps the editor
  simple and safe, at the cost of not being able to set a value
  longer than whatever was there originally.

- **The comment frame (`COMM`) has one extra layer of structure.**
  Unlike a plain text frame, a comment frame's data is
  `encoding byte + 3-byte language code + description + the comment
  text itself`. `edit_comment()` and `print_comment()` both have to
  skip over the encoding byte, the language code, and the
  (here, empty) description before reaching the actual comment text.

- **`get_value()` exists because shell arguments are split on
  spaces.** A command like `-t Vikram Title Track` arrives as three
  separate `argv` entries, not one string. `get_value()` rejoins
  everything between the edit option and the filename back into a
  single value with single spaces between words, so the user doesn't
  have to wrap multi-word values in quotes.

## Features

- View all ID3v2.3 tag fields in a file: title, artist, album, year,
  music/genre, and comment.
- Edit any of those fields directly from the command line.
- Multi-word values can be typed without quotes
  (`-t Vikram Title Track` becomes `"Vikram Title Track"`).
- Validates the file before touching it: checks for the `ID3` header,
  confirms the version is 2.3, and verifies the declared tag size
  actually fits inside the file before reading or writing anything.
- Edits are written in place; a new value is only accepted if it fits
  inside the existing frame, so the rest of the file (audio data,
  other tags) is never disturbed.

## Build

```bash
gcc *.c -o a.out
```

Requires a C compiler (gcc). No external libraries needed.

## Usage

### View a tag

```bash
./a.out -v sample.mp3
```

Example output:

```
ID3 VERSION : 2.3.0
TAG SIZE    : 81850 bytes
------------------------------------------------------------
TITLE   : Vikram Title Track
ARTIST  : Anirudh Ravichander
ALBUM   : Vikram (2022)
YEAR    : 2022
MUSIC   : Tamil
COMMENT : Elivation
```

### Edit a tag

```bash
./a.out -e <option> <new value> <file>
```

| Option | Field edited |
|--------|--------------|
| `-t`   | Title |
| `-a`   | Artist |
| `-A`   | Album |
| `-y`   | Year |
| `-m`   | Music / Genre |
| `-c`   | Comment |

Examples:

```bash
./a.out -e -t Vikram Title Track sample.mp3
./a.out -e -a Anirudh Ravichander sample.mp3
./a.out -e -A Vikram 2022 sample.mp3
./a.out -e -y 2022 sample.mp3
./a.out -e -m Tamil sample.mp3
./a.out -e -c Elivation sample.mp3
```

### Help

```bash
./a.out -h
```

## File structure

```
.
├── main.c      # Argument parsing and program entry point
├── menu.c      # Usage and help text
├── view.c      # Reads and prints ID3v2.3 tag data
├── edit.c      # Edits ID3v2.3 tag data
└── header.h    # Shared constants, struct, and function prototypes
```

## Notes and limitations

- Only **ID3v2.3** tags are supported (not ID3v1/v1.1, ID3v2.2, or ID3v2.4).
- Text is read and written as plain ASCII; non-English characters
  stored with Unicode encodings are not decoded.
- A new value must fit inside the size of the existing frame. If it's
  longer, the edit is rejected with an error and the file is left
  unchanged — the tag frame is not resized or moved.
- Track number and embedded album art are not displayed or edited.

## Error handling

The tool checks for and reports, without modifying the file:

- File can't be opened
- No `ID3` tag present
- ID3 version other than 2.3
- Corrupted or inconsistent tag size (larger than the file itself)
- Requested tag not found in the file
- New value too long for the existing frame

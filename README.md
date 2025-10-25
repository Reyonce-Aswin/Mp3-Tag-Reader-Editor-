# MP3 Tag Reader – Editor

A C-based command-line tool to **read and edit ID3v1 and ID3v2 MP3 tags**.  
This project demonstrates low-level file handling, metadata parsing, and tag manipulation using the C language.

---

## 📘 Overview

This program allows users to:

- Read **ID3v1** and **ID3v2.x** tags from `.mp3` files.
- Display metadata such as title, artist, album, year, track, and more.
- Edit specific tag fields directly from the terminal.
- Validate year and tag structure.
- Color-coded terminal output for better readability.

---

## 🧩 Features

✅ Read **ID3v1** and **ID3v2** tags  
✅ Edit individual tag frames (e.g., TIT2, TPE1, TALB, etc.)  
✅ Detect MP3 version automatically  
✅ Cross-platform (Linux/Windows)  
✅ Clean modular code with reusable functions  
✅ ANSI color output via `colour.h`

---

## 🗂️ File Structure

MP3-Tag-Reader-Editor/
├── main.c          # Entry point – parses arguments and controls flow
├── tag_read.c      # Handles reading ID3v2 tags and printing metadata
├── tag_edit.c      # Allows editing of tag frames and writing updates
├── tag_utils.c     # Utility functions (byte conversion, validation, etc.)
├── tag_v1.c        # Handles reading ID3v1 tag format
├── tag.h           # Common structures, enums, and function prototypes
├── colour.h        # ANSI color/style definitions for console output
└── README.md       # Project documentation

---

## ⚙️ Build Instructions

### 🧱 Compile

```bash
gcc main.c tag_read.c tag_edit.c tag_utils.c tag_v1.c -o mp3tag
```

### ▶️ Run

#### To read tag information

```bash
./mp3tag -v <filename.mp3>
```

#### To edit a tag frame

```bash
./mp3tag -e -<option> <new_value> <filename.mp3>
```

Example:

```bash
./mp3tag -e -t "New Song Title" song.mp3
```

---

## 🧠 How It Works

1. **Tag Detection:**  
   The program reads the file header and determines whether it contains ID3v1 or ID3v2 tags.

2. **Tag Parsing:**  
   Each tag frame (title, artist, album, etc.) is parsed according to ID3 specifications and stored in an `ID3v2Tag` structure.

3. **Editing:**  
   The selected tag frame is located, modified in memory, and rewritten back to the MP3 file.

4. **Validation:**  
   Includes checks for valid file extension, year formatting, and byte order conversions.

---

## 🧰 Dependencies

- Standard C libraries only (`stdio.h`, `string.h`, `ctype.h`)  
- ANSI escape codes for colored output (no external libraries)

---

## 💡 Example Output

```terminal
====================================================================================
|| MP3 TAG METADATA (ID3v2.3)                                                     ||
====================================================================================
|Title       | Shape of You                                                        |
|Artist      | Ed Sheeran                                                          |
|Album       | ÷ (Divide)                                                          |
|Year        | 2017                                                                |
|Composer    | Ed Sheeran                                                          |
|Genre       | Pop                                                                 |
|Track       | 01                                                                  |
|Comment     | Best song ever                                                      |
====================================================================================
```

---

## 🚀 Future Improvements

- Support for **embedded album art** extraction  
- Add **frame removal** and **bulk editing**  
- Extend to **ID3v2.4** advanced frames  
- Implement a GUI or TUI front-end  

---

## 🧑‍💻 Author

**Reyonce Aswin**  
🎓 Embedded Systems Student @ Emertxe Information Technologies, Bangalore   

---


> 💬 *If you find this project useful, give it a ⭐ on GitHub!*

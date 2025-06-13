# Blackhole Project — Concept, Motivation, and Design (Full Spec) (Github project name : PROJECT-BLACKHOLE)

## 📛 Project Name: **Blackhole**

> *"Everything goes in. Nothing escapes — except your entire project."*

---

## 🧠 Core Idea

**Blackhole** is a utility that takes a full source project (e.g., C++ compiler, game engine, etc.) and collapses it into a **single self-contained C++ file** (e.g., `blackhole.cpp`).

When compiled and executed, this file **recreates the original project structure and all contents**, byte-for-byte.

This is not about compression or archiving. This is about:

* Submitting rich projects under arbitrary file-count limits (e.g. university rules)
* Leaving behind a signature move in academic or professional contexts
* Combining creativity, engineering discipline, and terminal drama

---

## 🎯 Motivation

* University submission systems often limit file counts to prevent junk or build artifacts.
* However, this penalizes serious projects (e.g., compiler with 100+ source files and test suites).
* Rather than stripping things down or emailing professors, **Blackhole** is a form of technical rebellion — solving the problem entirely within the rules.
* It also becomes a **legacy tool**: reusable across departments (e.g., Game Dev, Systems, Graphics).

---

## 📦 What Blackhole Does

1. Reads a target project directory (source code, configs, assets, tests — everything).
2. Encodes all file contents and paths into a single C++ source file.
3. Generates `blackhole.cpp`:

   * When compiled, it reproduces the original project exactly.
   * Builds and runs on any system with a C++17+ compiler.
   * No external dependencies, zips, or binaries.

---

## 💡 Core Features

* ✅ Single-file C++ source output
* ✅ Uses `std::filesystem` to recreate directories
* ✅ Escapes all file contents as C++ strings
* ✅ Reconstructs all files with exact relative paths
* ✅ UTF-8 safe (for text-based content)
* ✅ Generates CLI logs during reconstruction

---

## 🖼️ Signature Visual: ASCII Progress Reveal

Instead of a boring loading bar, Blackhole prints a **7-line wide ASCII graphic** (Blackhole name/banner) that is gradually revealed from left to right using terminal cursor control (ANSI escape sequences).

Also put this kind of ascii (variable-coloured) art in both packer (project to file) and unpacker (file to project)



This becomes a terminal animation as the unpacking progresses.
```
░▒▓███████▓▒░░▒▓█▓▒░       ░▒▓██████▓▒░ ░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░      ░▒▓████████▓▒░
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░
░▒▓███████▓▒░░▒▓█▓▒░      ░▒▓████████▓▒░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓██████▓▒░
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░
░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░
░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░░▒▓██████▓▒░░▒▓████████▓▒░▒▓████████▓▒░


OR

 ▄▄▄▄    ██▓    ▄▄▄       ▄████▄   ██ ▄█▀ ██░ ██  ▒█████   ██▓    ▓█████
▓█████▄ ▓██▒   ▒████▄    ▒██▀ ▀█   ██▄█▒ ▓██░ ██▒▒██▒  ██▒▓██▒    ▓█   ▀
▒██▒ ▄██▒██░   ▒██  ▀█▄  ▒▓█    ▄ ▓███▄░ ▒██▀▀██░▒██░  ██▒▒██░    ▒███
▒██░█▀  ▒██░   ░██▄▄▄▄██ ▒▓▓▄ ▄██▒▓██ █▄ ░▓█ ░██ ▒██   ██░▒██░    ▒▓█  ▄
░▓█  ▀█▓░██████▒▓█   ▓██▒▒ ▓███▀ ░▒██▒ █▄░▓█▒░██▓░ ████▓▒░░██████▒░▒████▒
░▒▓███▀▒░ ▒░▓  ░▒▒   ▓▒█░░ ░▒ ▒  ░▒ ▒▒ ▓▒ ▒ ░░▒░▒░ ▒░▒░▒░ ░ ▒░▓  ░░░ ▒░ ░
▒░▒   ░ ░ ░ ▒  ░ ▒   ▒▒ ░  ░  ▒   ░ ░▒ ▒░ ▒ ░▒░ ░  ░ ▒ ▒░ ░ ░ ▒  ░ ░ ░  ░
 ░    ░   ░ ░    ░   ▒   ░        ░ ░░ ░  ░  ░░ ░░ ░ ░ ▒    ░ ░      ░
 ░          ░  ░     ░  ░░ ░      ░  ░    ░  ░  ░    ░ ░      ░  ░   ░  ░
      ░                  ░


OR

      ,.  - · - .,  '                ,.  '                       ,.,   '                   ,. - .,                              _  °        .·¨'`;        ,.·´¨;\          , ·. ,.-·~·.,   ‘             ,.  '                      _,.,  °    
,·'´,.-,   ,. -.,   `';,'           /   ';\                     ;´   '· .,             ,·'´ ,. - ,   ';\           ,.·,       :´¨   ;\        ';   ;'\       ';   ;::\        /  ·'´,.-·-.,   `,'‚           /   ';\               ,.·'´  ,. ,  `;\ '  
 \::\.'´  ;'\::::;:'  ,·':\'       ,'   ,'::'\                  .´  .-,    ';\       ,·´  .'´\:::::;'   ;:'\ '      ,'   ,'\     .'´ ,·´::'\       ;   ;::'\      ,'   ;::';      /  .'´\:::::::'\   '\ °       ,'   ,'::'\            .´   ;´:::::\`'´ \'\  
  '\:';   ;:;:·'´,.·'´\::::';     ,'    ;:::';'                /   /:\:';   ;:'\'    /  ,'´::::'\;:-/   ,' ::;  '    ;'  ,'::\ .·' .·´::::::;'      ;  ;::_';,. ,.'   ;:::';°  ,·'  ,'::::\:;:-·-:';  ';\‚      ,'    ;:::';'          /   ,'::\::::::\:::\:' 
  ,.·'   ,.·:'´:::::::'\;·´      ';   ,':::;'               ,'  ,'::::'\';  ;::';  ,'   ;':::::;'´ ';   /\::;' '      ;  ;::·´ .·´:::::::;·´     .'     ,. -·~-·,   ;:::'; ' ;.   ';:::;´       ,'  ,':'\‚     ';   ,':::;'          ;   ;:;:-·'~^ª*';\'´   
  '·,   ,.`' ·- :;:;·'´          ;  ,':::;' '           ,.-·'  '·~^*'´¨,  ';::;  ;   ;:::::;   '\*'´\::\'  °     ';  '´   ;´::::::;·´         ';   ;'\::::::::;  '/::::;    ';   ;::;       ,'´ .'´\::';‚    ;  ,':::;' '          ;  ,.-·:*'´¨'`*´\::\ '  
     ;  ';:\:`*·,  '`·,  °     ,'  ,'::;'              ':,  ,·:²*´¨¯'`;  ;::';  ';   ';::::';    '\::'\/.'        ;  ;'\   '\::;·´              ;  ';:;\;::-··;  ;::::;     ';   ':;:   ,.·´,.·´::::\;'°   ,'  ,'::;'            ;   ;\::::::::::::'\;'   
     ;  ;:;:'-·'´  ,.·':\       ;  ';_:,.-·´';\‘       ,'  / \::::::::';  ;::';   \    '·:;:'_ ,. -·'´.·´\‘     ;  ;:\:'·.  '·., ,.·';'         ':,.·´\;'    ;' ,' :::/  '     \·,   `*´,.·'´::::::;·´      ;  ';_:,.-·´';\‘     ;  ;'_\_:;:: -·^*';\   
  ,·',  ,. -~:*'´\:::::'\‘     ',   _,.-·'´:\:\‘     ,' ,'::::\·²*'´¨¯':,'\:;     '\:` ·  .,.  -·:´::::::\'    ;_;::'\::`·._,.·'´:\'          \:::::\    \·.'::::;         \\:¯::\:::::::;:·´         ',   _,.-·'´:\:\‘    ';    ,  ,. -·:*'´:\:'\° 
   \:\`'´\:::::::::'\;:·'´       \¨:::::::::::\';     \`¨\:::/          \::\'       \:::::::\:::::::;:·'´'     \::'\:;' '·::\::\:::::'\           \;:·´     \:\::';           `\:::::\;::·'´  °           \¨:::::::::::\';     \`*´ ¯\:::::::::::\;' '
    '\;\:::\;: -~*´‘            '\;::_;:-·'´‘        '\::\;'            '\;'  '       `· :;::\;::-·´           '\::\     `·'\::\;:·'´'                      `·\;'                ¯                       '\;::_;:-·'´‘         \:::::\;::-·^*'´     
             '                     '¨                   `¨'                                                     ¯          ¯'                              '                  ‘                         '¨                    `*´¯              


```

### 🖊️ **Colorization** Idea

In addition to line-by-line and column-by-column reveals, the visual can be enhanced by **per-column coloring** using ANSI color codes.

Each column (or every N columns) can be assigned a unique color, creating a rainbow or gradient animation effect as the ASCII art reveals.

This can be implemented by:

* Defining a list of ANSI color codes
* Cycling through them as each column prints
* Resetting color after each line to avoid bleed

This would turn the ASCII reveal into a vibrant, interactive terminal effect — making the unpacking not just functional, but *artistic*.

---

## 🗂️ Directory Structure

```
blackhole/
├── pack.py             # Python script that generates blackhole.cpp
├── blackhole.cpp       # Output file (generated)
├── README.md           # Documentation and usage
```

---

## 🛠️ Technical Breakdown

* Written in: Python 3 (packer), C++17 (output)
* Uses `os.walk()` to collect files and relative paths
* Escapes content using string replacement (quotes, backslashes)
* Output uses `unordered_map<string, string>` in C++
* Uses `std::ofstream` to write content
* Uses `std::filesystem::create_directories()`
* Terminal animation via `\033[F` (move cursor up) per printed line

---

## ✨ Future Add-ons

* [ ] Optional compression (base64 + zlib)
* [ ] Binary file support
* [ ] Strip or exclude patterns (`.git/`, `*.o`, etc.)
* [ ] CLI mode for `pack.py` with `--output`, `--exclude`, `--ascii` flags
* [ ] Multiple visual styles for banner (via `figlet`)

---

## 📌 When to Use

* Compiler or game dev project too big for university submission
* Want to include a full working snapshot in a single source file
* Want to submit to a contest or peer review with style
* Leaving behind something memorable at university (legacy project)

---

## 🧾 Taglines / Quotes

> *"Collapse Everything. Submit Anything. Reconstruct All."*

> *"One file. All code. No compromise."*

> *"If they won’t raise the file limit, collapse the universe instead."*
 
> "If you wish to make an apple pie from scratch, you must first invent the universe." https://www.youtube.com/watch?v=7s664NsLeFM

---

## 🕒 Project Status

**TODO — To start after compiler project is completed** (in \~2–3 months)

Document prepared to retain full idea until execution begins.

---

## 🧠 Last Notes

This project is not just a tool. It's an act of compression-fueled protest and style.
When someone runs `blackhole.cpp` and sees your entire project — compiler, tests, game — reappear from nothing but a single file… they'll remember.

This *is* your legacy drop.

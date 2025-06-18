# VERTEX_OS
![AGPL v3 License](https://img.shields.io/badge/license-AGPLv3-blue.svg)
![C++](https://img.shields.io/badge/language-C++-brightgreen)
![GTK](https://img.shields.io/badge/UI-GTK-blueviolet)

**VERTEX OS** is a fully simulated desktop operating system built using `C++`, `GTK`, and `Gstramer`. It features a modular, interactive UI and mimics core functionalities of a real OS environment.

🚀 Features
-----------

*   🖥️ Custom Boot Loader with Logo, Animation, and Sound
*   🔐 Login Screen with Password Authentication
*   🌌 Dynamic Wallpaper & UI Theming with CSS
*   🧠 Task Manager showing Real-Time RAM/Disk Usage (with Graphs)
*   📁 File Nest with Navigation
*   📝 Notepad App with File Save/Load
*   🧮 Calculator with Basic Functions
*   🗓️ Calendar with Date Picker
*   🎮 Mini Game (click-based and logic-based)
*   🎲 Random Die Roller 
*   🎵 Audio Player (GStreamer)
*   🎬 Video Player with Seek/Play/Pause and Window Integration
*   🧼 App Overlay & Window Management
*   ❌ App Duplication Prevention + Live Warnings
*   🧠 Simulated RAM/Disk tracking for each app

🛠️ Tech Stack
--------------

*   **C++** – Core Logic and Application Structure
*   **GTK 3** – GUI Framework for All Windows
*   **GStreamer** – Audio and Video Media Support
*   **Cairo** – Custom Graph Rendering (RAM/Disk/CPU)
*   **CSS** – For Styling GTK Widgets (Modern Look)

📂 Project Structure
--------------------

*   `ui.cpp` – Boot Sequence, Login Screen, Desktop
*   `launch.cpp` – Handles App Icons, Desktop Overlay, Backgrounds
*   `task.cpp` – Live System Monitor (Graphs, App Stats)
*   `notepad.cpp`, `calendar.cpp`, `calculator.cpp` – Individual App Modules
*   `video.cpp`, `audio.cpp` – Media Players using GStreamer
*   `globals.h` – Global Declarations, Shared State

📌 Key Highlights
-----------------

*   Real-time Graphs using `Cairo`
*   Proper resource tracking for running apps (RAM/Disk)
*   Prevention of duplicate app instances with visual feedback
*   Custom startup sound and boot animation using `GStreamer`
*   App windows styled with CSS for modern aesthetics

📸 Video
--------------
[Watch VERTES OS demo here](https://screenrec.com/share/MFsSiD8aGh)
*   Right-click or Ctrl+Click to open in new tab

📬 Contact
----------

If you want to contribute or need help, feel free to open an [issue](https://github.com/your-username/vertex-os/issues) or reach out on GitHub!

## 🧾 License

This project is licensed under the **GNU AGPL v3 License**.  
You can use, modify, and redistribute it under the same terms.  
See the [LICENSE](./LICENSE) file for full details.


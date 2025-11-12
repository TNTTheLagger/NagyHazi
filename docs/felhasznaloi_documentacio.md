# User Manual

**Project Title:** Walk3D \
**Author:** *Kaba Kevin Zsolt* \
**Neptune code:** *FF64XM* \
**Date:** *2025 november 12*

---

## 1. Introduction

This program is a **console-based 3D maze explorer**.
It displays a simple 3D environment in text mode using ASCII symbols and allows you to walk around using the keyboard.

You can:

* Load different map files (`.csv`) that describe the environment.
* Move around the map in first-person view.
* Save your current position to a file.
* Pause and resume the game.
* Quit anytime from the in-game menu.

No graphics card or GUI is required — the program runs entirely in the command line window.

---

## 2. System Requirements

### **Operating Systems:**

* Windows 10 or later
* Linux (Debian, Ubuntu, etc.)

### **Minimum Hardware:**

* Dual-core CPU
* 1 GB RAM
* Console terminal with at least 80x24 character display

### **Files Required:**

* The executable file (compiled program)
* One or more `.csv` map files located in the same folder

---

## 3. Starting the Program

1. Open a **terminal** or **command prompt**.
2. Navigate to the folder containing the program.
3. Run the executable:

    * On **Windows**:

      ```
      raycaster.exe
      ```
    * On **Linux**:

      ```
      ./raycaster
      ```

The main menu will appear automatically when the program starts.

---

## 4. Main Menu

When you start the program, you’ll see a centered text-based menu.
You can navigate using the **arrow keys** or **W/S** and confirm with **ENTER**.

### Available options:

| Menu Item    | Description                                                                             |
| ------------ | --------------------------------------------------------------------------------------- |
| **Resume**   | Continue exploring the map (only available if a map is loaded).                         |
| **Load Map** | Open the list of available `.csv` map files to load.                                    |
| **Save Map** | Save your current map and position to a file (`map_saved.csv` or the current map name). |
| **Quit**     | Exit the program safely.                                                                |

To open or close the menu during gameplay, press **ESC**.

---

## 5. Loading Maps

When you select **Load Map**, a new menu appears listing all `.csv` files in the current folder.

* Each file represents a map.
* To go back to the main menu, select **< Back>** at the top of the list.

### Map Format (.csv)

Each line in the file contains map characters separated by commas:

* `#` = wall
* `.` = empty space (walkable floor)
* `X` = player starting position

**Example:**

```
#,#,#,#
#,.,.,#
#,X,.,#
#,#,#,#
```

After selecting a map, the game will start in first-person view from the `X` position.

---

## 6. Controls During Gameplay

| Key     | Action                    |
| ------- | ------------------------- |
| **W**   | Move forward              |
| **S**   | Move backward             |
| **A**   | Turn left                 |
| **D**   | Turn right                |
| **ESC** | Toggle the main menu      |
| **Q**   | Quit the game immediately |

The player’s movement speed and rotation are smooth and time-based, so they adapt to your computer’s performance.

---

## 7. Saving Progress

At any time (via the menu), you can choose **Save Map** to store your current map.
The map will be written to a CSV file with your current position marked as `X`.
If you have loaded a map file, it overwrites that map; otherwise, it creates `map_saved.csv`.

---

## 8. Understanding the Display

The display uses **ASCII characters** to simulate a 3D environment:

| Symbol                   | Meaning                                                |
| ------------------------ | ------------------------------------------------------ |
| `#`, `@`, `M`, etc.      | Wall surfaces at different distances (darker = closer) |
| `.`, `'`, `,`, `~`, etc. | Floor texture gradient                                 |
| *(space)*                | Empty air above the horizon                            |

The closer a wall is, the darker its shading character will appear.

---

## 9. Exiting the Program

You can exit in two ways:

1. From the menu → **Quit**
2. In-game → press **Q**

All memory is automatically freed, and the console will return to normal state.

---

## 10. Troubleshooting

| Problem                                  | Possible Cause                                 | Solution                                              |
| ---------------------------------------- | ---------------------------------------------- | ----------------------------------------------------- |
| No maps appear when selecting “Load Map” | No `.csv` files found in the current directory | Add map files next to the executable                  |
| Player spawns inside a wall              | Invalid map layout                             | Edit your `.csv` to ensure `X` is in open space (`.`) |
| Menu not visible or distorted            | Terminal too small                             | Resize your console window to at least 80x24          |
| Program closes immediately               | Missing or invalid map file                    | Check file permissions and contents                   |

---

## 11. Tips for Creating Custom Maps

You can design your own maps using any text editor (like Notepad or VS Code):

1. Use `#` for walls, `.` for floor, and `X` for start.
2. Make sure each row has the same number of columns.
3. Save it as `yourmap.csv` in the same folder as the program.

Then select it from the **Load Map** menu.


# DarkTaskDialog
Win32 TaskDialog Dark Theme Header-Only Library  

<img src="https://github.com/SFTRS/DarkTaskDialog/assets/28443945/ae45505c-bca0-4c31-853c-9c6d88eea3c4" width="547px">

## Project
Windows 10 and Windows 11 lack support for the dark mode in the TaskDialog API. This project adds support for the dark theme to all its features without losing functionality, including callbacks and multiple pages.

## How it is done
- Detouring of DrawTheme... APIs
- Windows subclassing
- A lot of research and testing
- **No undocumented functions are used**

## Dependencies
- [Microsoft Detours](https://github.com/microsoft/Detours) - MIT licensed and lightweight

## How to use
1. Include `DarkTaskDialog.hpp` in your project.
2. Call `SFTRS::DarkTaskDialog::setTheme(dark|light)` before displaying your first task dialog.
3. Call the function again with the other theme when you need to switch it.

## License
GPL3


© Softros Systems

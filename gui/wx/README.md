# DiskShed

DiskShed is a wxWidgets desktop application for browsing and editing OS-9 RBF
and Disk BASIC images. It deliberately builds the existing ToolShed C filesystem
sources without changing the repository's current Makefiles.

## Build

Install CMake and wxWidgets 3.2 or newer, then run:

```sh
cmake -S gui/wx -B build/wx -DCMAKE_BUILD_TYPE=Release
cmake --build build/wx
```

On macOS with Homebrew, wxWidgets can be installed with:

```sh
brew install wxwidgets
```

On macOS, run `open build/wx/DiskShed.app`. On Windows, run `DiskShed.exe`; on
Linux, run `DiskShed` from the build directory. If wxWidgets is unavailable,
CMake still builds `toolshed-image-smoke`, which exercises the same image adapter
from a terminal:

```sh
build/wx/toolshed-image-smoke path/to/image.dsk
build/wx/toolshed-image-smoke path/to/image.dsk CMDS
build/wx/toolshed-image-smoke path/to/image.dsk --extract CMDS/shell shell
```

DiskShed supports opening an image, format detection, nested OS-9 directory
navigation, Back/Up/Refresh commands, native file export, file-size lookup, and
basic Disk BASIC type descriptions. The table adapts to the image format: OS-9
shows Size, Type, Attributes, Owner, and Modified, while Disk BASIC shows Size,
File Type, and Encoding. Double-click a folder to enter it or a file to export
it.

Opening another image creates another native window, so OS-9 and Disk BASIC
images can be browsed side by side. The first image reuses the initial empty
window. Multiple image files can be selected and dragged out together; folders
in a mixed selection are skipped.

Editing operations include file import, rename, delete, and OS-9 directory
creation. Before the first mutation in an open session, the adapter copies the
original image to a sibling file named `IMAGE.diskshed-backup` (or a numbered
variant if that name already exists). The current image is then edited in place.

Drag and drop works in both directions:

- Drop regular host files from Finder, Explorer, or a Linux file manager onto
  the file list to import them into the currently displayed image directory.
- Drag one or more selected image files from the list onto the native file
  system to export copies.
  The GUI materializes them in a private temporary directory for the duration of
  the app session so the operating system receives an ordinary file URL.

Host directories are ignored when dropped. Disk BASIC images remain flat and
therefore do not enable **New Folder**.

The mutation adapter also has a small command-line smoke utility:

```sh
build/wx/toolshed-image-edit-smoke image.dsk import host-file IMAGE_DIRECTORY
build/wx/toolshed-image-edit-smoke image.dsk mkdir IMAGE_DIRECTORY NAME
build/wx/toolshed-image-edit-smoke image.dsk rename IMAGE_PATH NEW_NAME
build/wx/toolshed-image-edit-smoke image.dsk delete IMAGE_PATH file
```

An image path may also be passed to the GUI executable on the command line.

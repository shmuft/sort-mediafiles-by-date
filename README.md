# SortImagesByDate

A cross-platform desktop application for organizing media files by their creation date. The application automatically sorts images, videos, and related metadata files into a structured directory hierarchy based on the date the media was created.

## Features

- **Automatic Date Detection**: Extracts creation dates from:
  - EXIF metadata for images
  - MOV, MP4, AVI, 3GP video metadata
  - XMP sidecar files
  - Filename patterns (e.g., "20230622_123456.jpg")
  - File modification time (optional)

- **Smart File Organization**:
  - Organizes files into Year/Month/Day folders
  - Separate directories for images and videos
  - Preserves original filenames
  - Handles duplicate files

- **Supported File Types**:
  - Images: Any format with EXIF support
  - Videos: MOV, MP4, AVI, 3GP
  - Metadata: XMP, THM files

- **User Interface**:
  - Qt-based GUI for easy interaction
  - Progress tracking with percentage complete
  - Command-line interface for automation

## Requirements

- Go 1.16 or higher
- Qt 6.x/5.x
- CMake 3.5 or higher
- C++ compiler with C++17 support (e.g., MSVC, GCC)

## Building from Source

1. Install prerequisites:
   - Go compiler
   - Qt development tools
   - CMake
   - C++ compiler (MSVC on Windows)

2. Clone the repository:
   ```bash
   git clone https://github.com/shmuft/SortImagesByDate.git
   cd SortImagesByDate
   ```

3. Build using CMake:
   ```bash
   cmake -S . -B build
   cmake --build build
   ```

## Usage

### GUI Application

Launch `sort-media-gui.exe` and:
1. Select source directory with media files
2. Choose destination directories for images and videos
3. Configure sorting options
4. Start the sorting process

### Command Line

```bash
sort-media.exe [options]
```

Options:
- `--source_dir`: Source directory (default: /temp)
- `--export_dir`: Export directory for images (default: /temp2)
- `--video_export_dir`: Export directory for videos (default: /tempVideo)
- `--use_mod_time_as_created`: Use modification time when no date found
- `--sync_std_in_out`: Synchronize standard input/output

## License

MIT License

Copyright (c) 2025 Ivan Dolgov (shmuft) 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. 

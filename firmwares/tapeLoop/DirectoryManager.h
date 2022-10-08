#ifndef DirectoryManager_h
#define DirectoryManager_h

#include <SD.h>
#include <Adafruit_SSD1306.h>

#define MAX_DIR   32
#define MAX_FILE  32
#define MAX_TOTAL MAX_DIR + MAX_FILE

String BACK_DIR = "..";

class DirectoryManager {

private:
  bool   _initialized = false;
  String _currentPath;
  int    _depth;
  int    _positions[32];

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  bool _isSupportedFileType (String fileName) {
    fileName.toUpperCase();
    return fileName.endsWith(".WAV");
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void _scanDirectory () {
    dirCount  = 0;
    fileCount = 0;

    unsigned int displayIndex = 0;

    if (_depth > 0) {
      dirNames[dirCount++] = BACK_DIR;
    }

    File directory = SD.open(_currentPath.c_str(), FILE_READ);

    while (true) {
      File file = directory.openNextFile(FILE_READ);
      if (!file) break;

      String fileName = file.name();

      if (file.isDirectory()) {
        if (dirCount < MAX_DIR) {
          dirNames[dirCount++] = fileName;
        }
      } else {
        if (fileCount < MAX_FILE && _isSupportedFileType(fileName)) {
          fileNames[fileCount++] = fileName;
        }
      }

      file.close();
    }

    totalItems = dirCount + fileCount;
    directory.close();

    // Update displayed names and convert file names to path
    String path = _currentPath.c_str();
    path.concat("/");

    for (int i = 0; i < dirCount;  ++i) _addDisplayName(dirNames[i],  displayIndex++);
    for (int i = 0; i < fileCount; ++i) {
      _addDisplayName(fileNames[i], displayIndex++);
      String fullPath = path.c_str();
      filePaths[i] = fullPath.concat(fileNames[i]);
    }
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void _addDisplayName (String fileName, unsigned int index) {
    if (!_initialized) _initializeDisplayNames();

    unsigned int len = fileName.length();
    for (unsigned int i = 0; i < 21; ++i) {
      if (i > len) displayNames[index].setCharAt(i, ' ');
      else         displayNames[index].setCharAt(i, fileName.charAt(i));
    }
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void _initializeDisplayNames () {
    for (unsigned int i = 0; i < MAX_TOTAL; ++i) {
      String blankName = "                     "; // 21 char
      displayNames[i] = blankName;
    }
    _initialized = true;
  }

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄

public:
  String dirNames[MAX_DIR];
  String fileNames[MAX_FILE];
  String filePaths[MAX_FILE];
  String displayNames[MAX_TOTAL];
  int    dirCount;
  int    fileCount;
  int    totalItems;

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void openRootDirectory () {
    _currentPath = "/";
    _depth       = 0;
    _scanDirectory();
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void openDirectoryAtIndex (int index) {
    if (_depth > 0) {
      if (index == 0) {
        openParentDirectory();
        return;
      }
    }

    if (index >= dirCount) return;

    // save position in parent directory
    _positions[_depth] = index;

    // update current path
    String dirName = dirNames[index];
    if (_depth > 0) _currentPath.concat("/");
    _currentPath.concat(dirName);

    // scan directory content
    _depth += 1;
    _positions[_depth] = 0;
    _scanDirectory();
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  int getPosition () {
    return _positions[_depth];
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void openParentDirectory () {
    if (_depth == 0) return;
    if (_depth == 1) {
      _depth = 0;
      _currentPath = "/";
      _scanDirectory();
      return;
    }

    unsigned int i = _currentPath.length();
    while (_currentPath.charAt(i) != '/') i--;
    _currentPath = _currentPath.substring(0, i);
    _depth -= 1;
    _scanDirectory();
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  String getFilePathAtIndex (int i) {
    i -= dirCount;
    if (i < 0) i = 0;
    return filePaths[i];
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void displayFiles (Adafruit_SSD1306* display, long cursor) {
    display->clearDisplay();
    display->setCursor(0, 0);

    // Calculate position of the cursor in the screen.
    // The screen is displaying 8 items maximum.
    int min = cursor - 4;
    if (min > totalItems - 8) {
      min = totalItems - 8;
    }
    if (min < 0) min = 0;

    // Drawing the list of items
    int line = 0;
    for (int i = min; i < totalItems; ++i) {
      if (++line > 8) break;
      String fileName = displayNames[i];
      if (cursor == i) {
        display->setTextColor(BLACK, WHITE); // 'inverted' text
      } else {
        display->setTextColor(WHITE);
      }
      display->println(fileName);
    }

    // Drawing scroller
    if (totalItems > 8) {
      long scrollerSize = 8 * 64 / totalItems + 1;
      long scrollerPosition = min * 64 / totalItems;
      display->drawRect(127, scrollerPosition, 1, scrollerSize, SSD1306_WHITE);
    }

    // Commit change to screen
    display->display();
  }

};

#endif // DirectoryManager_h

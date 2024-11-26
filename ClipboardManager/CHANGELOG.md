# Change log
## New
- Added setting to change "show window" shortcut (by default alt + space).
- Overlay mode (button on the top left of the window) now hides the application's window. When enabled the application will behave like a system overlay, being shown when hitting the "Show window" shortcut and hiding after losing focus.
    - If the hot key/shortcut fails to be enabled, the application will not hide.
- Search bar in actions page.

## Changes
- Added guard to prevent against duplicated clipboard events (bug from Chromium's copy).
- Removed OCR and clipboard history.
- Better explained settings.
- New setting category: Developer.

## Bugs

## TODO
- [ ] Tooltips (I18Ned).
- [ ] Add translated error messages for regex errors :
    - [ ] Code 8 - "Found a closing ) with no corresponding opening parenthesis."
    - [ ] Code 7 - "Unmatched [ or [^ in character class declaration."
    - [ ] Code 4 - "Invalid character class name, collating name, or character range."
    - [ ] Code 13 - "The repeat operator "*" cannot start a regular expression."
- [ ] Translate error messages (and add them to the .resw files) :
- [ ] System tray (https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona)
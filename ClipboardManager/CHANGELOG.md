# Change log
## New
- Added setting to change "show window" shortcut (by default alt + space).

## Changes
- Added guard to prevent against duplicated clipboard events (bug from Chromium's copy).
- Removed OCR and clipboard history.

## Bugs

## TODO
- [ ] Tooltips (I18Ned).
- [ ] Add translated error messages for regex errors :
    - [ ] Code 8 - "Found a closing ) with no corresponding opening parenthesis."
    - [ ] Code 7 - "Unmatched [ or [^ in character class declaration."
    - [ ] Code 4 - "Invalid character class name, collating name, or character range."
    - [ ] Code 13 - "The repeat operator "*" cannot start a regular expression."
- [ ] Translate error messages (and add them to the .resw files) :
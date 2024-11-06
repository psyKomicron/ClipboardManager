# Change log
## New

## Changes
- Added guard to prevent against duplicated clipboard events (bug from Chromium's copy).

## Bugs

## TODO
- [X] Implement clipboard triggers removal from clipboard actions.
- [ ] Tooltips (I18Ned).
- [ ] Add translated error messages for regex errors :
    - [ ] Code 8 - "Found a closing ) with no corresponding opening parenthesis."
    - [ ] Code 7 - "Unmatched [ or [^ in character class declaration."
    - [ ] Code 4 - "Invalid character class name, collating name, or character range."
    - [ ] Code 13 - "The repeat operator "*" cannot start a regular expression."
- [ ] Translate error messages (and add them to the .resw files) :

- [X] Add format checking/validation for `ClipboardActionView`.
- [ ] Implement copy and pin buttons for clipboard history items.
# Change log
## Changes
- Fixed clipboard triggers not properly saving, not being able to delete actions, not saving regex ignore case and algorithm matching mode.
- ~~Fixed matching triggers history duplication.~~
- Improved trigger modification.
- Now displaying if the trigger is using search mode and/or ignore case.

## New
- Add new clipboard triggers without interacting directly with the XML file. XML file editing is still possible.
- Added tooltips for quick settings buttons (`/Actions page`).

## Bugs

## TODO
- [X] Implement "import from clipboard".

- [ ] Implement `/Actions page/Clear actions`.
- [X] Implement `/Triggers page/Test regex`.
- [ ] Implement clipboard triggers removal from clipboard actions.
- [ ] Tooltips (I18Ned).
- [ ] Lower spacing between blocks `/Actions page/quick settings`.
- [ ] Add translated error messages for regex errors :
    - [ ] Code 8 - "Found a closing ) with no corresponding opening parenthesis."
    - [ ] Code 7 - "Unmatched [ or [^ in character class declaration."
    - [ ] Code 4 - "Invalid character class name, collating name, or character range."
    - [ ] Code 13 - "The repeat operator "*" cannot start a regular expression."
- [ ] Translate error messages (and add them to the .resw files) :
    - [ ] StringFormatError_MissingOpener
    - [ ] StringFormatError_MissingCloser
    - [ ] StringFormatError_MissingBraces
    - [ ] StringFormatErorrMissingValue
    - [ ] StringRegexError_Invalid
    - [ ] StringRegexError_EmptyString
    - [ ] TriggerError_FailedToReload
# Change log
## New
- Clipboard history will now show images stored in the clipboard.
- OCR will be performed on images stored in the clipboard and when an image is copied into it. If the text matches a trigger, matching actions will be created.

## Changes
- Implemented "Add duplicated actions" toggle in settings page.
- Fixed mismatch between settings defaulting "Allow notifications" to true and the rest of the application defaulting to false.
- Now defaulting "Add duplicated actions" to true.

## Bugs

## TODO
- [ ] Implement `/Actions page/Clear actions`.
- [ ] Implement clipboard triggers removal from clipboard actions.
- [ ] Tooltips (I18Ned).
- [ ] Add translated error messages for regex errors :
    - [ ] Code 8 - "Found a closing ) with no corresponding opening parenthesis."
    - [ ] Code 7 - "Unmatched [ or [^ in character class declaration."
    - [ ] Code 4 - "Invalid character class name, collating name, or character range."
    - [ ] Code 13 - "The repeat operator "*" cannot start a regular expression."
- [ ] Translate error messages (and add them to the .resw files) :
    - [X] StringFormatError_MissingOpener
    - [X] StringFormatError_MissingCloser
    - [X] StringFormatError_MissingBraces
    - [X] StringFormatError_Empty
    - [X] StringFormatError_ArgumentNotFound
    - [X] StringFormatError_InvalidFormat
    - [X] StringRegexError_Invalid
    - [X] StringRegexError_EmptyString
    - [X] TriggerError_FailedToReload

- [ ] Add format checking/validation for `ClipboardActionView`.
- [ ] Implement copy and pin buttons for clipboard history items.
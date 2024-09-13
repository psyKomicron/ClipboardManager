# Change log
## Changes
- New way to display messages.

## New
- Clipboard triggers now show if the format string is invalid, same for the regex string.
- If a trigger doesn't have a valid format string, a message will be shown to the user.

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
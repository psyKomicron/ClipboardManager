# Change log
## New
`v1.4.7`
- Log file not enabled by default, can be enabled in settings.

`v1.4.0`
- Log file (location can be modified in settings->dev options).

`v1.3.5`
- Added setting to change "show window" shortcut (by default alt + space).
- Overlay mode (button on the top left of the window) now hides the application's window. When enabled the application will behave like a system overlay, being shown when hitting the "Show window" shortcut and hiding after losing focus.
    - If the hot key/shortcut fails to be enabled, the application will not hide.
- Search bar in actions page.

## Changes & Fixes
`v1.4.8`
- `Logger` compile time string formatting (`std::format` and not `std::vformat`).
- Application no longer displays "Logging backend not initialized" if the user hasn't enabled logging.

`v1.4.5`
- Added descriptions for the app log file path text box.
- Fixed possible bug duplicating buttons when `ClipboardActionView` is loaded/unloaded by the `ScrollViewer` virtualization.
- Improved start up experience for users when the app is missing a user file :
    - File will be created empty when the user clicks the "Create" button.
    - Upon closing, the app will write to the file any actions or triggers (even if none are created, a node will be created).
    - If the file is emptied out by the user, or the app fails to save it a message will be shown to inform the user.

`v1.4.4`
- Moved log file to `~/Documents/`.
- Messages bar now displays the last message instead of the first.
- Improved settings category organization.

`v1.4.3`
- Removed log file as it crashes the application.

`v1.4.0`
- Changed message on startup when no triggers are loaded :
    - If no user file has been saved, info bar prompting to create/locate the user file.
    - If a user file has been saved, warning message saying no triggers have been loaded.
- Added message when the logger file hasn't been initialized correctly.
- User file saving fix.

`v1.3.5`
- Added guard to prevent against duplicated clipboard events (bug from Chromium's copy).
- Removed OCR and clipboard history.
- Better explained settings.
- New setting category: Developer.
- Better light mode.

## Bugs
- Outlook duplicates.
- ~~`.log` file requires elevation to create/write, making the application crash if it isn't run elevated.~~

## TODO
- [ ] `ClipboardTriggerEditControl` Format text box, error border is not rounded or padded.
- [ ] Tooltips (I18Ned).
- [ ] Add translated error messages for regex errors :
    - [ ] Code 8 - "Found a closing ) with no corresponding opening parenthesis."
    - [ ] Code 7 - "Unmatched [ or [^ in character class declaration."
    - [ ] Code 4 - "Invalid character class name, collating name, or character range."
    - [ ] Code 13 - "The repeat operator "*" cannot start a regular expression."
- [ ] Translate error messages (and add them to the .resw files) :
- [ ] System tray (https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shell_notifyicona)
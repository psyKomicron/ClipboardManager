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
- Better light mode.

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

# Update notes
# Search
Added search to actions/home page. Search allows you to filter your actions using a regex and either copy the formatted link by right clicking the search result or open your browser by clicking the trigger you want to use in the dropdown button.

# Light theme
Added new colors to better support light theme.

# Duplicated events
Some browsers may fire multiple (usually 2) clipboard events that actually originate from the same user action. To prevent against such events that would create the same number of actions than the number of events, a time based guard has been added. If x clipboard events are less than 50ms apart from each other they will be ignored.

# Simplification
Removed OCR and clipboard history as they were either not working well, and/or not useful.

# Overlay
Added a button in the top left of the window to switch the window into "overlay mode". This mode will mimic overlay windows that hide themselves until shown by hitting a keyboard shortcut, and re-hide themselves when the app window loses focus (ex: clipboard history - Windows + V).

# Triggers - Regex extraction
Regex extraction is now enabled by default, if the regex cannot be used to extract content from the text (no matching group) then the whole matching text will be used.

# Clipboard actions click
Clipboard actions can be clicked, and the behavior of the click can be changed in settings. Clicking an action will either launch the action (open in browser by default) or copy the link formatted by the first trigger of the action to the clipboard.
# Change log
## Changes
- Added scrolling to clipboard view.
- Fixed "Create" user file button not working.
- Reordered teaching tips to better match the flow of the guided tour.
- Fixed application crashing mid guided tour.
- Reload clipboard triggers with the reload button on the clipboard triggers page.
*XML triggers file*
- Renamed `<actions>` to `<triggers>` and `<action>` to `<trigger>`.
- Window can now be minimized on startup, the user needs to activate the option.

## New
- Added xml declaration example in settings to help user to create it's own file and debug errors.
- Added user messages (InfoBar) for when the application fails to load the triggers file.
    - File is missing 'triggers' node.
    - File has 'actions' node, meaning it has not been updated from version 0.0.x.
    - Other errors will have a generic message + the path that was invalid.
- Now restoring app window's size and position.
- App window has a minimum size, it cannot be resized smaller than that size by the user.
- Added custom browser support, you can use any executable not just a custom browser.
*XML triggers file*
- `<re>` node supports 2 attributes:
    - `mode="search|match"` Match is the default value. If the attribute is not added or the attribute value doesn't match any known value, the value will be `"match"`.
    - `ignoreCase="true|false"` False is the default value. If the attribute is not added or the attribute value doesn't match any known value, the value will be `"false"`.

## Bugs
- Locating user file when prompted to will not load the actions into the application.
- ~~Clicking "Create" on startup when asked wether to locate or create a user file if one is not found doesn't create a file.~~
- Settings page not translated.
- Clipboard action matches page has non functional buttons.

## TODO
- [X] Implement reload button on triggers page.
- [X] Implement triggers saving.
- [ ] Implement clipboard triggers removal from clipboard actions.
- [X] Add I18N entries
    - [X] ErrorMessage_CannotSaveTriggersFileNotFound
    - [X] ErrorMessage_CannotSaveTriggersNoUserFile
    - [X] ErrorMessage_CannotSaveTriggersXmlError
    - [X] ErrorMessage_TriggersFileNotFound
    - [X] ErrorMessage_XmlParserError
    - [X] ErrorMessage_InvalidTriggersFile
    - [X] ErrorMessage_MissingTriggersNode
        - : XML declaration is missing '\<triggers>' node.\nCheck settings for an example of a valid XML declaration.
    - [X] ErrorMessage_XmlOldVersion
        - \<actions> node has been renamed \<triggers> and \<action> \<actions>. Rename those nodes in your XML file and reload triggers.\nYou can easily access your user file via settings and see an example of a valid XML declaration there.
- [ ] Fix bugs. ??
- ~~[ ] Save regex flags.~~
- ~~[ ] Settings page/Regex options expander is not padded and not implemented.~~
- [X] Settings page/Start window minimized not implemented.
- [X] Create fr-fr language file.

### Windows 10
- [x] Notifications don't activate.
    - The function wasn't implemented to launch any URI.
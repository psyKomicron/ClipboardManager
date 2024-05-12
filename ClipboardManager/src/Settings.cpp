#include "pch.h"
#include "Settings.hpp"

#include "utils/helpers.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <ShlObj.h>

#include <iostream>

const wchar_t* ClassName{ L"[Settings]  " };

void clipmgr::Settings::open()
{
    auto path = getDefaultUserFileFolder();
    std::filesystem::path userFilePath = path / DEFAULT_USER_FILE_NAME;
    if (!clipmgr::utils::pathExists(userFilePath))
    {
        std::ignore = clipmgr::utils::createFile(userFilePath);
        firstTimeInitialization(userFilePath);
        std::wcout << ClassName << std::quoted(userFilePath.wstring()) << L" User file created." << std::endl;
    }

    try
    {
        std::wcout << ClassName << L" Reading xml file '" << userFilePath.wstring() << L"'" << std::endl;
        boost::property_tree::read_xml(userFilePath.string(), tree);
        empty = false;
    }
    catch (const boost::property_tree::xml_parser_error& parserError)
    {
        std::wcout << ClassName << L" Xml parser error '" << userFilePath.wstring() << L"': " << std::endl;

        if (parserError.line() == 0) // This should mean that the error is at the beginning of the file, thus it's empty.
        {
            firstTimeInitialization(userFilePath);
        }
    }
}

std::vector<clipmgr::ClipboardAction> clipmgr::Settings::getClipboardActions()
{
    std::vector<clipmgr::ClipboardAction> urls{};

    if (!empty)
    {
        for (auto&& node : tree.get_child(L"settings.actions"))
        {
            auto&& url = node.second.get_child(L"format").data();
            auto&& regex = node.second.get_child(L"re").data();
            auto&& label = node.second.get_child(L"label").data();

            urls.push_back(clipmgr::ClipboardAction(label, url, regex));
        }
    }

    return urls;
}

std::filesystem::path clipmgr::Settings::getDefaultUserFileFolder() const
{
    wchar_t* pWstr = nullptr;
    if (SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, nullptr, &pWstr) == S_OK && pWstr != nullptr)
    {
        // RAII pWstr.
        std::unique_ptr<void, std::function<void(void*)>> ptr{ pWstr, CoTaskMemFree };
        auto path = std::filesystem::path(pWstr) / APPLICATION_NAME;

        if (!clipmgr::utils::pathExists(path.c_str()))
        {
            clipmgr::utils::createDirectory(path.c_str());
            std::wcout << ClassName << std::quoted(path.wstring()) << L" Application directory created." << std::endl;
        }

        return path;
    }
    else
    {
        std::filesystem::path folderPath{ L"C:\\" };
        folderPath /= APPLICATION_NAME;
        clipmgr::utils::createDirectory(folderPath.c_str());

        std::wcout << ClassName << L"Couldn't get user documents library, creating application directory in system root " << std::quoted(folderPath.wstring()) << std::endl;

        return folderPath;
    }
}

void clipmgr::Settings::firstTimeInitialization(const std::filesystem::path& userFilePath)
{
    std::wcout << ClassName << "Performing first time initialization of user file.\n";
    auto&& actions = tree.put(L"settings.actions", L"");
    actions.add(L"action.re", L"[A-Z]{,3}");
    actions.add(L"action.format", L"https://example-namespace.com/search?={}");
    actions.add(L"action.label", L"Search example-namespace");

    tree.put(L"settings.preferences", L"");

    boost::property_tree::write_xml(userFilePath.string(), tree);
}

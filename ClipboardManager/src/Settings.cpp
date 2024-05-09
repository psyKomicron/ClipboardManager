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
        std::wcout << ClassName << std::quoted(userFilePath.wstring()) << L" User file created." << std::endl;
    }

    boost::property_tree::read_xml(userFilePath.string(), tree);
    empty = false;
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


clipmgr::ClipboardAction::ClipboardAction(const std::wstring& label, const std::wstring& format, const std::wstring& regexString) :
    _label{ label },
    _format{ format },
    _regex{ createRegex(regexString) }
{

}

std::wstring clipmgr::ClipboardAction::label() const
{
    return _label;
}

std::wstring clipmgr::ClipboardAction::format() const
{
    return _format;
}

boost::wregex clipmgr::ClipboardAction::regex() const
{
    return _regex;
}

boost::wregex clipmgr::ClipboardAction::createRegex(const std::wstring& string)
{
    //TODO: Parse regex for options.
    return boost::wregex(string);
}

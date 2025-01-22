#include "pch.h"
#include "ClipboardAction.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <chrono>
#include <sstream>

namespace clip
{
    ClipboardAction::ClipboardAction(boost::property_tree::wptree& node)
    {
        auto&& creationTimeAttr = node.get_child_optional(L"<xmlattr>.creationTime");
        if (creationTimeAttr && !creationTimeAttr.value().data().empty())
        {
            std::wstringstream wss{ creationTimeAttr.value().data() };
            std::chrono::time_point<std::chrono::system_clock> sysTime{};
            wss >> std::chrono::parse(TIME_T_FORMAT_DES, sysTime);
            _creationTime = sysTime;
        }
        else
        {
            _creationTime = std::chrono::system_clock::now();
        }
        _text = node.data();
    }

    ClipboardAction::ClipboardAction(const std::wstring_view&& text):
        _text{ text },
        _creationTime{ std::chrono::system_clock::now() }
    {
    }

    ClipboardAction::ClipboardAction(const std::wstring_view&& text, const winrt::Windows::Foundation::DateTime& timestamp):
        _text{ text },
        _creationTime{ winrt::clock::to_sys(timestamp) }
    {
    }

    std::wstring ClipboardAction::text() const
    {
        return _text;
    }

    ClipboardAction::time_t ClipboardAction::creationTime() const
    {
        return _creationTime;
    }

    void ClipboardAction::save(boost::property_tree::wptree& tree)
    {
        boost::property_tree::wptree node{ _text };
        node.add(L"<xmlattr>.creationTime", std::format(TIME_T_FORMAT_SER, _creationTime));

        tree.add_child(L"action", node);
    }
}
#include "pch.h"
#include "AppVersion.hpp"

namespace clip::utils
{
    AppVersion::AppVersion() :
        _name{ APP_VERSION_NAME }
    {
        parseString(std::wstring(APP_VERSION));
    }

    AppVersion::AppVersion(std::wstring string) :
        _name{ APP_VERSION_NAME }
    {
        parseString(string);
    }

    uint32_t AppVersion::major() const
    {
        return _major;
    }

    uint32_t AppVersion::minor() const
    {
        return _minor;
    }

    uint32_t AppVersion::revision() const
    {
        return _revision;
    }

    std::wstring AppVersion::name() const
    {
        return _name;
    }

    std::wstring AppVersion::versionString() const
    {
        return std::wstring(APP_VERSION);
    }

    void AppVersion::parseString(std::wstring string)
    {
        uint32_t ints[2]{ 0 };
        uint32_t intsIndex = 0;

        size_t index = 0;
        for (size_t i = 0; i < string.size(); i++)
        {
            if (string[i] == L'.')
            {
                try
                {
                    auto substr = string.substr(index, i - index);
                    index = i + 1;

                    uint32_t version = std::stoul(substr);

                    switch (intsIndex++)
                    {
                        case 0:
                            _major = version;
                            break;
                        case 1:
                            _minor = version;
                            break;
                    }
                }
                catch (std::invalid_argument /*invalidArg*/)
                {
                    // TODO: Log or do something about the exception, don't ignore it.
                }
            }
        }

        _revision = std::stoul(string.substr(index, string.size() - index));
    }
}

#include "pch.h"
#include "AppVersion.hpp"

namespace clip::utils
{
    AppVersion::AppVersion(std::wstring string)
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
                catch (std::invalid_argument invalidArg)
                {
                    // TODO: Log or do something about the exception, don't ignore it.
                }
            }
        }

        _revision = std::stoul(string.substr(index, string.size() - index));
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

    int AppVersion::compare(const AppVersion& other, const bool& ignoreRevision)
    {
        if (major() > other.major())
        {
            return 1;
        }
        else if (major() < other.major())
        {
            return -1;
        }

        if (minor() > other.minor())
        {
            return 1;
        }
        else if (minor() < other.minor())
        {
            return -1;
        }

        if (!ignoreRevision)
        {
            if (revision() > other.revision())
            {
                return 1;
            }
            else if (revision() < other.revision())
            {
                return -1;
            }
        }

        return 0;
    }
}

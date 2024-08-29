#pragma once
#include <string>

namespace clip::utils
{
    class AppVersion
    {
    public:
        AppVersion(std::wstring string);

        uint32_t major() const;
        uint32_t minor() const;
        uint32_t revision() const;

        int compare(const AppVersion& other, const bool& ignoreRevision = true);

    private:
        uint32_t _major = 0;
        uint32_t _minor = 0;
        uint32_t _revision = 0;
    };
}

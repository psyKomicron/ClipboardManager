#pragma once
#include <string>

constexpr const wchar_t* APP_VERSION = L"1.3.5";
constexpr const wchar_t* APP_VERSION_NAME = L"Chêne";

namespace clip::utils
{
    class AppVersion
    {
    public:
        AppVersion();
        AppVersion(std::wstring string);

        uint32_t major() const;
        uint32_t minor() const;
        uint32_t revision() const;
        std::wstring name() const;
        std::wstring versionString() const;

    private:
        uint32_t _major = 0;
        uint32_t _minor = 0;
        uint32_t _revision = 0;
        std::wstring _name{};

        void parseString(std::wstring string);
    };
}

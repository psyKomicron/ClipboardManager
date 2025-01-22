#pragma once
#include "utils/Logger.hpp"

#include <wil/registry.h>
#include <wil/registry_helpers.h>

#include <boost/property_tree/ptree.hpp>

#include <string>
#include <vector>
#include <variant>
#include <map>
#include <concepts>
#include <iostream>

//#define ENABLE_LOGGING

namespace clip
{
    using reg_types = std::variant<std::wstring, std::vector<std::wstring>, uint32_t, uint64_t>;

    enum class RegTypes
    {
        None = -1,
        String = 0,
        StringVector = 1,
        Uint32 = 2,
        Uint64 = 3
    };
}

namespace clip::concepts
{
    template<typename T>
    concept IntegralInsertable = std::same_as<T, uint32_t>
        || std::same_as<T, int32_t>
        || std::same_as<T, uint16_t>
        || std::same_as<T, int16_t>
        || std::same_as<T, uint8_t>
        || std::same_as<T, int8_t>
        || std::same_as<T, char>;

    template<typename T>
    concept EnumInsertable = std::is_enum_v<T> && requires(T t)
    {
        static_cast<uint32_t>(t);
    };

    template<typename T>
    concept LongIntegralInsertable = std::same_as<T, uint64_t>
        || std::same_as<T, int64_t>
        || std::same_as<T, long>;

    template<typename T>
    concept BooleanInsertable = std::same_as<T, bool>;

    template<typename T>
    concept StringInsertable = std::convertible_to<T, std::wstring> || std::constructible_from<T, std::wstring>;

    template<class T>
    concept Mappable = requires(T t)
    {
        {
            t.map()
        } -> std::same_as<std::map<std::wstring, clip::reg_types>>;

        t.insert(std::pair<std::wstring, clip::reg_types>());
    };

    template<typename T>
    concept ObjectInsertable = Mappable<T> || std::convertible_to<T, std::map<std::wstring, clip::reg_types>>;

    template<typename T>
    concept Insertable = IntegralInsertable<T> || EnumInsertable<T> || LongIntegralInsertable<T> || BooleanInsertable<T> || StringInsertable<T>;
}

namespace clip
{
    class Settings
    {
    public:
        using key_t = std::wstring;

        Settings()
        {
            hKey = wil::reg::create_unique_key(HKEY_CURRENT_USER, (L"SOFTWARE\\" + applicationName).c_str(), wil::reg::key_access::readwrite);
#ifdef ENABLE_LOGGING
            logger.debug(L"Opened registry node for application settings.");
#endif // ENABLE_LOGGING

        }

        std::vector<std::pair<std::wstring, clip::reg_types>> getAll();
        void clear();
        void remove(const key_t& key);
        bool contains(const key_t& key);

        template<concepts::StringInsertable T>
        std::optional<T> get(const key_t& key);

        template<concepts::BooleanInsertable T>
        std::optional<T> get(const key_t& key);

        template<concepts::IntegralInsertable T>
        std::optional<T> get(const key_t& key);

        template<concepts::LongIntegralInsertable T>
        std::optional<T> get(const key_t& key);

        template<concepts::EnumInsertable T>
        std::optional<T> get(const key_t& key);

        template<concepts::ObjectInsertable T>
        T get(const key_t& key);

        template<concepts::StringInsertable T>
        void insert(const key_t& key, const T& value);

        template<concepts::BooleanInsertable T>
        void insert(const key_t& key, const T& value);
        
        template<concepts::BooleanInsertable T>
        void insert(const key_t& key, const std::optional<T>& value);

        template<concepts::IntegralInsertable T>
        void insert(const key_t& key, const T& value);

        template<concepts::EnumInsertable T>
        void insert(const key_t& key, const T& value);

        template<concepts::ObjectInsertable T>
        void insert(const key_t& key, const T& value);

    private:
        const std::wstring applicationName = L"ClipboardManagerV2";
        wil::unique_hkey hKey{ nullptr };
#ifdef ENABLE_LOGGING
        utils::Logger logger{ L"Settings" };
#endif // ENABLE_LOGGING


        void clearKey(HKEY hkey);
        wil::shared_hkey createSubKey(const key_t& key);
        RegTypes getValueType(const key_t& name);
    };
}

#include "lib/implementation/Settings_implementation.hpp"
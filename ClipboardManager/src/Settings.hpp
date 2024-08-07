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
    concept Insertable = Mappable<T> || std::convertible_to<T, std::map<std::wstring, clip::reg_types>>;
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
            logger.debug(L"Opened registry node for application settings.");
        }

        std::vector<std::pair<std::wstring, clip::reg_types>> getAll()
        {
            std::vector<std::pair<std::wstring, reg_types>> entries{};

            logger.info(std::format(L"Retreiving all settings. {} values, {} subkeys.", 
                wil::reg::get_child_value_count(hKey.get()),
                wil::reg::get_child_key_count(hKey.get())));

            // Iterate sub keys.
            {
                auto range = wil::make_range(wil::reg::key_iterator(hKey.get()), wil::reg::key_iterator());
                for (auto&& key : range)
                {
                    entries.push_back({ key.name, L"" });
                }
            }

            // Iterate values.
            auto range = wil::make_range(wil::reg::value_iterator(hKey.get()), wil::reg::value_iterator());
            for (auto&& key : range)
            {
                key.name;
                reg_types variant{};
                switch (key.type)
                {
                    case REG_SZ:
                        variant.emplace<0>(get<std::wstring>(key.name).value());
                        break;
                    case REG_MULTI_SZ:
                        variant.emplace<1>(std::vector<std::wstring>());
                        break;
                    case REG_DWORD:
                        variant.emplace<2>(get<uint32_t>(key.name).value());
                        break;
                    case REG_QWORD:
                        variant.emplace<3>(get<uint64_t>(key.name).value());
                        break;
                }

                entries.push_back(std::make_pair(key.name, variant));
            }

            return entries;
        }

        void clear()
        {
            clearKey(hKey.get());
        }

        void remove(const key_t& key)
        {
            RegDeleteValueW(hKey.get(), key.c_str());
        }

        bool contains(const key_t& key)
        {
            bool contains = false;
            auto subKeys = wil::make_range(wil::reg::key_iterator(hKey.get()), wil::reg::key_iterator());
            for (auto&& subKey : subKeys)
            {
                if (subKey.name == key)
                {
                    contains = true;
                    break;
                }
            }
            return contains;
        }


        template<concepts::StringInsertable T>
        std::optional<T> get(const key_t& key)
        {
            std::optional<std::wstring> opt = wil::reg::try_get_value_string(hKey.get(), key.c_str());
            if (opt.has_value())
            {
                return T{ opt.value() };
            }

            return std::optional<T>();
        }

        template<>
        std::optional<std::wstring> get(const key_t& key)
        {
            return wil::reg::try_get_value_string(hKey.get(), key.c_str());
        }

        template<concepts::BooleanInsertable T>
        std::optional<T> get(const key_t& key)
        {
            std::optional<uint32_t> dword = wil::reg::try_get_value_dword(hKey.get(), key.c_str());

            if (dword.has_value())
            {
                return dword.value() == 1;
            }
            else
            {
                logger.debug(L"'" + key + L"' not in registry.");
                return std::optional<bool>();
            }
        }

        template<concepts::IntegralInsertable T>
        std::optional<T> get(const key_t& key)
        {
            return wil::reg::try_get_value_dword(hKey.get(), key.c_str());
        }

        template<concepts::LongIntegralInsertable T>
        std::optional<T> get(const key_t& key)
        {
            return wil::reg::try_get_value_qword(hKey.get(), key.c_str());
        }

        template<concepts::EnumInsertable T> 
        std::optional<T> get(const key_t& key)
        {
            auto optional = get<uint32_t>(key);
            if (optional.has_value())
            {
                return static_cast<T>(optional.value());
            }

            return std::optional<T>();
        }

        template<concepts::Insertable T>
        T get(const key_t& key)
        {
            if (!contains(key))
            {
                return T();
            }

            T map{};

            /* Open HKEY with 'key' name and iterate through it's keys.
            * -> 'key' HKEY
            *   -> 'aaa' HKEY: 12
            *   -> 'bbb' HKEY: "string"
            */
            auto subKeys = wil::make_range(
                wil::reg::key_iterator(wil::reg::open_unique_key(hKey.get(), key.c_str(), wil::reg::key_access::readwrite).get()),
                wil::reg::key_iterator());

            for (wil::reg::key_iterator_data<std::wstring> subKey : subKeys)
            {
                reg_types variant{};
                switch (getValueType(subKey.name))
                {
                    case RegTypes::String:
                        variant.emplace<0>(get<std::wstring>(subKey.name).value());
                        break;
                        // TODO: Implement multi-string.
                        /*case REG_MULTI_SZ:
                        variant.emplace<1>(std::vector<std::wstring>());
                        break;*/
                    case RegTypes::Uint32:
                        variant.emplace<2>(get<uint32_t>(subKey.name).value());
                        break;
                    case RegTypes::Uint64:
                        variant.emplace<3>(get<uint64_t>(subKey.name).value());
                        break;
                }
                break;

                map.insert({ subKey.name, variant });
            }

            return map;
        }

        template<concepts::StringInsertable T>
        void insert(const key_t& key, const T& value)
        {
            wil::reg::set_value(hKey.get(), key.c_str(), value.c_str());
        }

        template<concepts::BooleanInsertable T>
        void insert(const key_t& key, const T& value)
        {
            wil::reg::set_value_dword(hKey.get(), key.c_str(), value ? 1 : 0);
        }

        template<concepts::IntegralInsertable T>
        void insert(const key_t& key, const T& value)
        {
            uint32_t insertable = static_cast<uint32_t>(value);
            wil::reg::set_value_dword(hKey.get(), key.c_str(), value);
        }

        template<concepts::EnumInsertable T>
        void insert(const key_t& key, const T& value)
        {
            insert<uint32_t>(key, static_cast<uint32_t>(value));
        }

        template<concepts::Insertable T>
        void insert(const key_t& key, const T& value)
        {
            auto subKey = createSubKey(key);
            std::map<std::wstring, clip::reg_types> map{};
            if constexpr (concepts::Mappable<T>)
            {
                map = value.toMap();
            }
            else
            {
                map = value;
            }

            try
            {
                for (auto&& pair : map)
                {
                    key_t propKey = pair.first;
                    auto&& variant = pair.second;
                    switch (static_cast<RegTypes>(variant.index()))
                    {
                        case RegTypes::String:
                            wil::reg::set_value_string(subKey.get(), propKey.c_str(), std::get<std::wstring>(variant).c_str());
                            break;
                        case RegTypes::StringVector:
                            wil::reg::set_value_multistring(subKey.get(), propKey.c_str(), std::get<std::vector<std::wstring>>(variant));
                            break;
                        case RegTypes::Uint32:
                            wil::reg::set_value_dword(subKey.get(), propKey.c_str(), std::get<uint32_t>(variant));
                            break;
                        case RegTypes::Uint64:
                            wil::reg::set_value_dword(subKey.get(), propKey.c_str(), std::get<uint64_t>(variant));
                            break;
                    }
                }
            }
            catch (wil::ResultException ex)
            {
                auto count = wil::reg::get_child_value_count(subKey.get());
                if (count > 0)
                {
                    clearKey(subKey.get());
                }

                logger.error(ex.what());
                throw;
            }
        }

    private:
        const std::wstring applicationName = L"ClipboardManagerV2";
        wil::unique_hkey hKey{ nullptr };
        utils::Logger logger{ L"Settings" };

        void clearKey(HKEY hkey)
        {
            auto subKeys = wil::make_range(wil::reg::key_iterator(hKey.get()), wil::reg::key_iterator());
            auto keyValues = wil::make_range(wil::reg::value_iterator(hKey.get()), wil::reg::value_iterator());

            for (auto&& value : keyValues)
            {
                RegDeleteValueW(hKey.get(), value.name.c_str());
            }

            for (auto&& subKey : subKeys)
            {
                auto uniqueKey = wil::reg::create_unique_key(hKey.get(), subKey.name.c_str(), wil::reg::key_access::readwrite);
                clearKey(uniqueKey.get());
                RegDeleteKeyW(uniqueKey.get(), L"");
            }
        }

        wil::shared_hkey createSubKey(const key_t& key)
        {
            return wil::reg::create_shared_key(hKey.get(), key.c_str(), wil::reg::key_access::readwrite);
        }

        RegTypes getValueType(const key_t& name)
        {
            auto range = wil::make_range(
                wil::reg::value_iterator(wil::reg::open_unique_key(hKey.get(), name.c_str(), wil::reg::key_access::readwrite).get()), 
                wil::reg::value_iterator());

            for (auto&& key : range)
            {
                return static_cast<RegTypes>(key.type);
            }

            return RegTypes::None;
        }
    };
}

#pragma once
namespace clip
{
    inline std::vector<std::pair<std::wstring, clip::reg_types>> Settings::getAll()
    {
        std::vector<std::pair<std::wstring, reg_types>> entries{};

#ifdef ENABLE_LOGGING
        logger.info(std::format(L"Retreiving all settings. {} values, {} subkeys.", 
            wil::reg::get_child_value_count(hKey.get()),
            wil::reg::get_child_key_count(hKey.get())));
#endif

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

    inline void Settings::clear()
    {
        clearKey(hKey.get());
    }

    inline bool Settings::remove(const key_t& key)
    {
        return RegDeleteValueW(hKey.get(), key.c_str()) == ERROR_SUCCESS;
    }

    inline bool Settings::contains(const key_t& key)
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
    inline std::optional<T> Settings::get(const key_t& key)
    {
        if constexpr (std::same_as<T, std::wstring>)
        {
            return wil::reg::try_get_value_string(hKey.get(), key.c_str());
        }
        else
        {
            std::optional<std::wstring> opt = wil::reg::try_get_value_string(hKey.get(), key.c_str());
            if (opt.has_value())
            {
                return T{ opt.value() };
            }
            return std::nullopt;
        }
    }

    template<concepts::BooleanInsertable T>
    inline std::optional<T> Settings::get(const key_t& key)
    {
        std::optional<uint32_t> dword = wil::reg::try_get_value_dword(hKey.get(), key.c_str());

        if (dword.has_value())
        {
            return dword.value() == 1;
        }
        else
        {
#ifdef ENABLE_LOGGING
            logger.debug(L"'" + key + L"' not in registry.");
#endif // ENABLE_LOGGING

            return std::nullopt;
        }
    }

    template<concepts::IntegralInsertable T>
    inline std::optional<T> Settings::get(const key_t& key)
    {
        return wil::reg::try_get_value_dword(hKey.get(), key.c_str());
    }

    template<concepts::LongIntegralInsertable T>
    inline std::optional<T> Settings::get(const key_t& key)
    {
        return wil::reg::try_get_value_qword(hKey.get(), key.c_str());
    }

    template<concepts::EnumInsertable T> 
    inline std::optional<T> Settings::get(const key_t& key)
    {
        auto optional = get<uint32_t>(key);
        if (optional.has_value())
        {
            return static_cast<T>(optional.value());
        }

        return std::optional<T>();
    }

    template<concepts::ObjectInsertable T>
    inline T Settings::get(const key_t& key)
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
    inline void Settings::insert(const key_t& key, const T& value)
    {
        if constexpr (std::same_as<T, std::wstring>)
        {
            wil::reg::set_value(hKey.get(), key.c_str(), value.c_str());
        }
        else
        {
            wil::reg::set_value(hKey.get(), key.c_str(), std::wstring(value).c_str());
        }
    }

    template<concepts::BooleanInsertable T>
    inline void Settings::insert(const key_t& key, const T& value)
    {
        wil::reg::set_value_dword(hKey.get(), key.c_str(), value ? 1 : 0);
    }

    template<concepts::BooleanInsertable T>
    inline void Settings::insert(const key_t & key, const std::optional<T>& value)
    {
        if (value.has_value())
        {
            insert<T>(key, value.value());
        }
    }

    template<concepts::IntegralInsertable T>
    inline void Settings::insert(const key_t& key, const T& value)
    {
        uint32_t insertable = static_cast<uint32_t>(value);
        wil::reg::set_value_dword(hKey.get(), key.c_str(), value);
    }

    template<concepts::EnumInsertable T>
    inline void Settings::insert(const key_t& key, const T& value)
    {
        insert<uint32_t>(key, static_cast<uint32_t>(value));
    }

    template<concepts::ObjectInsertable T>
    inline void Settings::insert(const key_t& key, const T& value)
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

#ifdef ENABLE_LOGGING
            logger.error(ex.what());
#endif // ENABLE_LOGGING

            throw;
        }
    }


    inline void Settings::clearKey(HKEY hkey)
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

    inline wil::shared_hkey Settings::createSubKey(const key_t& key)
    {
        return wil::reg::create_shared_key(hKey.get(), key.c_str(), wil::reg::key_access::readwrite);
    }

    inline RegTypes Settings::getValueType(const key_t& name)
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
}
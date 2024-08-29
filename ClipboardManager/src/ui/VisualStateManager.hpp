#pragma once
#include "src/utils/Logger.hpp"

#include <vector>
#include <map>

namespace clip::ui
{
    namespace xaml = winrt::Microsoft::UI::Xaml;

    template<typename T>
    class VisualState
    {
    public:
        VisualState();

        VisualState(const winrt::hstring& name, const int32_t& group, const bool& active);

        int32_t group() const;
        bool active() const;
        void active(const bool& value);
        winrt::hstring name() const;

        bool operator==(const VisualState<T>& other);

        operator winrt::param::hstring() const;

    private:
        bool isInvalidState = false;
        winrt::hstring _name{};
        int32_t _group;
        bool _active;

        inline void throwIfInvalid() const;
    };

    template<typename T>
    class VisualStateManager
    {
    public:
        VisualStateManager(const xaml::Controls::Control& userControl);

        VisualState<T> getCurrentState(const int32_t& group);
        void initializeStates(const std::vector<VisualState<T>>& states);
        void goToState(const VisualState<T>& state, const bool& useTransitions = true);
        void switchState(int32_t group, const bool& useTransitions = true);
        void goToEnabledState(const bool& enabled, const bool& useTransitions = true) const;

    private:
        xaml::Controls::Control control{ nullptr };
        std::map<int32_t, std::vector<VisualState<T>>> _visualStates;
        clip::utils::Logger logger{ L"VisualStateManager" };
    };
}

#include "src/implementation/VisualStateManager_implementation.hpp"
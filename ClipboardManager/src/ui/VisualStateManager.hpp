#pragma once
#include "src/utils/Logger.hpp"

#include <vector>
#include <map>

namespace clipmgr::ui
{
    namespace xaml = winrt::Microsoft::UI::Xaml;

    template<typename T>
    class VisualState
    {
    public:
        VisualState()
        {
            isInvalidState = true;
        }

        VisualState(const winrt::hstring& name, const int32_t& group, const bool& active) :
            _name{ name },
            _group{ group },
            _active{ active }
        {
        }

        int32_t group() const
        {
            throwIfInvalid();
            return _group;
        }

        bool active() const
        {
            return _active;
        }

        void active(const bool& value)
        {
            _active = value;
        }

        winrt::hstring name() const
        {
            return _name;
        }

        bool operator==(const VisualState<T>& other)
        {
            throwIfInvalid();
            return other.name() == name();
        }

        operator winrt::param::hstring() const
        {
            throwIfInvalid();
            return _name;
        }

    private:
        bool isInvalidState = false;
        winrt::hstring _name{};
        int32_t _group;
        bool _active;

        inline void throwIfInvalid() const
        {
            if (isInvalidState)
            {
                throw winrt::hresult_invalid_argument(L"The current visual state is not valid (empty).");
            }
        }
    };

    template<typename T>
    class VisualStateManager
    {
    public:
        VisualStateManager(const xaml::Controls::Control& userControl)
        {
            control = userControl;
        }

        VisualState<T> getCurrentState(const int32_t& group)
        {
            for (size_t i = 0; i < _visualStates[group].size(); i++)
            {
                if (_visualStates[group][i].active())
                {
                    return _visualStates[group][i];
                }
            }
            return {};
        }

        void initializeStates(const std::vector<VisualState<T>>& states)
        {
            for (auto&& state : states)
            {
                _visualStates[state.group()].push_back(state);
            }
        }

        void goToState(const VisualState<T>& state, const bool& useTransitions = true)
        {
            try
            {
                xaml::VisualStateManager::GoToState(control, state, useTransitions);
                for (VisualState<T>& visualState : _visualStates[state.group()])
                {
                    visualState.active(visualState == state);
                }
                logger.debug(std::format(L"[VisualStateManager] Activated state '{}'", std::wstring(state.name())));
            }
            catch (winrt::hresult_wrong_thread)
            {
                logger.debug(std::format(L"[VisualStateManager] Failed to activate '{}', caller called from the wrong thread.", std::wstring(state.name())));
                throw;
            }
        }

        void switchState(int32_t group, const bool& useTransitions = true)
        {
            std::vector<VisualState<T>>& visualStates = _visualStates[group];
            for (size_t i = 0; i < visualStates.size(); i++)
            {
                if (visualStates[i].active())
                {
                    size_t nextIndex = (i + 1) % visualStates.size();
                    goToState(visualStates[nextIndex], useTransitions);
                    visualStates[i].active(false);
                    break;
                }
            }
        }

        void goToEnabledState(const bool& enabled, const bool& useTransitions = true) const
        {
#ifdef _DEBUG
            if (!xaml::VisualStateManager::GoToState(control, enabled ? L"Normal" : L"Disabled", useTransitions))
            {
                logger.debug(std::wstring(L"XAML VisualStateManager failed to go to state '") + (enabled ? L"Normal" : L"Disabled") + L"'.");
            }
            else
            {
                logger.debug(std::wstring(L"XAML VisualStateManager go to state '") + (enabled ? L"Normal" : L"Disabled") + L"'.");
            }
#else
            xaml::VisualStateManager::GoToState(control, enabled ? L"Enabled" : L"Disabled", useTransitions)
#endif
        }

    private:
        xaml::Controls::Control control{ nullptr };
        std::map<int32_t, std::vector<VisualState<T>>> _visualStates;
        clipmgr::utils::Logger logger{ L"VisualStateManager" };
    };
}


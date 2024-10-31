#pragma once
namespace clip::ui
{
    template<typename T>
    inline VisualState<T>::VisualState()
    {
        isInvalidState = true;
    }

    template<typename T>
    inline VisualState<T>::VisualState(const winrt::hstring& name, const int32_t& group, const bool& active) :
        _name{ name },
        _group{ group },
        _active{ active }
    {
    }

    template<typename T>
    inline int32_t VisualState<T>::group() const
    {
        throwIfInvalid();
        return _group;
    }

    template<typename T>
    inline bool VisualState<T>::active() const
    {
        return _active;
    }

    template<typename T>
    inline void VisualState<T>::active(const bool& value)
    {
        _active = value;
    }

    template<typename T>
    inline winrt::hstring VisualState<T>::name() const
    {
        return _name;
    }

    template<typename T>
    inline bool VisualState<T>::operator==(const VisualState<T>& other)
    {
        throwIfInvalid();
        return other.name() == name();
    }

    template<typename T>
    inline VisualState<T>::operator winrt::param::hstring() const
    {
        throwIfInvalid();
        return _name;
    }

    template<typename T>
    inline void VisualState<T>::throwIfInvalid() const
    {
        if (isInvalidState)
        {
            throw winrt::hresult_invalid_argument(L"The current visual state is not valid (empty).");
        }
    }
}

namespace clip::ui
{
    template<typename T>
    VisualStateManager<T>::VisualStateManager(const xaml::Controls::Control& userControl)
    {
        control = userControl;
    }

    template<typename T>
    inline VisualState<T> VisualStateManager<T>::getCurrentState(const int32_t& group)
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

    template<typename T>
    inline void VisualStateManager<T>::initializeStates(const std::vector<VisualState<T>>& states)
    {
        for (auto&& state : states)
        {
            _visualStates[state.group()].push_back(state);
        }
    }

    template<typename T>
    inline void VisualStateManager<T>::goToState(const VisualState<T>& state, const bool& useTransitions)
    {
        try
        {
            std::optional<bool> alreadyActive{};

            if (_visualStates.contains(state.group()))
            {
                for (VisualState<T>& visualState : _visualStates[state.group()])
                {
                    bool same = visualState == state;
                    if (same)
                    {
                        alreadyActive = visualState.active();
                    }

                    visualState.active(same);
                }
            }

            if (!alreadyActive.value_or(false))
            {
                xaml::VisualStateManager::GoToState(control, state, useTransitions);
            }
#ifdef LOG_VISUALSTATEMANAGER
            logger.debug(std::format(L"Activated state '{}'", std::wstring(state.name())));
#endif
        }
        catch (winrt::hresult_wrong_thread)
        {
#ifdef LOG_VISUALSTATEMANAGER
            logger.debug(std::format(L"Failed to activate '{}', caller called from the wrong thread.", std::wstring(state.name())));
#endif          
            throw;
        }
    }

    template<typename T>
    inline void VisualStateManager<T>::switchState(int32_t group, const bool& useTransitions)
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

    template<typename T>
    inline void VisualStateManager<T>::goToStateEnabled(const bool& enabled, const bool& useTransitions) const
    {
#ifdef LOG_VISUALSTATEMANAGER
        if (!xaml::VisualStateManager::GoToState(control, enabled ? L"Normal" : L"Disabled", useTransitions))
        {
            logger.debug(std::wstring(L"XAML VisualStateManager failed to go to state '") + (enabled ? L"Normal" : L"Disabled") + L"'.");
        }
        else
        {
            logger.debug(std::wstring(L"XAML VisualStateManager go to state '") + (enabled ? L"Normal" : L"Disabled") + L"'.");
        }
#else
        xaml::VisualStateManager::GoToState(control, enabled ? L"Normal" : L"Disabled", useTransitions);
#endif
    }
}
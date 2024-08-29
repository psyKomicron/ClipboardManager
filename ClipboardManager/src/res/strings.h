#pragma once
#include <winrt/base.h>

namespace clip::res
{
    using res_t = const winrt::hstring;

    // ClipboardEditTriggerControl
    res_t StringFormatError_MissingOpener{ L"Format string is missing opening brace '{'" };
    res_t StringFormatError_MissingCloser{ L"Format string is missing closing brace '}'" };
    res_t StringFormatError_MissingBraces{ L"Format string is missing opening and closing brace '{}'" };
    res_t StringFormatErorrMissingValue{ L"Format string can't be empty" };
    res_t StringFormatWarning_MissingProtocol{ L"Missing url protocol (https/http), application will default to https" };
    
    res_t StringRegexError_Invalid{ L"Invalid regex at position {}" };
    res_t StringRegexError_EmptyString{ L"Regex string cannot be empty" };



    res_t TriggerError_FailedToReload{ L"Failed to reload triggers" };
}
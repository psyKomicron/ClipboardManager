#pragma once
namespace clipmgr
{
    class Process
    {
    public:
        Process(const std::wstring& path);

        void invoke();
    };
}


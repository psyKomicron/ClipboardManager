#pragma once
namespace clipmgr
{
    class Process
    {
    public:
        Process(const std::wstring& command);
        ~Process();

        void invoke();

    private:
        std::wstring command{};
        void* processHandle = nullptr;
        void* processThreadHandle = nullptr;
    };
}


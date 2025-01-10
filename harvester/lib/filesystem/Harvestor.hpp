#pragma once

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>

constexpr auto WixComponent = "Component";
constexpr auto WixFile = "File";
constexpr auto WixSourceAttribute = "Source";

namespace lib::filesystem
{
    class Harvestor
    {
    public:
        Harvestor(const boost::filesystem::path& wixComponentPath);

        void harvest(const boost::filesystem::path& path, const std::vector<std::string>& excludeList);

    private:
        boost::filesystem::path wixComponentPath{};
        boost::property_tree::ptree xml{};

        std::string rewind(boost::filesystem::path path1, boost::filesystem::path path2);
    };
}
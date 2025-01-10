#include "Harvestor.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <iostream>
#include <ranges>
#include <vector>

namespace lib::filesystem
{
    Harvestor::Harvestor(const boost::filesystem::path& wixComponentPath) :
        wixComponentPath{ wixComponentPath }
    {
        boost::property_tree::read_xml(wixComponentPath.string(), xml, boost::property_tree::xml_parser::trim_whitespace);

        auto& wix = xml.get_child("Wix");
        wix.get_child("Fragment.ComponentGroup");
    }

    void Harvestor::harvest(const boost::filesystem::path& path, const std::vector<std::string>& excludeList)
    {
        if (!boost::filesystem::exists(path))
        {
            throw std::invalid_argument("path");
        }

        auto&& wix = xml.get_child("Wix");
        auto&& componentGroup = wix.get_child("Fragment.ComponentGroup");

        uint64_t harvestSize{};

        std::cout << "\nHarvesting list :\n";
        std::vector<boost::filesystem::path> harvestingList{};
        for (auto&& directoryEntry : boost::filesystem::directory_iterator(path))
        {
            bool exclude = false;
            for (auto&& excludedItem : excludeList)
            {
                if (excludedItem == directoryEntry.path().filename().string())
                {
                    exclude = true;
                    break;
                }
            }

            std::cout << (exclude ? "\033[31m[-]\033[0m " : "\033[32m[+]\033[0m ") << directoryEntry.path().filename() << std::endl;

            if (!exclude && !directoryEntry.is_directory())
            {
                harvestingList.push_back(directoryEntry.path());
                harvestSize += boost::filesystem::file_size(directoryEntry.path());
            }
        }

        std::cout << "Compiling files (" << harvestSize << " bytes)..." << std::endl;
        boost::property_tree::ptree& root = componentGroup;

        size_t removedCount = 0;
        while (root.count("Component") > 0)
        {
            removedCount += root.erase("Component");
        }
        std::cout << removedCount << " node(s) removed." << std::endl;

        for (size_t i = 0; i < harvestingList.size(); i++)
        {
            boost::property_tree::ptree component{};
            auto& fileNode = component.put_child(WixFile, {});
            auto fileSource = rewind(wixComponentPath, harvestingList[i]);
            if (fileSource.empty())
            {
                throw std::runtime_error("Failed to get relative file path for entry " + harvestingList[i].string());
            }

            fileNode.add(std::string("<xmlattr>.") + WixSourceAttribute, fileSource);
            root.add_child(WixComponent, component);

            std::cout << "[" << (i + 1) << "/" << harvestingList.size() << "] Component file source:  " << fileSource << std::endl;
        }

        try
        {
            std::cout << root.count(WixComponent) << " components. Writing to " << wixComponentPath << std::endl;
            boost::property_tree::xml_writer_settings<std::string> settings{ '\t', 1 };
            boost::property_tree::write_xml(wixComponentPath.string(), xml, std::locale(), settings);
        }
        catch (boost::property_tree::xml_parser_error& error)
        {
            std::cerr << "\033[33m" << "[ERROR]" << "\033[0m" << "Writing xml to output file : " << error.message() << std::endl;
        }
    }

    std::string Harvestor::rewind(boost::filesystem::path path1, boost::filesystem::path path2)
    {
        uint32_t rDepth{};

        auto&& filename = path2.filename().string();
        path1 = path1.parent_path();
        path2 = path2.parent_path();

        uint16_t max = ~0;
        uint16_t count{};
        while ((--max) > 0)
        {
            auto&& leftPP  = path1;
            boost::filesystem::path rightPP{ path2 };

            while (!rightPP.empty())
            {
                if (leftPP == rightPP)
                {
                    std::string relative{};
                    for (size_t i = 0; i < rDepth; i++)
                    {
                        relative += std::string("..") + (rDepth > 1 ? "\\" : "");
                    }
                    relative += path2.string().substr(rightPP.size());
                    boost::filesystem::path relativePath{ relative };
                    return (relativePath / filename).string();
                }
                rightPP = rightPP.parent_path();
            }
            path1 = path1.parent_path();
            rDepth++;
        }

        return std::string();
    }
}

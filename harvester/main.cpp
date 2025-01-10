#include "lib/filesystem/Harvestor.hpp"

#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>

#include <string>
#include <iostream>

namespace po = boost::program_options;

int main(int argc, char** argv)
{
    po::options_description desc{ "Options" };
    desc.add_options()
        ("path", po::value<std::string>(), "text")
        ("exclude", po::value<std::string>()->default_value(""), "text")
        ("wix", po::value<std::string>(), "text");
    po::variables_map vm{};
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (!vm.contains("path") || !vm.contains("wix"))
    {
        std::cout << "Missing required arguments.\n";
        return -2;
    }

    auto&& path = vm["path"].as<std::string>();
    auto&& exclude = vm["exclude"].as<std::string>();
    auto&& wixFilePath = vm["wix"].as<std::string>();

    std::vector<std::string> results{};
    boost::split(results, exclude, [](auto&& elem) -> bool
    {
        return elem == ',';
    });

    std::string excludeList{};
    if (!results.empty())
    {
        excludeList = results[0];
        for (size_t i = 1; i < results.size(); i++)
        {
            excludeList += std::string(", ") + results[i];
        }
    }

    std::cout << "Harvesting \"" << path << "\"" << std::endl
        << "Exclude list: " << excludeList << std::endl
        << "Wix output file: " << wixFilePath << std::endl
        << std::flush;

    try
    {
        std::cout << "Creating harvester.\n";
        lib::filesystem::Harvestor harvestor{ boost::filesystem::path(wixFilePath) };
        std::cout << "Harvesting directory.\n";
        harvestor.harvest(boost::filesystem::path(path), results);
    }
    catch (std::exception& exception)
    {
        std::cerr << "\033[33m" << "[ERROR]" << "\033[0m " << exception.what() << std::endl;
    }

    return 0;
}
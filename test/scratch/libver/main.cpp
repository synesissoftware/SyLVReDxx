
#include <sylvredxx/sylvredxx.h>

#include <stlsoft/system/cmdargs.hpp>
#include <platformstl/filesystem/path_functions.h>

#include <iomanip>
#include <iostream>

#include <cstdlib>


// NOTE: this program does not do explicit exception handling.

int main(int argc, char* argv[])
{
    stlsoft::cmdargs args(argc, argv);

    if (args.has_option("help"))
    {
        std::cout
            << "USAGE: "
            << platformstl::get_executable_name_from_path(args.program_name().c_str())
            << " { --help | --version }"
            << std::endl;

        return EXIT_SUCCESS;
    }

    if (args.has_option("version"))
    {
        auto const libver = sylvredxx::api_version();

        std::cout
            << "SyLVReDxx v"
            << ((libver >> 24) & 0xff)
            << '.'
            << ((libver >> 16) & 0xff)
            << '.'
            << ((libver >> 8) & 0xff)
            << '.'
            << ((libver >> 0) & 0xff)
            << std::endl;

        return EXIT_SUCCESS;
    }

    if (!args.options().empty())
    {
        std::cout
            << "USAGE: unrecognised flag/option '"
            << args.options()[0].name
            << "'; use --help for usage"
            << std::endl;

        return EXIT_SUCCESS;
    }

    if (!args.values().empty())
    {
        std::cout
            << "USAGE: unrecognised value '"
            << args.values()[0].name
            << "'; use --help for usage"
            << std::endl;

        return EXIT_SUCCESS;
    }


    std::cout
        << "USAGE: no flags specified; use --help for usage"
        << std::endl;

    return EXIT_SUCCESS;
}

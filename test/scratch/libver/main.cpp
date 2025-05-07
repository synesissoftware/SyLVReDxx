
#include <sylvredxx/sylvredxx.h>

#include <stlsoft/system/cmdargs.hpp>
#include <platformstl/filesystem/path_functions.h>

#ifdef SYLVREDXX_HAS_b64
# include <b64/b64.h>
#endif

#ifdef SYLVREDXX_HAS_Pantheios
# include <pantheios/pantheios.h>
#endif

#ifdef SYLVREDXX_HAS_recls
# include <recls/recls.h>
#endif

#ifdef SYLVREDXX_HAS_shwild
# include <shwild/shwild.h>
#endif


#include <iomanip>
#include <iostream>

#include <cstdlib>


template<
    typename T_stream
,   typename T_integer
>
void
version(
    T_stream&   stm
,   char const* libname
,   T_integer   libver
)
{
    stm
        << libname
        << " v"
        << ((libver >> 24) & 0xff)
        << '.'
        << ((libver >> 16) & 0xff)
        << '.'
        << ((libver >> 8) & 0xff)
        << '.'
        << ((libver >> 0) & 0xff)
        << std::endl
        ;
}


// NOTE: this program does not do explicit exception handling.

int main(int argc, char* argv[])
{
    stlsoft::cmdargs args(argc, argv);

    if (args.has_option("help"))
    {
        std::cout
            << "USAGE: "
            << platformstl::get_executable_name_from_path(args.program_name().c_str())
            << " { --help | --version | --all-versions }"
            << std::endl;

        return EXIT_SUCCESS;
    }

    if (args.has_option("version"))
    {
        auto const libver = sylvredxx::api_version();

        version(std::cout, "SyLVReDxx", libver);

        return EXIT_SUCCESS;
    }

    if (args.has_option("all-versions"))
    {
        {
            auto const libver = sylvredxx::api_version();

            version(std::cout, "SyLVReDxx", libver);
        }

        std::cout
            << std::endl
            << "implemented in terms of:"
            << std::endl
            << std::endl
            ;

#ifdef SYLVREDXX_HAS_b64

        {
            auto const libver = B64_VER;

            version(std::cout, "\tb64", libver);
        }
#endif

#ifdef SYLVREDXX_HAS_Pantheios

        {
            auto const libver = PANTHEIOS_VER;

            version(std::cout, "\tPantheios", libver);
        }
#endif

#ifdef SYLVREDXX_HAS_recls

        {
            auto const libver = RECLS_VER;

            version(std::cout, "\trecls", libver);
        }
#endif

#ifdef SYLVREDXX_HAS_shwild

        {
            auto const libver = SHWILD_VER;

            version(std::cout, "\tshwild", libver);
        }
#endif

        {
            auto const libver = _STLSOFT_VER;

            version(std::cout, "\tSTLSoft", libver);
        }


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

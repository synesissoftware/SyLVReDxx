
#include <sylvredxx/sylvredxx.h>

#include <libclimate/main.hpp>

#include <pantheios/pan.hpp>
#include <pantheios/inserters/i.hpp>
#include <pantheios/trace.h>
#include <pantheios/frontends/stock.h>
#include <pantheios/util/system/threadid.h>

// #include <pantheios/extras/xhelpers/invoke.h>

#include <recls/recls.hpp>

#include <platformstl/synch/sleep_functions.h>
#include <stlsoft/conversion/sas_to_string.hpp>
#include <stlsoft/smartptr/scoped_lambda.hpp>

#include <list>
#include <new>
#include <span>
#include <string>
#include <thread>


const int PROGRAM_VER_MAJOR =   0;
const int PROGRAM_VER_MINOR =   0;
const int PROGRAM_VER_PATCH =   0;
const int PROGRAM_VER_BUILD =   0;
#define PROGRAM_VER_LIST    PROGRAM_VER_MAJOR, PROGRAM_VER_MINOR, PROGRAM_VER_PATCH, PROGRAM_VER_BUILD

#define PROGRAM_NAME                                        "multiple_roots_multithreaded"
#define PROGRAM_SUMMARY                                     "SyLVReDxx scratch tests"
#define PROGRAM_COPYRIGHT                                   "(c) Matthew Wilson, 2019-2025"
#define PROGRAM_DESCRIPTION                                 "performs recursive search and correlation of results, executing each search in a separate thread"
#define PROGRAM_USAGE                                       "USAGE: :program_name: [ ... flags / options ... ] <search-root-1> [ ... <search-root-N> ]"

PANTHEIOS_EXTERN_C PAN_CHAR_T const PANTHEIOS_FE_PROCESS_IDENTITY[] = PANTHEIOS_LITERAL_STRING(PROGRAM_NAME);


clasp_alias_t const libCLImate_specifications[] =
{
    CLASP_STOCK_FLAG_HELP,
    CLASP_STOCK_FLAG_VERSION,

    CLASP_SPECIFICATION_ARRAY_TERMINATOR
};


int run(
    std::list<std::string> const& search_roots
)
{
    PANTHEIOS_TRACE_DEBUG("search_roots[", pan::i(search_roots.size()), "]={...}");

    // version 1: 1 thread per search-root

    std::list<std::thread>  threads;

    for (auto const& search_root : search_roots)
    {
        // auto search_root = srch_root.substr(0);
        char const* search_dir = search_root.c_str();

        auto th = std::thread([search_dir] () {

            // TODO: use Pantheios.Extras.xHelpers

            try
            {
                pan::log_INFORMATIONAL("starting search in '", search_dir, "'");

                stlsoft::scoped_lambda scoper_1([search_dir] {
                    pan::log_INFORMATIONAL("completed search in '", search_dir, "'");
                });

                platformstl::micro_sleep(1250000);
            }
            catch(std::bad_alloc&)
            {
                ;
            }
            catch(std::exception& x)
            {
            }
        });

        threads.push_back(std::move(th));
    }

    for (auto& th : threads)
    {
        th.join();
    }


    return 0;
}


int libCLImate_program_main(
    clasp_arguments_t const* args
)
{
    if (clasp::flag_specified(args, "--help"))
    {
        libCLImate_show_usage(args, libCLImate_specifications, stdout, PROGRAM_VER_LIST, PROGRAM_NAME, PROGRAM_SUMMARY, PROGRAM_COPYRIGHT, PROGRAM_DESCRIPTION, PROGRAM_USAGE, 0);

        return EXIT_SUCCESS;
    }

    if (clasp::flag_specified(args, "--version"))
    {
        libCLImate_show_version(args, libCLImate_specifications, stdout, PROGRAM_VER_LIST, PROGRAM_NAME);

        return EXIT_SUCCESS;
    }

    clasp::verify_all_flags_and_options_used(args);

    if (0 == args->numValues)
    {
        libCLImate::contingent_report("no search roots provided");
    }
    else
    {
        std::list<std::string>  search_roots;

        for (auto const& value : std::span(args->values, args->numValues))
        {
            auto const de = recls::stat(value.value, recls::DETAILS_LATER);

            if (!de.is_directory())
            {
                libCLImate::contingent_report("given search root not a directory", value.value.ptr);
            }
            else
            {
                search_roots.push_back(stlsoft::sas_to_string(value.value));
            }
        }

        STLSOFT_ASSERT(!search_roots.empty());

        return run(search_roots);
    }

    return EXIT_FAILURE;
}


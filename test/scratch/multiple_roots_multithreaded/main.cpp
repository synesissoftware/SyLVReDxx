
/* /////////////////////////////////////////////////////////////////////////
 * compatibility
 */

#if __cplusplus < 201703L
# error Requires C++17 or later
#endif


/* /////////////////////////////////////////////////////////////////////////
 * includes
 */

#include <sylvredxx/sylvredxx.h>

#include <libclimate/main.hpp>

#include <pantheios/pan.hpp>
#include <pantheios/inserters/i.hpp>
#include <pantheios/trace.h>
#include <pantheios/frontends/fe.simple.h>
#include <pantheios/frontends/stock.h>
#include <pantheios/util/system/threadid.h>

// #include <pantheios/extras/xhelpers/invoke.h>

#include <recls/recls.hpp>

#include <platformstl/synch/sleep_functions.h>
#include <stlsoft/conversion/sas_to_string.hpp>
#include <stlsoft/smartptr/scoped_lambda.hpp>
#include <stlsoft/synch/lock_scope.hpp>

#include <list>
#include <iostream>
#include <mutex>
#include <new>
#include <span>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


/* /////////////////////////////////////////////////////////////////////////
 * program description constructs
 */

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


/* /////////////////////////////////////////////////////////////////////////
 * constants
 */

char const FILE_PATTERNS[] = "*.c|*.cpp|*.cs|*.go|*.h|*.hpp|*.java|*.js|*.pl|*.py|*.rb|*.rs|*.ts";

/* /////////////////////////////////////////////////////////////////////////
 * types
 */

typedef std::pair<
    recls::uint64_t //  nodeIndex
,   size_t          //  deviceId
>                                                           entry_key_t;

struct hash_entry_key
{
    std::size_t
    operator()(entry_key_t const& ek) const noexcept
    {
        return std::hash<recls::uint64_t>()(ek.first);
    }
};

typedef std::vector<recls::entry>                           entries_t;

typedef std::unordered_map<
    entry_key_t
,   entries_t
,   hash_entry_key
>                                                           entries_map_t;

typedef std::mutex                                          mx_t;

struct program_context
{
    mx_t                mx;
    entries_map_t       m;
    std::uint64_t       num_entries;
    std::uint64_t       num_distinct_entries;
};


/* /////////////////////////////////////////////////////////////////////////
 * implementation functions
 */

int run(
    std::list<std::string> const& search_roots
)
{
    PANTHEIOS_TRACE_DEBUG("search_roots[", pan::i(search_roots.size()), "]={...}");

    // version 1: 1 thread per search-root

    program_context pc {};

    std::list<std::thread>  threads;

    bool            processing_complete =   false;
    std::thread     th_statistics([&pc, &processing_complete] () {

        pan::log_NOTICE("starting statistics task");

        stlsoft::scoped_lambda scoper_1([] {
            pan::log_NOTICE("completed statistics task");
        });

        for (std::uint64_t i = 0; ; ++i)
        {
            if (0 == (i % 4) || processing_complete)
            {
                pan::log_INFORMATIONAL("stats: ", pan::i(pc.num_entries), " entries; ", pan::i(pc.num_distinct_entries), " distinct entries;");
            }

            if (processing_complete)
            {
                break;
            }
            else
            {
                platformstl::micro_sleep(250000);
            }
        }
    });

    for (auto const& search_root : search_roots)
    {
        // auto search_root = srch_root.substr(0);
        char const* search_dir = search_root.c_str();

        auto th = std::thread([&pc, search_dir] () {

            // TODO: use Pantheios.Extras.xHelpers

            try
            {
                pan::log_NOTICE("starting search in '", search_dir, "'");

                stlsoft::scoped_lambda scoper_1([search_dir] {
                    pan::log_NOTICE("completed search in '", search_dir, "'");
                });

                recls::search_sequence files(search_dir, FILE_PATTERNS, recls::FILES | recls::RECURSIVE | recls::IGNORE_HIDDEN_ENTRIES | recls::RECLS_F_LINK_COUNT | recls::NODE_INDEX);

                for (auto fe : files)
                {
                    pan::log_DEBUG("\t", fe);

                    {
                        // TODO: provide compatibility of `stlsoft::lock_scope` with `std::mutex` (and other elements)

                        // stlsoft::lock_scope lock(mx);
                        std::scoped_lock lock(pc.mx);

                        auto const  k   =   std::make_pair(fe.node_index(), fe.device_id());
                        auto        i   =   pc.m.find(k);

                        if (pc.m.end() == i)
                        {
                            pc.m.insert(std::make_pair(k, entries_t { fe }));

                            ++pc.num_distinct_entries;
                        }
                        else
                        {
                            // at this point we have:
                            //
                            // 1. a hard-link to the same file with a _different_ name, which we want to record; or
                            // 2. a duplicate search results, which we want to ignore

                            auto& duplicates = (*i).second;

                            if (duplicates.end() != std::find(duplicates.begin(), duplicates.end(), fe))
                            {
                                // 2.

                                continue; // so skip increase in `num_entries`
                            }
                            else
                            {
                                // 1.

                                (*i).second.push_back(fe);
                            }
                        }

                        ++pc.num_entries;
                    }
                }
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

    processing_complete = true;

    th_statistics.join();


    // now output results

    {
        std::cout
            << "results"
            << " ("
            << pc.num_distinct_entries
            << " distinct entries"
            << "; "
            << pc.num_entries
            << " entries"
            << "):"
            << std::endl;

        for (auto const& [ key, duplicates ] : pc.m)
        {
            if (duplicates.size() > 1)
            {
                std::cout
                    << '\t'
                    << "duplicates for "
                    << key.second
                    << ':'
                    << key.first
                    << ": "
                    << std::endl;

                for (auto const& fe : duplicates)
                {
                    std::cout
                        << '\t'
                        << '\t'
                        << fe
                        << std::endl;
                }
            }
        }
    }

    return 0;
}

/* /////////////////////////////////////////////////////////////////////////
 * main()
 */

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

#if __cplusplus < 202002L

# error Ensure depending on latest CLASP

        for (auto const& value : clasp::values(args))
#else

        for (auto const& value : std::span(args->values, args->numValues))
#endif
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


        pantheios_fe_simple_setSeverityCeiling(PANTHEIOS_SEV_INFORMATIONAL);


        return run(search_roots);
    }

    return EXIT_FAILURE;
}


/* ///////////////////////////// end of file //////////////////////////// */


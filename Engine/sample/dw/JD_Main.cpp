#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <xel/Util/Command.hpp>
#include <xel_ext/Util/FileSystem.hpp>
#include <iostream>

#include <jdc/jdc.hpp>

#ifdef X_SYSTEM_WINDOWS
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

using namespace xel;
using namespace std;
using namespace jdc;

int main(int argc, char *argv[])
{
#ifdef X_SYSTEM_WINDOWS
    SetConsoleOutputCP(CP_UTF8);
#endif

    xCommandLine Cmd { argc, argv, {
        { 'd', nullptr, "decompile", false },
        { 'i', nullptr, "input_dir", true },
        { 'o', nullptr, "output_dir", true },
    }};

    auto OptInputDir = Cmd["input_dir"];
    auto OptOutputDir = Cmd["output_dir"];
    std::string InputDir = OptInputDir() ? OptInputDir->c_str() : "./test_assets/";
    std::string OutputDir = OptOutputDir() ? OptOutputDir->c_str() : "./logs";

    if (Cmd["decompile"]()) {
        auto JdcConfig = xJdcConfig{
            InputDir,
            OutputDir,
        };
        auto Handle = InitJdc(JdcConfig);
        if (!Handle) {
            cerr << "Failed to init jdc" << endl;
            return -1;
        }
        if (!ExecuteJdc(Handle)) {
            cerr << "Failed to exec jdc" << endl;
            return -1;
        }
        CleanJdc(Handle);
        return 0;
    }

    return 0;
}

#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <xel/Util/Command.hpp>
#include <iostream>

#include <jdc/JD_Class.hpp>
#include <jdc/JD_ClassDump.hpp>
#include <jdc/JD_CodeGenerator.hpp>

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
        { 'r', nullptr, "class_root", true },
        { 'c', nullptr, "class_path", true },
    }};

    if (Cmd["class_path"]()) {
        auto Filename= *Cmd["class_path"];
        cout << "LoadClassFromFile: " << Filename << endl;
        auto LoadResult = LoadClassInfoFromFile(Filename);
        if (LoadResult.ResultCode) {
            cerr << "LoadClassError: " << LoadResult.ErrorMessage << endl;
            return -1;
        }
        cout << DumpClass(LoadResult.Data) << endl;
        // cout << GenerateClassCode(LoadResult.Data) << endl;
        return 0;
    }

    return 0;
}

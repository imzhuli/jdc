#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <xel/Util/Command.hpp>
#include <xel_ext/Util/FileSystem.hpp>
#include <iostream>

#include <jdc/JD_Class.hpp>
#include <jdc/JD_ClassDump.hpp>
#include <jdc/JD_CodeGenerator.hpp>
#include <jdc/JD_Util.hpp>
#include <jdc/JD_Zip.hpp>

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
        { 't', nullptr, "test", false},
        { 'x', nullptr, "unzip", true},
        { 'o', nullptr, "output", true},
    }};

    if (Cmd["class_path"]()) {
        auto Filename= *Cmd["class_path"];
        cout << "LoadClassFromFile: " << Filename << endl;
        auto LoadResult = LoadClassInfoFromFile(Filename);
        if (LoadResult.ResultCode) {
            cerr << "LoadClassError: " << LoadResult.ErrorMessage << endl;
            return -1;
        }
        cout << Dump(LoadResult.Data) << endl;
        cout << GenerateClassCode(LoadResult.Data) << endl;
        return 0;
    }

    if (Cmd["test"]()) {
        auto Tmp = xTempPath();
        cout << "TempDir: " << Tmp.ToString() << endl;
        cin.get();
        return 0;
    }

    if (Cmd["unzip"]() && Cmd["output"]()) {
        auto Source = *Cmd["unzip"];
        auto Target = *Cmd["output"];

        if (!UnzipJar(Target, Source)) {
            cerr << "Failed to unzip file" << endl;
            return -1;
        }
        return 0;
    }

    return 0;
}

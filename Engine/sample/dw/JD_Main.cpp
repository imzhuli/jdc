#include <xel/Common.hpp>
#include <xel/String.hpp>
#include <xel/Util/Command.hpp>
#include <xel_ext/Util/FileSystem.hpp>
#include <iostream>

#include <jdc/jvm/JD_ClassInfo.hpp>
#include <jdc/decompiler/JD_Instructions.hpp>

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
    }};

    return 0;
}

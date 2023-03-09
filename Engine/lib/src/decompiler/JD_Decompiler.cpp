#include <jdc/decompiler/JD_Decompiler.hpp>
#include <filesystem>

namespace jdc
{

    bool xJdc::Init(const xJdcConfig & Config)
    {
        // check:
        if (!std::filesystem::is_directory(Config.InputDirectory)) {
            X_DEBUG_PRINTF("Config.InputDirectory is not a directory\n");
            return false;
        }

        // init
        _Config = Config;

        // debug output:
        X_DEBUG_PRINTF("Config: Input:%s, Output:%s\n", Config.InputDirectory.c_str(), Config.OutputDirectory.c_str());
        return true;
    }

    bool xJdc::Execute()
    {
        _JavaSpace = xJavaSpace::LoadJavaSpace(_Config.InputDirectory);
        if (!_JavaSpace) {
            return false;
        }
        auto JavaSpaceCleaner = xel::xScopeGuard([&]{ _JavaSpace.reset(); });


        return true;
    }

    void xJdc::Clean()
    {

    }

}

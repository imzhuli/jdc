#include <jdc/jdc.hpp>
#include <jdc/syntax/_.hpp>
#include <jdc/decompiler/_.hpp>
#include <jdc/decompiler/JD_Decompiler.hpp>
#include <mutex>

namespace jdc
{

    std::mutex InitMutex;
    size_t InitCounter = 0;

    static bool InitGlobals()
    {
        auto InitGuard = std::lock_guard(InitMutex);
        if (InitCounter++) {
            return true;
        }

        if (!InitJavaSyntax()) {
            return false;
        }
        auto JavaSyntaxCleaner = xel::xScopeGuard([]{
            CleanJavaSyntax();
        });

        JavaSyntaxCleaner.Dismiss();
        return true;
    }

    static void CleanGlobals()
    {
        auto InitGuard = std::lock_guard(InitMutex);
        if (--InitCounter) {
            return;
        }
        CleanJavaSyntax();
    }

    xJdcHandle InitJdc(const xJdcConfig & Config)
    {
        if (!InitGlobals()) {
            return nullptr;
        }
        auto JdcHandle = new xJdc();
        if (!JdcHandle->Init(Config)) {
            delete JdcHandle;
            return nullptr;
        }
        return JdcHandle;
    }

    bool ExecuteJdc(xJdcHandle JdcHandle)
    {
        assert(JdcHandle);
        return JdcHandle->Execute();
    }

    void CleanJdc(xJdcHandle JdcHandle)
    {
        JdcHandle->Clean();
        delete JdcHandle;
        CleanGlobals();
    }

}

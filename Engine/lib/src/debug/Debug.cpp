#include "./Debug.hpp"

using namespace std;
using namespace xel;

namespace jdc
{

    class xDebugManager final
    : public iDebugManager
    {
    public:
        bool Init(const std::string & ConfigFilename);
        void Clean();

        bool IsDebugMethod(const class xJavaMthod * JavaMethodPtr);

    private:
        std::set<std::string> DebugMethodNames;
    };

    static xDebugManager DebugManager;
    iDebugManager * DebugManagerPtr = &DebugManager;

    bool xDebugManager::Init(const std::string & ConfigFilename)
    {
        return true;
    }

    void xDebugManager::Clean()
    {

    }

    bool xDebugManager::IsDebugMethod(const class xJavaMthod * JavaMethodPtr)
    {
        return true;
    }


}

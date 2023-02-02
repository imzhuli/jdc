#include <jdc/JD_Class.hpp>
#include <xel/String.hpp>
#include <xel/Byte.hpp>
#include <sstream>
#include <filesystem>

using namespace std;
using namespace xel;

namespace xjd
{

    xJDResult<xClass> LoadClassInfoFromFile(const std::string & Filename)
    {
        xClass JavaClass = {};
        JavaClass.Name = std::filesystem::path(Filename).stem().string();

        auto FileDataOpt = FileToStr(Filename);
        if (!FileDataOpt()) {
            return { JDR_FILE_ERROR, "Failed to read file into memory block" };
        }

        auto Reader = xStreamReader(FileDataOpt->data());
        auto RemainSize = FileDataOpt->size();

        // read magic and version:
        if (RemainSize < 8) { // magic . maj_ver . min_ver
            return { JDR_DATA_SIZE_ERROR, "Read class info error@" X_STRINGIFY(__LINE__)};
        }
        JavaClass.Magic        = Reader.R4();
        JavaClass.MinorVersion = Reader.R2();
        JavaClass.MajorVersion = Reader.R2();

        return { JavaClass };
    }

    std::string DumpStringFromClass(const xClass & JavaClass)
    {
        std::stringstream ss;
        ss << "ClassName " << JavaClass.Name << endl;
        ss << " - MagicCheck: " << YN(JavaClass.Magic == 0xCAFEBABE) << endl;
        ss << " - MajorVersion: " << JavaClass.MajorVersion << endl;
        ss << " - MinorVersion: " << JavaClass.MinorVersion << endl;

        return ss.str();
    }

}

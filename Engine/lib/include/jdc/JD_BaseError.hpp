#include "./JD_Base.hpp"
#include <string>

namespace xjd
{

    enum eJDResultCode : uint16_t
    {
        JDR_OK = 0,
        JDR_FILE_ERROR,
        JDR_DATA_SIZE_ERROR,

        JDR_UNEXPECTED_ERROR = 65535,
    };

    template<typename tData = xel::xNone>
    struct xJDResult
    {
        eJDResultCode   ResultCode = JDR_OK;
        tData           Data = {};
        std::string     ErrorMessage = {};

        xJDResult() = default;
        xJDResult(const tData & Data) : Data(Data) {}
        xJDResult(tData && Data) : Data(std::move(Data)) {}
        xJDResult(eJDResultCode ResultCode, const std::string & ErrorMessage)
        : ResultCode(ResultCode), ErrorMessage(ErrorMessage)
        {}

    };

}


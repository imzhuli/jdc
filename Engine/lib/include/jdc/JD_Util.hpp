#include "./JD_Base.hpp"
#include <sstream>
#include <string>

namespace jdc
{

    template <typename tIterator, typename tSeprrator>
    std::string Join(tIterator Begin, tIterator End, tSeprrator separator)
    {
        std::ostringstream o;
        if(Begin != End)
        {
            o << *Begin++;
            for(;Begin != End; ++Begin)
                o  << separator << *Begin;
        }
        return o.str();
    }

}
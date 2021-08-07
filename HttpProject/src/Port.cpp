#include <Port.h>

std::unique_ptr<Port> make_port()
{
#ifdef WINDOWS

#else

#endif
    return std::unique_ptr<Port>();
}

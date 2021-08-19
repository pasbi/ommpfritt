#include <fmt/format.h>

#if FMT_VERSION < 50000
namespace fmt
{

using format_error = FormatError;

}  // namespace fmt
#endif

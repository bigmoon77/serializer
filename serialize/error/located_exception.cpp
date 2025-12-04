#include "located_exception.h"
#include <format>

serializer::error::located_exception::located_exception(const std::string& mess, const std::source_location& location)
	:exception(std::format<const char*, const char*, std::uint_least32_t, std::uint_least32_t>(
		"{},from {},line {},column {}",
		mess.c_str(),
		location.file_name(),
		location.line(),
		location.column()
	).c_str())
{
}
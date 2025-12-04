#pragma once

#include <exception>
#include <string>
#include <source_location>
namespace serializer {

	namespace error {

		class located_exception : public std::exception {
		public:
			located_exception(const std::string& mess, const std::source_location& location = std::source_location::current());
		};
	}
};

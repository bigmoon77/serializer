#pragma once

#include <source_location>
#include <format>
#include <fstream>
#include "serializeable_concept.h"

namespace serializer {




	template<serializer::serializeable t, is_literate<t> u = default_literate<t>>
	class hold_object {
		std::filesystem::path _hold_location;
		t& _obj;
		u _literate;
	public:

		hold_object(t& obj,const u& literate = u(), const std::source_location& location = std::source_location::current())
			:_obj(obj), _literate(literate)
		{
			{
				size_t dir = std::hash<const char*>()(location.file_name());
				size_t fun = std::hash<const char*>()(location.function_name());
				size_t line = location.line();
				size_t column = location.column();

				auto str = std::format<size_t>("hold_object\\{}_{}", std::move(dir), std::move(fun));

				if (!std::filesystem::exists(str)) {
					std::filesystem::create_directories(str);
				}
				_hold_location = std::move(str + std::format<size_t, size_t, size_t>("\\{}_{}_{}", typeid(t).hash_code(), std::move(line), std::move(column)));
			}

			if (std::filesystem::exists(_hold_location)) {//load
				_literate.read(obj, _hold_location);
			}
		}

		hold_object(t& obj,const u& literate, const std::source_location& location, const std::string& additional_path)
			:_obj(obj), _literate(literate)
		{
			{
				size_t dir = std::hash<const char*>()(location.file_name());
				size_t fun = std::hash<const char*>()(location.function_name());
				size_t line = location.line();
				size_t column = location.column();

				auto str = std::format<size_t>("hold_object\\{}_{}", std::move(dir), std::move(fun));

				if (!std::filesystem::exists(str)) {
					std::filesystem::create_directories(str);
				}
				_hold_location = std::move(
					str +
					std::format<size_t, size_t, size_t, const char*>(
						"\\{}_{}_{}_{}",
						typeid(t).hash_code(),
						std::move(line),
						std::move(column),
						additional_path.c_str()
					)
				);
			}

			if (std::filesystem::exists(_hold_location)) {//load
				_literate.read(_obj, _hold_location);
			}
		}

		~hold_object() {
			_literate.write(_obj,_hold_location);
		}

	};
};
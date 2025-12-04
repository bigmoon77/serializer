#pragma once

#include <source_location>
#include <format>
#include <fstream>
#include "serializeable_concept.h"

namespace serializer {

	template<serializer::serializeable t>
	class hold_object {
		std::filesystem::path _hold_location;
		t& _obj;
	public:
	
		hold_object(t& obj, const std::source_location& location = std::source_location::current())
			:_obj(obj)
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
				std::ifstream bin_stream(_hold_location);
	
				if (!bin_stream.is_open())//failed
					return;
	
				std::vector<std::uint8_t> bin;
	
				constexpr size_t buffer_size = 128;
	
				std::uint8_t buffer[buffer_size];
				size_t current = 0;
	
				while(!bin_stream.eof())
				{
					bin_stream.read((char*)&buffer[current], buffer_size);
					
					current += bin_stream.gcount();
					bin.resize(current);
					std::memcpy(&bin.data()[current - bin_stream.gcount()], buffer, bin_stream.gcount());
				}
				serialize_read<t>(_obj, bin);
			}
		}
	
		hold_object(t& obj, const std::source_location& location,const std::string& additional_path)
			:_obj(obj)
		{
			{
				size_t dir = std::hash<const char*>()(location.file_name());
				size_t fun = std::hash<const char*>()(location.function_name());
				size_t line = location.line();
				size_t column = location.column();
	
				auto str = std::format<size_t>("hold_object\\{}_{}", std::move(dir),std::move(fun));
	
				if (!std::filesystem::exists(str)) {
					std::filesystem::create_directories(str);
				}
				_hold_location = std::move(
					str + 
					std::format<size_t, size_t, size_t,const char*>(
						"\\{}_{}_{}_{}",
						typeid(t).hash_code(),
						std::move(line),
						std::move(column),
						additional_path.c_str()
						)
				);
			}
	
			if (std::filesystem::exists(_hold_location)) {//load
				std::ifstream bin_stream(_hold_location);
	
				if (!bin_stream.is_open())//failed
					return;
	
				std::vector<std::uint8_t> bin;
	
				constexpr size_t buffer_size = 128;
	
				std::uint8_t buffer[buffer_size];
				size_t current = 0;
	
				while (!bin_stream.eof())
				{
					bin_stream.read((char*)&buffer[current], buffer_size);
	
					current += bin_stream.gcount();
					bin.resize(current);
					std::memcpy(&bin.data()[current - bin_stream.gcount()], buffer, bin_stream.gcount());
				}
				serialize_read<t>(_obj, bin);
			}
		}
	
		~hold_object() {
	
			auto bin = serialize_write(_obj);
			std::ofstream bin_stream(_hold_location);
			bin_stream.write((char*)bin.data(), bin.size());
		}
	
	private:
	
		template<serializer::serializeable t>
		static std::vector<std::uint8_t> serialize_write(const t& a) {
			serializer::serialize_traits<t> traits;
			return traits(a);
		}
	
		template<serializer::serializeable t>
		static void serialize_read(t& obj, const std::vector<std::uint8_t>& bin) {
			serializer::serialize_traits<t> traits;
			traits.store(obj, bin);
		}
	
	};
};
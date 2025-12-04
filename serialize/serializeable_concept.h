#pragma once
#include <type_traits>
#include <vector>

namespace serializer{

	template<typename t>
	concept serialized = std::is_trivially_copyable_v<t>;

	template<typename t>
	struct serialize_traits;

	template<serialized t>
	struct serialize_traits<t> {
		inline std::vector<std::uint8_t> operator()(const t& a) const {
			std::vector <std::uint8_t> res(sizeof(t));
			std::memcpy(res.data(), &a, sizeof(t));
			return res;
		}
		inline void store(t& a, const std::vector<std::uint8_t>& bin)const {
			if (sizeof(t) > bin.size())
				throw std::runtime_error("no match type");
			std::memcpy(&a, bin.data(), sizeof(t));
		}

		/// <summary>
		/// return of loaded bytes
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		/// <returns></returns>
		inline size_t store(t& a, const std::uint8_t* bin)const {
			std::memcpy(&a, bin, sizeof(t));

			return sizeof(t);
		}
	};

	/// <summary>
	/// string—á
	/// </summary>
	template<>
	struct serialize_traits<std::string> {
		using t = std::string;
		inline std::vector<std::uint8_t> operator()(const t& a)const {
			std::vector<std::uint8_t> bin;
			auto size = a.size();

			bin.resize(sizeof(t::size_type) + size * sizeof(t::value_type));

			std::memcpy(bin.data(), &size, sizeof(t::size_type));
			std::memcpy(&bin.data()[sizeof(t::size_type)], a.data(), size * sizeof(t::value_type));

			return bin;
		}

		inline void store(t& a, const std::vector<std::uint8_t>& bin)const {
			if (bin.size() <= sizeof(t::size_type))//•¶Žš—ñ‚ª‹ó
				return;

			t::size_type size = *(t::size_type*)bin.data();
			a.resize(size);
			std::memcpy(a.data(), &bin.data()[sizeof(t::size_type)], size);
		}

		/// <summary>
		/// return of loaded bytes
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		/// <returns></returns>
		inline size_t store(t& a, const std::uint8_t* bin)const {

			t::size_type size = *(t::size_type*)bin;
			a.resize(size);
			std::memcpy(a.data(), &bin[sizeof(t::size_type)], size);

			return sizeof(t::size_type) + size;
		}
	};


	template<typename t>
	concept serializeable =
		std::is_trivially_copyable_v<t> ||
		requires(t a) {
		serialize_traits<t>()(a);
		std::same_as<decltype(serialize_traits<t>()(a)), std::vector<std::uint8_t>>;
		serialize_traits<t>().store(a, std::vector<std::uint8_t>());
	};

}
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
	/// string例
	/// </summary>
	template<typename elem_t, typename traits_t, typename alloc_t>
	struct serialize_traits<std::basic_string<elem_t, traits_t, alloc_t>> {
		using t = std::basic_string<elem_t, traits_t, alloc_t>;

		inline std::vector<std::uint8_t> operator()(const t& a)const {
			std::vector<std::uint8_t> bin;
			auto size = a.size();

			bin.resize(sizeof(typename t::size_type) + size * sizeof(typename t::value_type));

			//書き込むのは文字数.
			//文字サイズに復元する必要がある.
			std::memcpy(bin.data(), &size, sizeof(typename t::size_type));
			std::memcpy(&bin.data()[sizeof(typename t::size_type)], a.data(), size * sizeof(typename t::value_type));

			return bin;
		}

		inline void store(t& a, const std::vector<std::uint8_t>& bin)const {
			if (bin.size() <= sizeof(typename t::size_type))//文字列が空
				return;

			typename t::size_type size = *(typename t::size_type*)bin.data();
			a.resize(size);
			std::memcpy(a.data(), &bin.data()[sizeof(typename t::size_type)], size * sizeof(typename t::value_type));
		}

		/// <summary>
		/// return of loaded bytes
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		/// <returns></returns>
		inline size_t store(t& a, const std::uint8_t* bin)const {
			typename t::size_type size = *(typename t::size_type*)bin;

			a.resize(size);
			std::memcpy(a.data(), &bin[sizeof(typename t::size_type)], size * sizeof(typename t::value_type));

			return sizeof(typename t::size_type) + size * sizeof(typename t::value_type);
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



	/*
	
	実際に読み書きを行う為のconcepts
	
	*/
	template<typename writer_type, typename obj_type>
	concept is_writer = requires(const writer_type& writer, const obj_type& obj, const std::filesystem::path & path) {
		writer.write(obj, path);
	};

	template<typename reader_type, typename obj_type>
	concept is_reader = requires(const reader_type& reader, obj_type & obj, const std::filesystem::path & path) {
		reader.read(obj, path);
	};

	template<typename literate_type, typename obj_type>
	concept is_literate = is_reader<literate_type, obj_type>&& is_writer<literate_type, obj_type>;

	template<typename t>
	inline void default_reader(t& obj, const std::filesystem::path& path) {
		std::ifstream bin_stream(path, std::ios::binary);

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

		serializer::serialize_traits<t> traits;
		traits.store(obj, bin);
	}

	template<typename t>
	inline void default_writer(const t& obj, const std::filesystem::path& path) {
		serializer::serialize_traits<t> traits;

		auto&& bin = traits(obj);

		std::ofstream bin_stream(path,std::ios::binary);

		bin_stream.write((char*)bin.data(), bin.size());
	}


	template<typename t>
	struct default_literate {
		inline void write(const t& obj, const std::filesystem::path& path) const {
			default_writer<t>(obj, path);
		}
		inline void read(t& obj, const std::filesystem::path& path) const {
			default_reader<t>(obj, path);
		}
	};
}
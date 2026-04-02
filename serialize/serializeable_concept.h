#pragma once
#include <type_traits>
#include <vector>

namespace serializer{

	/// <summary>
	/// 追加定義が必要かどうか
	/// </summary>
	template<typename t>
	concept serialized = std::is_trivially_copyable_v<t>;

	/// <summary>
	/// 全てのバイナリ変換を受ける
	/// </summary>
	/// <typeparam name="t"></typeparam>
	template<typename t>
	struct serialize_traits;


	/// <summary>
	/// memcpy可能なデータはこのtraitsが対応する
	/// vector<uint8_t>に変換し、byte列とデータの相互変換を行う
	/// 
	/// </summary>
	/// <typeparam name="t"></typeparam>
	template<serialized t>
	struct serialize_traits<t> {
		
		/// <summary>
		/// オブジェクトをビット列に変換する
		/// </summary>
		/// <param name="a"></param>
		/// <returns></returns>
		inline std::vector<std::uint8_t> operator()(const t& a) const {
			std::vector <std::uint8_t> res(sizeof(t));
			std::memcpy(res.data(), &a, sizeof(t));
			return res;
		}

		/// <summary>
		/// ビット列からオブジェクトを初期化する
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		inline void store(t& a, const std::vector<std::uint8_t>& bin)const {
			if (sizeof(t) > bin.size())
				throw std::runtime_error("no match type");
			std::memcpy(&a, bin.data(), sizeof(t));
		}

		/// <summary>
		/// ビット列からオブジェクトを初期化する
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
	/// データ固有のserializer
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

	/// <summary>
	/// serialize_traitsでのシリアライズが可能かどうかを判定するコンセプト
	/// </summary>
	template<typename t>
	concept serializeable =
		std::is_trivially_copyable_v<t> ||
		requires(t a) {
		serialize_traits<t>()(a);
		std::same_as<decltype(serialize_traits<t>()(a)), std::vector<std::uint8_t>>;
		serialize_traits<t>().store(a, std::vector<std::uint8_t>());
	};


	//読み込み、書き込み時に呼ばれるイベント
	template<typename t>
	struct read_event;

	template<typename t>
	struct write_event;


	template<typename t>
	concept support_read_event = requires
		(const read_event<t>&eve,
			const t & obj,
			const std::filesystem::path & path) {

		eve(obj, path);
	};

	template<typename t>
	concept support_write_event = requires
		(const write_event<t>&eve,
			const t & obj,
			const std::filesystem::path & path) {
		eve(obj, path);
	};

	/*
	
	実際に読み書きを行う為のconcepts

	ファイルへの入出力は、構造体で行う
	
	writer,
	reader,
	literate
	以上三種類が存在する


	要件は以下の通り
	------------------------------------------------
	writer
	void write(const t& , const std::filesystem::path&) const;

	の実装
	------------------------------------------------
	reader
	void read(t& , const std::filesystem::path&) const;
	
	の実装
	------------------------------------------------
	
	literate
	void write(const t& , const std::filesystem::path&) const;
	void read(t& , const std::filesystem::path&) const;

	の実装
	------------------------------------------------
	これらは実装内で
	read_event,write_eventの対応したイベント関数を呼び出す必要がある


	default_literateが内部でバイナリ変換をしている事からわかる通り、
	これらの関数は型をそのまま受け取る

	バイナリへの変換方法はあくまでもユーザーに委ねる

	*/

	//writerが、obj_typeの書き込みをサポートしている
	template<typename writer_type, typename obj_type>
	concept is_writer = requires(const writer_type& writer, const obj_type& obj, const std::filesystem::path & path) {
		writer.write(obj, path);
	};

	//readerが、obj_typeの読み込みをサポートしている
	template<typename reader_type, typename obj_type>
	concept is_reader = requires(const reader_type& reader, obj_type & obj, const std::filesystem::path & path) {
		reader.read(obj, path);
	};

	//literate_typeがobj_typeの読み書きをサポートしている
	template<typename literate_type, typename obj_type>
	concept is_literate = is_reader<literate_type, obj_type> && is_writer<literate_type, obj_type>;


	/// <summary>
	/// 標準関数でのファイル読み取りを行う
	/// 読み込みイベントがある場合は読み込み後に行う
	/// </summary>
	/// <typeparam name="t"></typeparam>
	/// <param name="obj"></param>
	/// <param name="path"></param>
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
			bin_stream.read((char*)buffer, buffer_size);

			current += bin_stream.gcount();
			bin.resize(current);
			std::memcpy(&bin.data()[current - bin_stream.gcount()], buffer, bin_stream.gcount());
		}


		serializer::serialize_traits<t> traits;
		traits.store(obj, bin);


		if constexpr (support_read_event<t>) {
			read_event<t> static_eve;
			static_eve(obj, path);
		}

	}


	/// <summary>
	/// 標準関数でのファイル書き込みを行う
	/// 書き込みイベントがある場合、書き込む前に行う
	/// </summary>
	/// <typeparam name="t"></typeparam>
	/// <param name="obj"></param>
	/// <param name="path"></param>
	template<typename t>
	inline void default_writer(const t& obj, const std::filesystem::path& path) {
		
		if constexpr (support_write_event<t>) {
			write_event<t> static_eve;
			static_eve(obj, path);
		}

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
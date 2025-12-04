#pragma once

#include "mono_cash.h"
#include "cash_guard.h"

#include "../serializeable_concept.h"

#include <unordered_map>
#include <cstdint>

namespace serializer {


	namespace cassher {

		/// <summary>
		/// cash proxyからcash objを公開する
		/// 基本的にholderに入れて使う事を想定する
		/// 
		/// cashに対して、基本的に変数は一つしか対応させない 
		/// 所有権を明確にする必要がある
		/// 
		/// </summary>
		/// <typeparam name="key_type_"></typeparam>
		/// <typeparam name="value_type_"></typeparam>
		template<typename key_type_, typename value_type_>
		class mono_cash_map {
			std::unordered_map<key_type_, mono_cash<value_type_>> _map;
			size_t _id_counter = 0;

			friend struct serializer::serialize_traits<mono_cash_map<key_type_, value_type_>>;
		public:
			using key_type = key_type_;
			using value_type = value_type_;

			mono_cash_map() = default;
			~mono_cash_map() = default;

			/// <summary>
			/// キャッシュが定義されているかを見る
			/// </summary>
			/// <param name="key"></param>
			/// <returns></returns>
			bool exists(const key_type& key) {
				return _map.contains(key);
			}

			/// <summary>
			/// キャッシュを定義する
			/// キャッシュを定義すると 変数とlink出来るようになる
			/// </summary>
			/// <param name="key"></param>
			void register_cash(const key_type& key) {
				_map.emplace(key, _id_counter);
				_id_counter++;
			}

			/// <summary>
			/// scoped link
			/// guardのメモリライフタイムはキャッシュとの接続期間
			/// 接続時に読み込み、切断時に書き込む
			/// </summary>
			/// <param name="key"></param>
			/// <param name="obj"></param>
			/// <returns></returns>
			cash_guard<value_type> link(const key_type& key, value_type& obj) {
				auto& cash = _map.at(key);
				cash.catch_object(obj);
				return cash_guard(&cash);
			}

		};

	};

	//以下loader
	//どうせ使う場所は限るのでinline
	template<typename key_t, typename val_t>
	struct serialize_traits<cassher::mono_cash_map<key_t, val_t>> {
		using t = cassher::mono_cash_map<key_t, val_t>;

		inline std::vector<std::uint8_t> operator()(const t& a) const {

			std::vector<std::uint8_t> bin;

			{
				serialize_traits<size_t> pair_count_traits;

				auto&& tmp = pair_count_traits(a._map.size());
				bin.insert(bin.end(), tmp.begin(), tmp.end());
			}

			{
				serialize_traits<typename t::key_type> key_traits;
				for (auto& [key, val] : a._map)
				{
					auto&& tmp = key_traits(key);
					bin.insert(bin.end(), tmp.begin(), tmp.end());
				}
			}

			{

				//idを記録し、基づいて復元可能にする.
				serialize_traits<typename decltype(a._map)::value_type::second_type::id_type> val_traits;
				for (auto& [key, val] : a._map)
				{
					auto&& tmp = val_traits(val.get_id());
					bin.insert(bin.end(), tmp.begin(), tmp.end());
				}
			}

			return bin;
		}

		inline void store(t& a, const std::vector<std::uint8_t>& bin)const {

			if (bin.size() <= sizeof(size_t)) {
				return;
			}
			size_t offset = 0;

			size_t pair_count = 0;
			{
				serialize_traits<size_t> pair_count_traits;
				offset += pair_count_traits.store(pair_count, &bin.data()[offset]);
			}

			std::vector<typename t::key_type> keys(pair_count);

			{
				serialize_traits<typename t::key_type> key_traits;//鍵復元

				for (size_t i = 0; i < pair_count; i++)
				{
					offset += key_traits.store(keys[i], &bin[offset]);
				}
			}


			using id_type = typename decltype(a._map)::value_type::second_type::id_type;
			std::vector<id_type> ids(pair_count);

			{
				serialize_traits<id_type> id_traits;

				for (size_t i = 0; i < pair_count; i++)
				{
					offset += id_traits.store(ids[i], &bin[offset]);
				}
			}


			for (size_t i = 0; i < pair_count; i++)
			{
				a._map.emplace(
					std::move(keys[i]),
					std::move(ids[i])
				);
			}

			a._id_counter = pair_count;//increment counterなので導出.
		}

	};

};
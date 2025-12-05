#pragma once
#include "../hold_object.h"
#include "../serializeable_concept.h"
#include "../error/located_exception.h"


namespace serializer {

	namespace cassher {


		/// <summary>
		/// cashである事を holderに明示する（holderのパスをある程度固定する）
		/// キャッシュを識別する為にidを設定する
		/// 
		/// 任意のタイミングでキャッシュとのデータリンク、切断を行う 
		/// </summary>
		/// <typeparam name="obj_type"></typeparam>
		template<typename obj_type,is_literate<obj_type> literate_type = default_literate<obj_type>>
		class mono_cash {

			std::unique_ptr<hold_object<obj_type,literate_type>> _holder;//lifetime
			obj_type* _obj = nullptr;
			std::source_location _loc;
			literate_type _literate;

		public:
			using id_type = size_t;
		private:

			id_type _id;
		public:

			mono_cash(id_type id , const literate_type& literate = literate_type())
				:_id(id),_literate(literate)
			{
				_loc = std::source_location::current();
			}

			mono_cash(mono_cash&& other)
				:_id(other._id), _literate(std::move(other._literate)), _loc(std::move(other._loc)), _obj(other._obj)
			{
				other._obj = nullptr;
			}

			~mono_cash() {
				if (_obj)
					release_object();
			}


			const id_type get_id() const {
				return _id;
			}

			/// <summary>
			/// 変数にcashを読み込む
			/// </summary>
			/// <param name="obj"></param>
			void catch_object(obj_type& obj) {
				if (_obj)
					throw error::located_exception("in use cash");

				_obj = &obj;
				read_cash();
			}

			/// <summary>
			/// cashに変数を書き込む
			/// </summary>
			void release_object() {
				write_cash();
				_obj = nullptr;//ここで明確にobjとのlinkを切断する,
			}

			obj_type& get() {
				if (!_obj)
					throw error::located_exception("bad pointer");
				return *_obj;
			}
			const obj_type& get() const {
				if (!_obj)
					throw error::located_exception("bad pointer");
				return *_obj;
			}

		private:
			void read_cash() {
				if (!_obj)
					throw error::located_exception("bad pointer");

				_holder.reset(new hold_object<obj_type,literate_type>(
					*_obj,
					_literate,
					_loc,
					std::to_string(_id))
				);
			}
			void write_cash() {
				if (!_obj)
					throw error::located_exception("bad pointer");
				_holder.reset();//書込み読み込み.
			}
		};

	};

}
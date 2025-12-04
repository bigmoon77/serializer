#pragma once
#include "mono_cash.h"

namespace serializer {

	namespace cassher {

		/// <summary>
		/// cashとのlinkを遅延する
		/// もしくはcashと複数の変数をlinkする為の仲介
		/// メモリライフタイムでキャッシュとの接続を保証する
		/// </summary>
		template<typename t>
		class cash_guard {
			mono_cash<t>* _cash;
		public:

			cash_guard()
				:_cash(nullptr) {

			}

			cash_guard(mono_cash<t>* cash)
				:_cash(cash) {

			}

			cash_guard(cash_guard&& other) noexcept
				:_cash(other._cash)
			{
				other._cash = nullptr;
			}

			//所有権の破棄.
			~cash_guard() {
				if (_cash)
					_cash->release_object();
			}


			//所有権の移動.
			cash_guard& operator = (cash_guard&& other)noexcept {
				_cash = other._cash;
				other._cash = nullptr;

				return *this;
			}

		};

	}
}
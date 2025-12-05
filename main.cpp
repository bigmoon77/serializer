#include <filesystem>
#include <iostream>

#include <unordered_map>
#include <mutex>
#include <functional>
#include <format>


#include "serialize\hold_object.h"
#include "serialize/cassher/mono_cash_map.h"


using namespace std;
template<typename t>
struct custom_literate {
	void read(t& obj,const std::filesystem::path& path) const{
		cout << obj << " r: " << path << endl;
	}
	void write(const t& obj,const std::filesystem::path& path)const {
		cout << obj << " w: " << path << endl;
	}
};
int main() {
	using namespace std;
	using namespace serializer::cassher;
	using namespace serializer;


	mono_cash_map<std::wstring, int,custom_literate<int>> cash_map;
	hold_object<mono_cash_map<std::wstring, int, custom_literate<int>>> cash_holder(cash_map);
	

	for (size_t i = 0; i < 10; i++)
	{
		if (!cash_map.exists(L"hoge")) {
			cash_map.register_cash(L"hoge");
		}
		int tmp = 0;

		cash_guard<int, custom_literate<int>> guard;
		guard = cash_map.link(L"hoge", tmp);

		cout << tmp << endl;

		tmp++;
	}
	
	return 0;
}
#include <filesystem>
#include <iostream>

#include <unordered_map>
#include <mutex>
#include <functional>
#include <format>


#include "serialize\hold_object.h"
#include "serialize/cassher/mono_cash_map.h"

int main() {
	using namespace std;
	using namespace serializer::cassher;
	using namespace serializer;


	mono_cash_map<std::string, int> cash_map;
	hold_object<mono_cash_map<std::string, int>> cash_holder(cash_map);

	for (size_t i = 0; i < 10; i++)
	{
		if (!cash_map.exists("hoge")) {
			cash_map.register_cash("hoge");
		}
		int tmp = 0;

		cash_guard<int> guard;
		guard = cash_map.link("hoge", tmp);

		cout << tmp << endl;

		tmp++;


	}
	
	return 0;
}
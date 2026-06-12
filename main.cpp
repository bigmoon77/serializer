
#include <d3dcompiler.h>
#include <filesystem>
#include <iostream>
#include <map>
#include <vector>
#include <list>

#include <mutex>
#include <typeindex>

#include "serialize/hold_object.h"
#include "serialize/error/located_exception.h"
#include "serialize/mono_resource_map.h"

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



struct shader_literate {
	void read(ID3DBlob*& bin, const std::filesystem::path& path)const {
		D3DReadFileToBlob(path.c_str(), &bin);
	}
	//const引数は無理だった ごめん const_castは使いたくありません
	//この関数に渡されたbinは破棄されるので注意する
	//hold_objectでは、writeが最後の関数となるので
	void write(ID3DBlob*& bin, const std::filesystem::path& path)const {
		D3DWriteBlobToFile(bin, path.c_str(), true);

		if (bin) bin->Release();
	}
};


namespace serializer {

	template<typename _t>
	struct serialize_traits<std::vector<_t>> {
		using t = std::vector<_t>;
		
		/// <summary>
		/// オブジェクトをビット列に変換する
		/// </summary>
		/// <param name="a"></param>
		/// <returns></returns>
		inline std::vector<std::uint8_t> operator()(const t& a) const {
			std::vector <std::uint8_t> res(sizeof(size_t) + sizeof(typename t::value_type) * a.size());
			size_t size = a.size();

			std::memcpy(res.data(),& size , sizeof(size_t));//サイズ入力

			//size_t offset = sizeof(size_t);
			serialize_traits<typename t::value_type> ele_t;

			for (auto& i : a)
			{
				auto temp = ele_t(i);
				res.insert(res.end(), temp.begin(), temp.end());//必ず同じサイズかわからん
				//offset += temp.size();
			}

			return res;
		}

		/// <summary>
		/// ビット列からオブジェクトを初期化する
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		inline void store(t& a, const std::vector<std::uint8_t>& bin)const {
			
			{
				size_t size;
				std::memcpy(&size, bin.data(), sizeof(size_t));
				a.resize(size);
			}

			size_t offset = sizeof(size_t);
			serialize_traits<typename t::value_type> ele_t;
			
			auto head = bin.data();
			for (auto& i : a)
			{
				offset += ele_t.store(i, head + offset);
			}
		}

		/// <summary>
		/// ビット列からオブジェクトを初期化する
		/// return of loaded bytes
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		/// <returns></returns>
		inline size_t store(t& a, const std::uint8_t* bin)const {

			{
				size_t size;
				std::memcpy(&size, bin, sizeof(size_t));
				a.resize(size);
			}

			size_t offset = sizeof(size_t);
			serialize_traits<typename t::value_type> ele_t;

			auto head = bin;
			for (auto& i : a)
			{
				offset += ele_t.store(i, head + offset);
			}
			
			return offset;
		}
	};


	template<typename _t,typename _u>
	struct serialize_traits<std::pair<_t,_u>> {
		using t = std::pair<_t,_u>;

		/// <summary>
		/// オブジェクトをビット列に変換する
		/// </summary>
		/// <param name="a"></param>
		/// <returns></returns>
		inline std::vector<std::uint8_t> operator()(const t& a) const {
			std::vector <std::uint8_t> res;
			
			serialize_traits<_t> tt;
			serialize_traits<_u> ut;

			auto temp = tt(a.first);
			res.insert(res.end(), temp.begin(), temp.end());

			temp = ut(a.second);
			res.insert(res.end(), temp.begin(), temp.end());


			return res;
		}

		/// <summary>
		/// ビット列からオブジェクトを初期化する
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		inline void store(t& a, const std::vector<std::uint8_t>& bin)const {

			serialize_traits<_t> tt;
			serialize_traits<_u> ut;
			
			auto offset = tt.store(a.first, bin.data());
			ut.store(a.second, bin.data() + offset);

		}

		/// <summary>
		/// ビット列からオブジェクトを初期化する
		/// return of loaded bytes
		/// </summary>
		/// <param name="a"></param>
		/// <param name="bin"></param>
		/// <returns></returns>
		inline size_t store(t& a, const std::uint8_t* bin)const {

			serialize_traits<_t> tt;
			serialize_traits<_u> ut;

			auto offset = tt.store(a.first, bin);
			offset += ut.store(a.second, bin + offset);

			return offset;
		}
	};

}



using namespace serializer;


int main() {
	using namespace std;
	using namespace serializer;

	std::vector<std::pair<int, int>> a;

	hold_object<std::vector<std::pair<int, int>>> lockobj(a);



	return 0;
	mono_resource_map map;
	map.set<int>("hoge");
	
	auto obj = map.use<int>("hoge");

	map.unuse<int>("hoge");


	return 0;
}
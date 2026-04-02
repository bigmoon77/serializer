
#include <d3dcompiler.h>
#include <filesystem>
#include <iostream>
#include <map>
#include <mutex>
#include <typeindex>

#include "serialize/hold_object.h"
#include "serialize/error/located_exception.h"

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


using namespace serializer;

/// <summary>
/// pathの参照数を見る
/// リソースをメモリに複数展開しない為
/// </summary>
struct mono_resource_map {

public:

	struct iliterate {
		virtual void read(void*, const std::filesystem::path&) = 0;
		virtual void write(void*, const std::filesystem::path&) = 0;
	};

	template<typename t>
	struct literate;

	//定義例とデフォルトルート
	//このmapで扱うリソースに固有読み書きを定義する場合
	//literateは iliterateを継承する必要がある

	template<typename t>
	struct literate : public iliterate {

		void read(void* obj, const std::filesystem::path& path)override {
			default_literate<t> literate;
			literate.read(*(t*)obj, path);
		}
		void write(void* obj, const std::filesystem::path& path) override {
			default_literate<t> literate;
			literate.write(*(t*)obj, path);
		}
	};


private:
	struct resource_info {
		size_t usecount = 0;
		std::type_index tid = typeid(void);
		std::unique_ptr<iliterate> liter;
		std::shared_ptr<void> obj;//型を抽象化する為にshared ptr

		resource_info() = default;


		void addref() {
			++usecount;
		}
		void release() {
			--usecount;
		}

		bool nouse()const {
			return usecount == 0;
		}
	};	
	std::map<std::filesystem::path, resource_info> _map;
	std::mutex _mtx;

public:

	template<typename value_type>
	void set(const std::filesystem::path& path) {
		std::lock_guard lock(_mtx);
		auto& target = _map[path];
		target.tid = typeid(value_type);
		target.liter.reset(new literate<value_type>());
	}

	template<typename value_type>
	auto use(const std::filesystem::path& path) {
		std::lock_guard lock(_mtx);
		auto& target = _map[path];

#ifdef _DEBUG
		if (target.tid != typeid(value_type)) {
			throw error::located_exception("型の不一致");
		}
#endif

		if (!target.obj) {//未定義

			auto temp = std::make_shared<value_type>();
			target.liter->read(temp.get(), path);
			target.obj = temp;
		}

		target.addref();
		
		return std::static_pointer_cast<value_type>(target.obj);
	}

	template<typename value_type>
	void unuse(const std::filesystem::path& path) {
		std::lock_guard lock(_mtx);
		auto& target = _map[path];

#ifdef _DEBUG
		if (target.tid != typeid(value_type)) {
			throw error::located_exception("型の不一致");
		}
		if (target.nouse()) {
			//release前に0なのはおかしい
			//健全なプログラマーの為にthrow
			throw serializer::error::located_exception("使用回数、解放回数の不一致");
		}
#endif 

		target.release();

		if (target.nouse()) {//参照カウント0

			target.liter->write(target.obj.get(), path);
			target.obj.reset();
		}
	}

};


int main() {
	using namespace std;
	using namespace serializer;



	mono_resource_map map;
	map.set<int>("hoge");
	
	auto obj = map.use<int>("hoge");

	map.unuse<int>("hoge");


	return 0;
}
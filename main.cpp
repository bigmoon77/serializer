
#include <d3dcompiler.h>
#include <filesystem>
#include <iostream>
#include <map>
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
	//constˆø”‚Í–³—‚¾‚Á‚½ ‚²‚ß‚ñ const_cast‚ÍŽg‚¢‚½‚­‚ ‚è‚Ü‚¹‚ñ
	//‚±‚ÌŠÖ”‚É“n‚³‚ê‚½bin‚Í”jŠü‚³‚ê‚é‚Ì‚Å’ˆÓ‚·‚é
	//hold_object‚Å‚ÍAwrite‚ªÅŒã‚ÌŠÖ”‚Æ‚È‚é‚Ì‚Å
	void write(ID3DBlob*& bin, const std::filesystem::path& path)const {
		D3DWriteBlobToFile(bin, path.c_str(), true);

		if (bin) bin->Release();
	}
};


using namespace serializer;

int main() {
	using namespace std;
	using namespace serializer;



	mono_resource_map map;
	map.set<int>("hoge");
	
	auto obj = map.use<int>("hoge");

	map.unuse<int>("hoge");


	return 0;
}
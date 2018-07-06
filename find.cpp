//From https://ds9a.nl/articles/posts/cpp-3/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace db {}
namespace db::err {
	void abort(const std::string& msg) { std::cout << "ERROR (ABORTING): " << msg << std::endl; }
}

struct word_location {
	fs::directory_entry file;
	int pos;
};

void index_words_r(const std::string& path, std::map<std::string, std::vector<word_location>>& index, const std::vector<std::string>& ignores) {
	fs::directory_entry _d(path);
	if (fs::is_directory(_d.status()))
	for (auto&& dirent : fs::recursive_directory_iterator(path)) {
		//std::cout << dirent.path();
		if (fs::is_regular_file(dirent)) {
			std::ifstream ifs(dirent.path());
			const auto fsize = fs::file_size(dirent);
			std::string buf;
			while(ifs >> buf) {
				//std::cout << "WORD: " << buf << " | POS: " << ifs.tellg() << std::endl;
				index[buf].push_back({dirent, (int)(ifs.tellg())-(buf.length())});
			}
		}
	}
}

bool pgetline(std::string message, std::string& str) {
	std::cout << message;
	getline(std::cin, str);
	return !(str == "Quit" || str=="Exit" || str=="q"); 
}

int main(int argc, char * argv[]) {
	//Parse arguments
	if (argc <= 1) { db::err::abort("Not enough arguments (got " + std::to_string(argc-1) + ", expected 1)"); return 1; }

	//Assume user knows that we are passing the path to the main folder as the first argument, and the path to an ignore file as the second

	std::string source = argv[1];
	std::vector<std::string> ignores;
	std::map<std::string, std::vector<word_location>> index;
	
	//Now create the map
	index_words_r(source, index, ignores);
	
	std::string inbuf;
	while (pgetline("\nEnter a command: ", inbuf)) {
		if (inbuf == "find") {
			if (pgetline("\nWord to find: ", inbuf)) {
				auto itr = index.find(inbuf);
				if (itr == index.end()) {
					std::cout << "\nWord not found. Sorry!";
					continue;
				}
				std::cout << std::endl << "Word occurred " << itr->second.size() << " times.\n";
				for (auto&& wl : itr->second) {
					std::cout << "\tFilename: " << wl.file.path() << "\tLocation: " << wl.pos << std::endl;
				}
			}
		}
	}

	return 0;
}

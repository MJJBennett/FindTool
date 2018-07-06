/**
 * Find.cpp - A CLI for indexing and searching a project directory or file
 * 
 * Inspired by (and somewhat taken from) https://ds9a.nl/articles/posts/cpp-3/
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
//#include <filesystem>
#include <experimental/filesystem>

//namespace fs = std::filesystem;
namespace fs = std::experimental::filesystem;

//Playing around here, this is not necessarily a good idea
namespace db {}
namespace db::err {
	void abort(const std::string& msg) { std::cout << "ERROR (ABORTING): " << msg << std::endl; }
}

struct word_location {
	fs::directory_entry file;
	int pos;
};

void index_words_r(const std::string& path, std::map<std::string, std::vector<word_location>>& index, const std::vector<std::string>& ignores) {
    /**
     * Recursively indexes all the words in the directory structure given through path.
     */

    //TODO - support passing in a file; simplify this section
	fs::directory_entry _d(path);
	if (fs::is_directory(_d.status()))
	for (auto&& dirent : fs::recursive_directory_iterator(path)) {
		if (fs::is_regular_file(dirent)) {
			std::ifstream ifs(dirent.path());
			const auto fsize = fs::file_size(dirent);
			std::string buf;
			while(ifs >> buf) {
				index[buf].push_back({dirent, (int)(ifs.tellg())-(buf.length())});
			}
		}
	}
}

std::string get_lower(std::string s) {
    //Found this great solution https://stackoverflow.com/questions/313970/how-to-convert-stdstring-to-lower-case
    std::transform(s.begin(), s.end(), s.begin(), std::tolower);
    return s;
}

bool pgetline(std::string message, std::string& str) {
    //Purely a custom function for use in this program, prints a message, gets input, returns if the input is to quit
	std::cout << message;
	getline(std::cin, str);
    auto str_cmd = get_lower(str); 
	return !(str_cmd == "quit" || str_cmd=="exit" || str_cmd=="q"); 
}

int main(int argc, char * argv[]) {
	//Parse arguments
	if (argc <= 1) { db::err::abort("Not enough arguments (got " + std::to_string(argc-1) + ", expected 1)"); return 1; }

	//Assume user knows that we are passing the path to the main folder as the first argument, and the path to an ignore file as the second
    //TODO - Add support for ignores (this may require restructuring the indexing system)
	std::string source = argv[1];
	std::vector<std::string> ignores;
	std::map<std::string, std::vector<word_location>> index;
	
	//Now create the map
	index_words_r(source, index, ignores);
	
	std::string inbuf;
	while (pgetline("\nEnter a command: ", inbuf)) {
		if (inbuf == "find") {
			pgetline("\nWord to find: ", inbuf)
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

	return 0;
}

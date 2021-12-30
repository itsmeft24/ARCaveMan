#pragma once
#include <unordered_map>
#include <filesystem>
#include "arc.h"
namespace ARCaveMan {
	namespace FrontEnd {
	    

        enum class Operation : uint8_t
        {
            GenericFileAddition,
            FileInDirectoryAddition,
            Replace,
            ReplaceExpand,
            StreamFileAddition,
            StreamReplace,
            StreamReplaceExpand,
        };

        struct HelperFileInfo {
            std::filesystem::path filepath;
            std::string arcpath;
            bool is_stream;
            Operation op;
            //size_t compressed_size;
            size_t decompressed_size;
            std::string extra_info;
        };

		void collect_files(Arc& arc, std::vector<HelperFileInfo>& vec) {
            std::filesystem::path mods_dir("./mods/");
            for (const auto& entry : std::filesystem::recursive_directory_iterator(mods_dir)) {

                if (entry.is_regular_file()) {

                    std::string filepath = entry.path().string();

                    std::string arcpath = ARCaveMan::replace_all(ARCaveMan::replace_all(filepath, mods_dir.string() + "\\", ""), ";", ":");

                    std::transform(arcpath.begin(), arcpath.end(), arcpath.begin(), std::tolower);

                    bool is_stream = arcpath.substr(0, 7) == "stream";

                    Operation op{};

                    std::string extra_info = "";

                    if (is_stream) {
                        try {
                            arc.stream_lookup_by_hash(Hash40::from_str(arcpath));
                            op = Operation::StreamReplace;
                        }
                        catch(std::exception& e){
                            op = Operation::StreamFileAddition;
                        }
                    }
                    else {
                        try {
                            arc.file_lookup_by_hash(Hash40::from_str(arcpath));
                            op = Operation::Replace;
                        }
                        catch (std::exception& e) {
                            if (arcpath.substr(0, 3) == "ui")
                                op = Operation::GenericFileAddition;
                            else {
                                op = Operation::FileInDirectoryAddition;
                                extra_info = "config[arcpath]['dir_info']";
                            }
                        }
                    }

                    vec.push_back(
                        HelperFileInfo{ entry.path(), arcpath, is_stream, op, std::filesystem::file_size(entry.path()), extra_info }
                    );
                }
            }
		
		}
	
	};
};

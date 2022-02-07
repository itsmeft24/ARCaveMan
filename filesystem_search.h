#pragma once
#include "filesystem.h"

struct PathListEntry;
struct FolderPathListEntry;

// almost nothing in here is complete

namespace ARCaveMan {

struct SearchListEntry {
    HashToIndex path;
    HashToIndex parent;
    HashToIndex file_name;
    HashToIndex ext;
};

struct PathListEntry {
    HashToIndex path;
    HashToIndex parent;
    HashToIndex file_name;
    HashToIndex ext;

    bool is_directory(){
        return parent.index & 0x400000 != 0;
    }

    // FolderPathListEntry
    SearchListEntry as_folder_entry(){
        auto file_path = path;
        file_path.index = 0xFFFFFF;

        auto parent_ = parent;
        parent_.index = 0x400000;

        auto ext_ = ext;
        ext_.hash = 0xFFFFFFFF;
        ext_.length = 0x00;
        ext_.index = 0x00;

        return SearchListEntry{ file_path, parent_, ext_, file_name };
    }
};

struct FolderPathListEntry {
    HashToIndex path;
    HashToIndex parent;
    HashToIndex file_name;
    HashToIndex ext;

    uint32_t get_child_count() {
        return parent.index;
    }

    uint64_t get_first_child_index() {
        return ext.as_hash40().as_u64() & 0xFFFFFF;
    }

    // PathListEntry
    SearchListEntry as_path_entry() {
        auto file_path = path;
        file_path.index = 0xFFFFFF;
        auto parent_ = parent;
        parent_.index = 0x400000;
        auto ext_ = ext;
        ext_.hash = 0x0;
        ext_.length = 0x0;
        ext_.index = 0x0;
        return SearchListEntry{ file_path, parent_, file_name, ext_ };
    }

    //static SearchListEntry from_path(std::string _path, std::string _parent, std::string _file_name, std::string _ext) {
    //    HashToIndex::from_hash40(Hash40::from_str(_path), 0xFFFFFF), HashToIndex::from_hash40(Hash40::from_str(_parent), 0x400000), HashToIndex::from_hash40(Hash40::from_str(_file_name), 0), HashToIndex::from_hash40(, 0),
    //}
};

struct FileSystemSearchHeader {
    uint64_t section_size;
    uint32_t folder_path_to_index_count;
    uint32_t unk3;
    uint32_t unk4;
};
}
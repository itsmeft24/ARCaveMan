#pragma once
#include <cstdint>
#include "hash40.h"

#pragma pack(push, 1)

namespace ARCaveMan {
	struct FileSystemHeader {
		uint32_t table_filesize;
		uint32_t file_info_path_count;
		uint32_t file_info_index_count;
		uint32_t folder_count;

		uint32_t folder_offset_count_1;

		uint32_t hash_folder_count;
		uint32_t file_info_count;
		uint32_t file_info_sub_index_count;
		uint32_t file_data_count;

		uint32_t folder_offset_count_2;
		uint32_t file_data_count_2;
		uint32_t padding;

		uint32_t unk1_10; // always 0x10
		uint32_t unk2_10; // always 0x10

		uint8_t regional_count_1;
		uint8_t regional_count_2;
		uint16_t padding2;

		uint32_t version;
		uint32_t extra_folder;
		uint32_t extra_count;

		uint32_t unk[2];

		uint32_t extra_count_2;
		uint32_t extra_sub_count;
	};
	struct RegionalStruct {
		uint32_t unk;
		uint32_t unk1;
		uint32_t index;
	};
	struct StreamHeader {
		uint32_t quick_dir_count;
		uint32_t stream_hash_count;
		uint32_t stream_file_index_count;
		uint32_t stream_offset_entry_count;
	};
	struct QuickDir {
		uint32_t hash;
		uint32_t name_length : 8;
		uint32_t count : 24;
		uint32_t index;
	};
	struct StreamEntry {
		uint32_t hash;
		uint32_t name_length : 8;
		uint32_t index : 24;
		uint32_t flags;
	};
	struct HashToIndex {
		uint32_t hash;
		uint32_t length : 8;
		uint32_t index : 24;
		Hash40 as_hash40() const {
			return { hash, length };
		}
		static HashToIndex from_hash40(Hash40 hash, uint32_t index) {
			return { hash.crc, hash.len, index };
		}
		void set_hash40(Hash40 hash_) {
			hash = hash_.crc;
			length = hash_.len;
		}
	};
	struct FileInfoBucket {
		uint32_t start;
		uint32_t count;
	};
	struct FilePath {
		HashToIndex path;
		HashToIndex ext;
		HashToIndex parent;
		HashToIndex file_name;
		static FilePath from_full_path(std::string str, uint32_t file_info_index_index) {

			std::string _filename = str;

			std::string _extension = _filename.substr(_filename.find_last_of(".") + 1);

			std::string _path = _filename.substr(0, _filename.find_last_of("/") - 1);

			std::string _basename = _filename.substr(_filename.find_last_of("/") + 1);

			FilePath fp{
				HashToIndex::from_hash40(Hash40::from_str(_filename), file_info_index_index),
				HashToIndex::from_hash40(Hash40::from_str(_extension), 0),
				HashToIndex::from_hash40(Hash40::from_str(_path), 0),
				HashToIndex::from_hash40(Hash40::from_str(_basename), 0),
			};

			return fp;
		}
	};
	struct FileInfoIndex {
		uint32_t dir_offset_index;
		uint32_t file_info_index;
	};
	struct DirInfoFlags {
		uint32_t unk1 : 26;
		uint32_t redirected : 1;
		uint32_t unk2 : 1;
		uint32_t is_symlink : 1;
		uint32_t unk3 : 3;
	};
	struct DirInfo {
		HashToIndex path;
		Hash40 name;
		Hash40 parent;
		uint32_t extra_dis_re;
		uint32_t extra_dis_re_length;
		uint32_t file_info_start_index;
		uint32_t file_count;
		uint32_t child_dir_start_index;
		uint32_t child_dir_count;
		DirInfoFlags flags;
	};
	struct StreamData {
		uint64_t size;
		uint64_t offset;
	};
	struct DirectoryOffset {
		uint64_t offset;
		uint32_t decomp_size;
		uint32_t size;
		uint32_t file_start_index;
		uint32_t file_count;
		uint32_t directory_index;
	};
	struct FileInfoFlags {
		uint32_t unused : 4;
		uint32_t is_redirect : 1;
		uint32_t unused2 : 7;
		uint32_t unknown1 : 1;
		uint32_t padding3 : 2;
		uint32_t is_regional : 1;
		uint32_t is_localized : 1;
		uint32_t unused3 : 3;
		uint32_t unknown2 : 1;
		uint32_t unknown3 : 1;
		uint32_t unused4 : 10;
	};
	struct FileInfo {
		uint32_t file_path_index;
		uint32_t file_info_indice_index;
		uint32_t info_to_data_index;
		FileInfoFlags flags;
	};
	struct FileInfoToFileData {
		uint32_t folder_offset_index;
		uint32_t file_data_index;
		uint32_t file_info_idx : 24;
		uint32_t load_type : 8;
	};
	struct FileDataFlags {
		uint32_t compressed : 1;
		uint32_t use_zstd : 1;
		uint32_t unk : 30;
	};
	struct FileData {
		uint32_t offset_in_folder;
		uint32_t comp_size;
		uint32_t decomp_size;
		FileDataFlags flags;
	};
};

#pragma pack(pop)
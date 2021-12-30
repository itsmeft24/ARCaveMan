#pragma once
#include <cstdint>
#include <vector>
#include "filesystem.h"
#include "filesystem_search.h"
#include "patch_section.h"
#include <fstream>
#include <zstd.h>
#include <algorithm>
#include <chrono>
#include <assert.h>

// lazy macro lol

#define VEC_SIZE(vec) vec.size()*sizeof(decltype(vec)::value_type)

#define VEC_PTR(vec) (char*)&vec[0]

#define INTO(VAR) (char*)&VAR, sizeof(VAR)

#define FROM(VAR) (char*)&VAR, sizeof(VAR)

#define MEMCPY_INTO(BUF, VAR, POS) memcpy(&VAR, BUF+POS, sizeof(VAR)); POS+=sizeof(VAR);

#define MEMCPY_FROM(BUF, VAR, POS) memcpy(BUF+POS, &VAR, sizeof(VAR)); POS+=sizeof(VAR);

// WILDLY unsafe. i know.
#define READ_INTO_VEC(stream, count, vec, t) vec.resize(count); stream.read(VEC_PTR(vec), VEC_SIZE(vec));

#define MEMCPY_INTO_VEC(count, vec, t, buffer, pos) vec.resize(count); memcpy(VEC_PTR(vec), buffer+pos, VEC_SIZE(vec)); pos+=VEC_SIZE(vec);

#define WRITE_FROM_VEC(stream, vec) if (VEC_SIZE(vec) != 0) { stream.write(VEC_PTR(vec), VEC_SIZE(vec)); }

#define MEMCPY_FROM_VEC(buffer, pos, vec) if (VEC_SIZE(vec) != 0) { memcpy(buffer+pos, VEC_PTR(vec), VEC_SIZE(vec)); pos+=VEC_SIZE(vec); }

//template<typename T> constexpr auto READ_INTO_VEC_CONSTEXPR(std::istream& stream, unsigned long long count, std::vector<T> vec) { return vec.resize(count); stream.read((char*)&vec[0], vec.size() * sizeof(T)); }

namespace ARCaveMan {

std::string replace_all(std::string data, const std::string& toSearch, const std::string& replaceStr)
{
	// Get the first occurrence
	size_t pos = data.find(toSearch);
	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + replaceStr.size());
	}
	return data;
}

struct ArcHeader {
	uint64_t	magic;
	uint64_t	stream_offset;
	uint64_t	file_data_offset;
	uint64_t	shared_file_data_offset;
	uint64_t	file_system_offset;
	uint64_t	file_system_search_offset;
	uint64_t	patch_section_offset;
};

struct FileSystemCompressedTableHeader {
	uint32_t	padding;
	uint32_t	decomp_size;
	uint32_t	comp_size;
	uint32_t	section_size;
};

typedef FileSystemCompressedTableHeader FileSystemSearchCompressedTableHeader;
typedef FileSystemCompressedTableHeader PatchSectionCompressedTableHeader;

bool bucket_sorter(const HashToIndex& x, const HashToIndex& y) {
	return x.as_hash40().as_u64() < y.as_hash40().as_u64();
}

class FileSystem {
public:
	FileSystemHeader file_system_header;
	char extra_data[0x50];
	StreamHeader stream_header;
	std::vector<QuickDir> quick_dirs;
	std::vector<HashToIndex> stream_hash_to_entries;
	std::vector<StreamEntry> stream_entries;
	std::vector<uint32_t> stream_file_indices;
	std::vector<StreamData> stream_datas;
	uint32_t hash_index_group_count;
	uint32_t bucket_count;
	std::vector<FileInfoBucket> file_info_buckets;
	std::vector<HashToIndex> file_hash_to_path_index;
	std::vector<FilePath> file_paths;
	std::vector<FileInfoIndex> file_info_indices;
	std::vector<HashToIndex> dir_hash_to_info_index;
	std::vector<DirInfo> dir_infos;
	std::vector<DirectoryOffset> folder_offsets;
	std::vector<HashToIndex> folder_child_hashes;
	std::vector<FileInfo> file_infos;
	std::vector<FileInfoToFileData> file_info_to_datas;
	std::vector<FileData> file_datas;
	// char* extra_data_end;
	std::vector<uint8_t> extra_data_end;
	
	FileSystem(std::istream& stream) {
		std::cout << "[ARCaveMan::FileSystem] Parsing data from std::istream buffer..." << std::endl;
		//stream.read((char*)&FileSystemHeader, sizeof(FileSystemHeader));
		stream.read(INTO(file_system_header));
		stream.seekg(0x58, std::ios_base::cur);
		stream.read(extra_data, 0x50);
		stream.read(INTO(stream_header));

		// READ_INTO_VEC_CONSTEXPR<QuickDir>(stream, StreamHeader.quick_dir_count, quick_dirs);

		READ_INTO_VEC(stream, stream_header.quick_dir_count, quick_dirs, QuickDir);
		READ_INTO_VEC(stream, stream_header.stream_hash_count, stream_hash_to_entries, HashToIndex);
		READ_INTO_VEC(stream, stream_header.stream_hash_count, stream_entries, StreamEntry);
		READ_INTO_VEC(stream, stream_header.stream_file_index_count, stream_file_indices, uint32_t);
		READ_INTO_VEC(stream, stream_header.stream_offset_entry_count, stream_datas, StreamData);

		stream.read(INTO(hash_index_group_count));
		stream.read(INTO(bucket_count));

		READ_INTO_VEC(stream, bucket_count, file_info_buckets, FileInfoBucket);
		READ_INTO_VEC(stream, hash_index_group_count, file_hash_to_path_index, HashToIndex);
		READ_INTO_VEC(stream, file_system_header.file_info_path_count, file_paths, FilePath);
		READ_INTO_VEC(stream, file_system_header.file_info_index_count, file_info_indices, FileInfoIndex);
		READ_INTO_VEC(stream, file_system_header.folder_count, dir_hash_to_info_index, HashToIndex);
		READ_INTO_VEC(stream, file_system_header.folder_count, dir_infos, DirInfo);

		uint64_t DIROFFSET_ITER = (uint64_t)file_system_header.folder_offset_count_1 + file_system_header.folder_offset_count_2 + file_system_header.extra_folder;

		READ_INTO_VEC(stream, DIROFFSET_ITER, folder_offsets, DirectoryOffset);
		READ_INTO_VEC(stream, file_system_header.hash_folder_count, folder_child_hashes, HashToIndex);

		uint64_t FILEINFO_ITER = (uint64_t)file_system_header.file_info_count + file_system_header.file_data_count_2 + file_system_header.extra_count;

		READ_INTO_VEC(stream, FILEINFO_ITER, file_infos, FileInfo);

		uint64_t FILEINFO_DATAS_ITER = (uint64_t)file_system_header.file_info_sub_index_count + file_system_header.file_data_count_2 + file_system_header.extra_count_2;

		READ_INTO_VEC(stream, FILEINFO_DATAS_ITER, file_info_to_datas, FileInfoToFileData);

		uint64_t FILEDATAS_ITER = (uint64_t)file_system_header.file_data_count + file_system_header.file_data_count_2 + file_system_header.extra_count;

		READ_INTO_VEC(stream, FILEDATAS_ITER, file_datas, FileData);

		auto curr_pos = stream.tellg();

		stream.seekg(0, std::ios_base::end);

		auto size = stream.tellg();

		stream.seekg(curr_pos, std::ios_base::beg);

		READ_INTO_VEC(stream, size - curr_pos, extra_data_end, uint8_t);

		//extra_data_end = new char[size - curr_pos];

		//stream.read(extra_data_end, size - curr_pos);
	}
	
	FileSystem(char* buf, uint64_t size) {
		std::cout << "[ARCaveMan::FileSystem] Parsing data from char array (buffer)..." << std::endl;
		uint64_t pos = 0;

		MEMCPY_INTO(buf, file_system_header, pos);
		pos += 0x58;
		memcpy(extra_data, buf, 0x50); pos += 0x50;
		MEMCPY_INTO(buf, stream_header, pos);

		// READ_INTO_VEC_CONSTEXPR<QuickDir>(stream, StreamHeader.quick_dir_count, quick_dirs);
		MEMCPY_INTO_VEC(stream_header.quick_dir_count, quick_dirs, QuickDir, buf, pos);
		MEMCPY_INTO_VEC(stream_header.stream_hash_count, stream_hash_to_entries, HashToIndex, buf, pos);
		MEMCPY_INTO_VEC(stream_header.stream_hash_count, stream_entries, StreamEntry, buf, pos);
		MEMCPY_INTO_VEC(stream_header.stream_file_index_count, stream_file_indices, uint32_t, buf, pos);
		MEMCPY_INTO_VEC(stream_header.stream_offset_entry_count, stream_datas, StreamData, buf, pos);
		
		MEMCPY_INTO(buf, hash_index_group_count, pos);
		MEMCPY_INTO(buf, bucket_count, pos);

		MEMCPY_INTO_VEC(bucket_count, file_info_buckets, FileInfoBucket, buf, pos);
		MEMCPY_INTO_VEC(hash_index_group_count, file_hash_to_path_index, HashToIndex, buf, pos);
		MEMCPY_INTO_VEC(file_system_header.file_info_path_count, file_paths, FilePath, buf, pos);
		MEMCPY_INTO_VEC(file_system_header.file_info_index_count, file_info_indices, FileInfoIndex, buf, pos);
		MEMCPY_INTO_VEC(file_system_header.folder_count, dir_hash_to_info_index, HashToIndex, buf, pos);
		MEMCPY_INTO_VEC(file_system_header.folder_count, dir_infos, DirInfo, buf, pos);

		uint64_t DIROFFSET_ITER = (uint64_t)file_system_header.folder_offset_count_1 + file_system_header.folder_offset_count_2 + file_system_header.extra_folder;

		MEMCPY_INTO_VEC(DIROFFSET_ITER, folder_offsets, DirectoryOffset, buf, pos);
		MEMCPY_INTO_VEC(file_system_header.hash_folder_count, folder_child_hashes, HashToIndex, buf, pos);

		uint64_t FILEINFO_ITER = (uint64_t)file_system_header.file_info_count + file_system_header.file_data_count_2 + file_system_header.extra_count;

		MEMCPY_INTO_VEC(FILEINFO_ITER, file_infos, FileInfo, buf, pos);

		uint64_t FILEINFO_DATAS_ITER = (uint64_t)file_system_header.file_info_sub_index_count + file_system_header.file_data_count_2 + file_system_header.extra_count_2;

		MEMCPY_INTO_VEC(FILEINFO_DATAS_ITER, file_info_to_datas, FileInfoToFileData, buf, pos);

		uint64_t FILEDATAS_ITER = (uint64_t)file_system_header.file_data_count + file_system_header.file_data_count_2 + file_system_header.extra_count;

		MEMCPY_INTO_VEC(FILEDATAS_ITER, file_datas, FileData, buf, pos);

		std::cout << "[ARCaveMan::FileSystem] Reading into extra_data from pos=" << pos << " with len=" << size - pos << std::endl;

		MEMCPY_INTO_VEC(size - pos, extra_data_end, uint8_t, buf, pos);

		

		//extra_data_end = new char[size - pos];

		//memcpy(extra_data_end, buf+pos, size - pos); pos = size;
	}
	
	FileSystem() {}
	
	uint64_t get_table_size() {
		uint64_t SIZE = sizeof(file_system_header) * 2 + 0x50 + sizeof(stream_header) + VEC_SIZE(quick_dirs) +
			VEC_SIZE(stream_hash_to_entries) + VEC_SIZE(stream_entries) + VEC_SIZE(stream_file_indices) +
			VEC_SIZE(stream_datas) + sizeof(hash_index_group_count) + sizeof(bucket_count) + VEC_SIZE(file_info_buckets) +
			VEC_SIZE(file_hash_to_path_index) + VEC_SIZE(file_paths) + VEC_SIZE(file_info_indices) + VEC_SIZE(dir_hash_to_info_index) +
			VEC_SIZE(dir_infos) + VEC_SIZE(folder_offsets) + VEC_SIZE(folder_child_hashes) + VEC_SIZE(file_infos) +
			VEC_SIZE(file_info_to_datas) + VEC_SIZE(file_datas) + VEC_SIZE(extra_data_end);
		return SIZE;
	}

	void REBUILD_FILEINFOBUCKETS() {
		std::cout << "[ARCaveMan::FileSystem::FileInfoBuckets] Rebuilding FileInfoBuckets..." << std::endl;
		std::vector<std::vector<HashToIndex>> buckets(bucket_count);

		for (int x = 0; x < file_paths.size(); x++) {
			const auto& file_path = file_paths[x];
			Hash40 hash = file_path.path.as_hash40();
			auto bucket_index = hash.as_u64() % bucket_count;

			auto hashtoidx = HashToIndex::from_hash40(hash, x);
			buckets[bucket_index].push_back(hashtoidx);
		}

		file_hash_to_path_index.clear();
		file_info_buckets.clear();

		uint32_t start = 0;

		for (auto& bucket : buckets) {
			std::sort(bucket.begin(), bucket.end(), &bucket_sorter);
			std::copy(bucket.begin(), bucket.end(), std::back_inserter(file_hash_to_path_index));
			file_info_buckets.push_back(FileInfoBucket{ start, (uint32_t)bucket.size() });
			start += bucket.size();
		}
	}

	void write(std::string path) {
		auto start = std::chrono::high_resolution_clock::now();
		std::cout << "[ARCaveMan::FileSystem::Write] Started export process..." << std::endl;
		std::ofstream file(path, std::ios::out | std::ios::binary);

		file_system_header.table_filesize = get_table_size();

		file.write(FROM(file_system_header));
		file.write(FROM(file_system_header));
		file.write(extra_data, 0x50);
		file.write(FROM(stream_header));
		
		WRITE_FROM_VEC(file, quick_dirs);
		WRITE_FROM_VEC(file, stream_hash_to_entries);
		WRITE_FROM_VEC(file, stream_entries);
		WRITE_FROM_VEC(file, stream_file_indices);
		WRITE_FROM_VEC(file, stream_datas);

		file.write(FROM(hash_index_group_count));
		file.write(FROM(bucket_count));

		WRITE_FROM_VEC(file, file_info_buckets);
		WRITE_FROM_VEC(file, file_hash_to_path_index);
		WRITE_FROM_VEC(file, file_paths);
		WRITE_FROM_VEC(file, file_info_indices);
		WRITE_FROM_VEC(file, dir_hash_to_info_index);
		WRITE_FROM_VEC(file, dir_infos);
		WRITE_FROM_VEC(file, folder_offsets);
		WRITE_FROM_VEC(file, folder_child_hashes);
		WRITE_FROM_VEC(file, file_infos);
		WRITE_FROM_VEC(file, file_info_to_datas);
		WRITE_FROM_VEC(file, file_datas);

		WRITE_FROM_VEC(file, extra_data_end);
		file.close();

		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		std::cout << "[ARCaveMan::FileSystem::Write] Finished writing file system to " << path << " in " << duration.count() << "ms." << std::endl;
	}
	
	void write_uncompressed_to_buf(char* buffer, uint64_t max_size, uint64_t& pos) {

		MEMCPY_FROM(buffer, file_system_header, pos);
		MEMCPY_FROM(buffer, file_system_header, pos);
		memcpy(buffer + pos, extra_data, 0x50); pos += 0x50;
		MEMCPY_FROM(buffer, stream_header, pos);

		MEMCPY_FROM_VEC(buffer, pos, quick_dirs);
		MEMCPY_FROM_VEC(buffer, pos, stream_hash_to_entries);
		MEMCPY_FROM_VEC(buffer, pos, stream_entries);
		MEMCPY_FROM_VEC(buffer, pos, stream_file_indices);
		MEMCPY_FROM_VEC(buffer, pos, stream_datas);

		MEMCPY_FROM(buffer, hash_index_group_count, pos);
		MEMCPY_FROM(buffer, bucket_count, pos);

		MEMCPY_FROM_VEC(buffer, pos, file_info_buckets);
		MEMCPY_FROM_VEC(buffer, pos, file_hash_to_path_index);
		MEMCPY_FROM_VEC(buffer, pos, file_paths);
		MEMCPY_FROM_VEC(buffer, pos, file_info_indices);
		MEMCPY_FROM_VEC(buffer, pos, dir_hash_to_info_index);
		MEMCPY_FROM_VEC(buffer, pos, dir_infos);
		MEMCPY_FROM_VEC(buffer, pos, folder_offsets);
		MEMCPY_FROM_VEC(buffer, pos, folder_child_hashes);
		MEMCPY_FROM_VEC(buffer, pos, file_infos);
		MEMCPY_FROM_VEC(buffer, pos, file_info_to_datas);
		MEMCPY_FROM_VEC(buffer, pos, file_datas);

		MEMCPY_FROM_VEC(buffer, pos, extra_data_end);

	}

	// writes to buffer then compresses to zstd then writes to file. returns compressed size.
	uint64_t write_compressed(std::string path) {
		auto start = std::chrono::high_resolution_clock::now();
		std::cout << "[ARCaveMan::FileSystem::Write] Started export process..." << std::endl;
		uint64_t SIZE = get_table_size();

		file_system_header.table_filesize = SIZE;

		char* buffer = new char[SIZE]; uint64_t pos = 0;

		write_uncompressed_to_buf(buffer, SIZE, pos);

		std::ofstream file_dcmp("output_fs_edited.tbl", std::ios::out | std::ios::binary); // hardcoded because lazy moment
		file_dcmp.write(buffer, SIZE);
		file_dcmp.close();

		char* buffer_out = new char[SIZE];
		std::cout << "[ARCaveMan::FileSystem::Write] Compressing data.arc file system..." << std::endl;
		uint64_t out_size = ZSTD_compress(buffer_out, SIZE, buffer, SIZE, ZSTD_maxCLevel());
		delete[] buffer;
		std::cout << "[ARCaveMan::FileSystem::Write] Writing data.arc file system to " << path << "..." << std::endl;
		std::ofstream file(path, std::ios::out | std::ios::binary);
		file.write(buffer_out, out_size);
		file.close();
		delete[] buffer_out;
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
		std::cout << "[ARCaveMan::FileSystem::Write] Finished writing file system in " << duration.count()/1000 << "ms." << std::endl;
		return out_size;
		
	}
	
	// DO NOT USE!!!!!!!
	uint64_t write_directly_to_arc(std::string path) {
		
		std::cout << "[ARCaveMan::FileSystem::WriteDirectlyToARC] Writing directly to ARC at " << path << "..." << std::endl;

		uint64_t SIZE = write_compressed("output_fs_compressed.tbl");
		std::ifstream fsdata("output_fs_compressed.tbl", std::ios::in | std::ios::out | std::ios::binary);

		char* buffer = new char[SIZE];
		fsdata.read(buffer, SIZE);
		fsdata.close();

		std::ifstream arc_input(path, std::ios::in | std::ios::binary);
		if (!arc_input) {
			std::cout << "[ARCaveMan::FileSystem::WriteDirectlyToARC] Failed to open ARC. Aborting..." << std::endl;
			return 1;
		}
		std::ofstream arc_output;

		ArcHeader local_arc_header{};

		arc_input.read(INTO(local_arc_header));

		arc_input.seekg(local_arc_header.file_system_offset);

		FileSystemCompressedTableHeader local_fs_ctable_header{};

		arc_input.read(INTO(local_fs_ctable_header));

		if (local_fs_ctable_header.section_size < SIZE) {

			arc_input.seekg(0, std::ios_base::end);
			uint64_t size_tmp = arc_input.tellg();
			uint64_t fs_search_size = size_tmp - local_arc_header.file_system_search_offset;
			arc_input.seekg(local_arc_header.file_system_search_offset);

			char* fs_search_data = new char[fs_search_size];
			arc_input.read(fs_search_data, fs_search_size);

			arc_input.close();

			uint64_t header_shift = local_fs_ctable_header.section_size - SIZE;
			local_arc_header.file_system_search_offset += header_shift;

			local_fs_ctable_header.comp_size = SIZE;
			local_fs_ctable_header.decomp_size = get_table_size();
			local_fs_ctable_header.section_size += header_shift;

			arc_output.open(path, std::ios::in | std::ios::out | std::ios::binary);

			arc_output.write(FROM(local_arc_header));
			arc_output.seekp(local_arc_header.file_system_offset);
			arc_output.write(FROM(local_fs_ctable_header));
			arc_output.write(buffer, SIZE);
			arc_output.write(fs_search_data, fs_search_size);
			delete[] fs_search_data;
		}
		else {
			arc_input.close();

			arc_output.open(path, std::ios::in | std::ios::out | std::ios::binary);

			arc_output.seekp(local_arc_header.file_system_offset);
			local_fs_ctable_header.comp_size = SIZE;
			local_fs_ctable_header.decomp_size = get_table_size();
			arc_output.write(FROM(local_fs_ctable_header));
			arc_output.write(buffer, SIZE);

		}
		delete[] buffer;
		
		arc_output.close();

		std::cout << "[ARCaveMan::FileSystem::WriteDirectlyToARC] Wrote filesystem data directly to " << path << "." << std::endl;

		return 0;

	}
	
	/*~FileSystem() {
		delete[] extra_data_end;
	}*/
};

class FileSystemSearch {
public:
	FileSystemSearchHeader file_system_search_header;
	std::vector<HashToIndex> folder_path_to_index;
	std::vector<FolderPathListEntry> folder_path_list;
	std::vector<HashToIndex> path_to_index;
	std::vector<uint32_t> path_list_indices;
	std::vector<PathListEntry> path_list;
	FileSystemSearch(std::istream& stream) {
		std::cout << "[ARCaveMan::FileSystemSearch] Parsing data from std::istream buffer..." << std::endl;
		
		stream.read(INTO(file_system_search_header));

		READ_INTO_VEC(stream, file_system_search_header.folder_path_to_index_count, folder_path_to_index, HashToIndex);
		READ_INTO_VEC(stream, file_system_search_header.folder_path_to_index_count, folder_path_list, FolderPathListEntry);
		READ_INTO_VEC(stream, file_system_search_header.unk3, path_to_index, HashToIndex);
		READ_INTO_VEC(stream, file_system_search_header.unk4, path_list_indices, uint32_t);
		READ_INTO_VEC(stream, file_system_search_header.unk3, path_list, PathListEntry);
	}
	FileSystemSearch(char* buf, uint64_t size) {
		std::cout << "[ARCaveMan::FileSystemSearch] Parsing data from char array (buffer)..." << std::endl;
		uint64_t pos = 0;

		MEMCPY_INTO(buf, file_system_search_header, pos);

		MEMCPY_INTO_VEC(file_system_search_header.folder_path_to_index_count, folder_path_to_index, HashToIndex, buf, pos);
		MEMCPY_INTO_VEC(file_system_search_header.folder_path_to_index_count, folder_path_list, FolderPathListEntry, buf, pos);
		MEMCPY_INTO_VEC(file_system_search_header.unk3, path_to_index, HashToIndex, buf, pos);
		MEMCPY_INTO_VEC(file_system_search_header.unk3, path_list_indices, uint32_t, buf, pos);
		MEMCPY_INTO_VEC(file_system_search_header.unk4, path_list, PathListEntry, buf, pos);

	}
	FileSystemSearch() {}
	void write(std::string path) {
		std::cout << "[ARCaveMan::FileSystemSearch::Write] Started export process..." << std::endl;
		std::ofstream file(path, std::ios::out | std::ios::binary);

		file.write(FROM(file_system_search_header));

		WRITE_FROM_VEC(file, folder_path_to_index);
		WRITE_FROM_VEC(file, folder_path_list);
		WRITE_FROM_VEC(file, path_to_index);
		WRITE_FROM_VEC(file, path_list_indices);
		WRITE_FROM_VEC(file, path_list);
	}
	uint64_t write_compressed(std::string path) {
		std::cout << "[ARCaveMan::FileSystemSearch::Write] Started export process..." << std::endl;
		uint64_t SIZE = sizeof(file_system_search_header) + VEC_SIZE(folder_path_to_index) + VEC_SIZE(folder_path_list) +
			VEC_SIZE(path_to_index) + VEC_SIZE(path_list_indices) + VEC_SIZE(path_list);

		char* buffer = new char[SIZE]; uint64_t pos = 0;

		MEMCPY_FROM(buffer, file_system_search_header, pos);

		MEMCPY_FROM_VEC(buffer, pos, folder_path_to_index);
		MEMCPY_FROM_VEC(buffer, pos, folder_path_list);
		MEMCPY_FROM_VEC(buffer, pos, path_to_index);
		MEMCPY_FROM_VEC(buffer, pos, path_list_indices);
		MEMCPY_FROM_VEC(buffer, pos, path_list);

		char* buffer_out = new char[SIZE];
		std::cout << "[ARCaveMan::FileSystem::Write] Compressing data.arc file system..." << std::endl;
		uint64_t out_size = ZSTD_compress(buffer_out, SIZE, buffer, SIZE, ZSTD_maxCLevel());
		delete[] buffer;
		std::cout << "[ARCaveMan::FileSystem::Write] Writing data.arc file system to " << path << "..." << std::endl;
		std::ofstream file(path, std::ios::out | std::ios::binary);
		file.write(buffer_out, out_size);
		file.close();
		delete[] buffer_out;
		return out_size;
	}
	void add_folder_recursive(std::string path) {
		std::string parent_str = path.substr(0, path.find_last_of("/") - 1);
		if (path == parent_str + "/") {
			FolderPathListEntry{ HashToIndex::from_hash40(Hash40::from_str(path), 0xFFFFFF), HashToIndex::from_hash40(Hash40::from_str(""), 0x400000), };
		}

	}
};

class Arc {
public:
	ArcHeader arc_header;
	FileSystemCompressedTableHeader file_system_comp_table_header;
	FileSystem file_system;
	FileSystemSearchCompressedTableHeader file_system_search_comp_table_header;
	FileSystemSearch file_system_search;
	PatchSectionCompressedTableHeader patch_section_comp_table_header;
	PatchSection patch_section;
	
	Arc(std::string path) {
		std::cout << "[ARCaveMan::ARC] Opening data.arc..." << std::endl;
		std::ifstream file(path, std::ios::in | std::ios::binary);
		file.read(INTO(arc_header));
		file.seekg(arc_header.file_system_offset);

		//filesystem
		file.read(INTO(file_system_comp_table_header));
		char* comp_data = new char[file_system_comp_table_header.comp_size];
		char* decomp_data = new char[file_system_comp_table_header.decomp_size];
		file.read(comp_data, file_system_comp_table_header.comp_size);
		std::cout << "[ARCaveMan::ARC] Decompressing file system data..." << std::endl;
		if (ZSTD_isError(ZSTD_decompress(decomp_data, file_system_comp_table_header.decomp_size, comp_data, file_system_comp_table_header.comp_size)))
			std::cout << "[ARCaveMan::ARC] Failed to decompress file system data." << std::endl;
		delete[] comp_data;
		std::cout << "[ARCaveMan::ARC] Initializing file system..." << std::endl;
		file_system = { decomp_data, file_system_comp_table_header.decomp_size };
		delete[] decomp_data;

		// filesystemsearch
		file.seekg(arc_header.file_system_search_offset);
		file.read(INTO(file_system_search_comp_table_header));
		comp_data = new char[file_system_search_comp_table_header.comp_size];
		decomp_data = new char[file_system_search_comp_table_header.decomp_size];
		file.read(comp_data, file_system_search_comp_table_header.comp_size);
		std::cout << "[ARCaveMan::ARC] Decompressing file system search data..." << std::endl;
		if (ZSTD_isError(ZSTD_decompress(decomp_data, file_system_search_comp_table_header.decomp_size, comp_data, file_system_search_comp_table_header.comp_size)))
			std::cout << "[ARCaveMan::ARC] Failed to decompress file system search data." << std::endl;
		delete[] comp_data;
		std::cout << "[ARCaveMan::ARC] Initializing file system search..." << std::endl;
		file_system_search = { decomp_data, file_system_search_comp_table_header.decomp_size };
		delete[] decomp_data;
		
		// patchsection
		if (arc_header.patch_section_offset != 0) {
			file.seekg(arc_header.patch_section_offset);
			file.read(INTO(patch_section_comp_table_header));
			comp_data = new char[patch_section_comp_table_header.comp_size];
			decomp_data = new char[patch_section_comp_table_header.decomp_size];
			file.read(comp_data, patch_section_comp_table_header.comp_size);
			std::cout << "[ARCaveMan::ARC] Decompressing file system search data..." << std::endl;
			if (ZSTD_isError(ZSTD_decompress(decomp_data, patch_section_comp_table_header.decomp_size, comp_data, file_system_search_comp_table_header.comp_size)))
				std::cout << "[ARCaveMan::ARC] Failed to decompress file system search data." << std::endl;
			delete[] comp_data;
			std::cout << "[ARCaveMan::ARC] Initializing file system search..." << std::endl;
			patch_section = { decomp_data, patch_section_comp_table_header.decomp_size };
			delete[] decomp_data;
		}
		
		//file_system_search.write("dump_fs_search.tbl");

		file.close();
		std::cout << "[ARCaveMan::ARC] Completed ARC Initialization." << std::endl;

	}
	
	void REBUILD_DIR_HASH_TO_INFO_INDEX() {
		std::cout << "[ARCaveMan::DirHashToInfoIndex] Rebuilding DirHashToInfoIndices..." << std::endl;
		file_system.dir_hash_to_info_index.clear();
		for (int x = 0; x < file_system.dir_infos.size(); x++) {
			const auto& dir_info = file_system.dir_infos[x];
			auto hashtoidx = HashToIndex::from_hash40(dir_info.path.as_hash40(), x);
			file_system.dir_hash_to_info_index.push_back(hashtoidx);
		}
	}

	FileInfoToFileData& get_fileinfotodata(FileInfo& fileinfo) {
		return file_system.file_info_to_datas[(uint64_t)fileinfo.info_to_data_index + fileinfo.flags.is_regional * 2];
	}
	
	FileData& fileinfo_to_filedata(FileInfo& fileinfo) {
		return file_system.file_datas[get_fileinfotodata(fileinfo).file_data_index];
	}
	
	StreamData& stream_lookup_by_hash(Hash40 hash) {
		for(const auto& entry : file_system.stream_entries){
			if (entry.hash == hash.crc) {
				return file_system.stream_datas[file_system.stream_file_indices[entry.index]];
			}
		}
	}
	
	FileInfo& file_lookup_by_hash(Hash40 hash) {
		auto bucket_index = hash.as_u64() % file_system.file_info_buckets.size();
		auto& bucket = file_system.file_info_buckets[bucket_index];
		for (int x = bucket.start; x < bucket.start + bucket.count; x++) {
			if (file_system.file_hash_to_path_index[x].as_hash40() == hash) {
				auto file_info_index_index = file_system.file_paths[file_system.file_hash_to_path_index[x].index].path.index; // top tier variable naming
				auto index = file_system.file_info_indices[file_info_index_index].file_info_index; //Yeah that totally makes sense - _ - Man the arc sucks.;
				return file_system.file_infos[index];
			}
		}
	}
	
	void* get_directory_dependency(DirInfo& dir_info){
		if (dir_info.flags.redirected){
			auto directory_index = file_system.folder_offsets[dir_info.path.index].directory_index;
			if (directory_index != 0xFFFFFF){
				if (dir_info.flags.is_symlink) {
					return &file_system.dir_infos[directory_index]; //Intermediate DirInfo ?
				}
				else {
					return &file_system.folder_offsets[directory_index];
				}
			}
			else {
				return nullptr;
			}
		}
		else {
			return nullptr;
		}
	}

	void resolve_dirinfo_and_diroffset(DirInfo& in_dir_info, DirInfo* out_dir_info, DirectoryOffset* out_dir_offset) {
		if (in_dir_info.flags.redirected) {
			auto directory_index = file_system.folder_offsets[in_dir_info.path.index].directory_index;
			if (directory_index != 0xFFFFFF) {
				if (in_dir_info.flags.is_symlink) {
					*out_dir_info = file_system.dir_infos[directory_index]; //Intermediate DirInfo ?
					*out_dir_offset = file_system.folder_offsets[file_system.dir_infos[directory_index].path.index];
				}
				else {
					for (const auto& elem : file_system.dir_infos) {
						if (elem.path.index == directory_index) {
							*out_dir_info = elem;
						}
					}
					*out_dir_offset = file_system.folder_offsets[directory_index];
				}
			}
			else {
				//return nullptr;
			}
		}
		else {
			*out_dir_info = in_dir_info;
			*out_dir_offset = file_system.folder_offsets[in_dir_info.path.index];
		}
	}
	
	DirInfo& get_dir_info_from_path_hash(Hash40 hash) {
		for (const auto& dir_info_idx : file_system.dir_hash_to_info_index) {
			if (dir_info_idx.as_hash40() == hash) {
				return file_system.dir_infos[dir_info_idx.index];
			}
		}
	}
	
	uint32_t get_dir_info_index_from_path_hash(Hash40 hash) {
		for (const auto& dir_info_idx : file_system.dir_hash_to_info_index) {
			if (dir_info_idx.as_hash40() == hash) {
				return dir_info_idx.index;
			}
		}
	}
	
	uint32_t get_shared_file(Hash40 hash) {
		auto& file_info = file_lookup_by_hash(hash);
		auto new_hash = file_system.file_paths[file_info.file_path_index].path.as_hash40();
		if (new_hash == hash)
			return file_info.file_path_index;
		else
			return get_shared_file(new_hash);
	}

	DirectoryOffset get_dir_offset_from_fileinfo(FileInfo& fileinfo) {
		auto& file_info_to_data = get_fileinfotodata(fileinfo);
		auto folder_offset_index = file_info_to_data.folder_offset_index;
		return file_system.folder_offsets[folder_offset_index];
	}
	
	DirectoryOffset& get_dir_offset_from_dir_info(DirInfo& dir_info) {
		return file_system.folder_offsets[dir_info.path.index];
	}
	
	DirInfo& get_dir_info_from_fileinfo(FileInfo& fileinfo) {
		for (auto& elem : file_system.dir_infos) {
			if (elem.path.index == get_fileinfotodata(fileinfo).folder_offset_index) {
				return elem;
			}
		}
	}

	// NOT TESTED
	DirInfo& get_dir_info_from_dir_offset(DirectoryOffset& diroffset) {
		
		auto index = (&diroffset - &file_system.folder_offsets[0]);
		
		for (auto& elem : file_system.dir_infos) {
			if (elem.path.index == index) {
				return elem;
			}
		}
	}

	std::string get_string_from_fileinfo(FileInfo& fileinfo) {
		return Hashes::unhash_from_labels(file_system.file_paths[fileinfo.file_path_index].path.as_hash40());
	}
	/*
	void append_stream(Hash40 hash, uint64_t size) {// Only works for music in stream:/sound/bgm/. Sorry m8.
		auto TARGET_START_IDX = file_system.quick_dirs[0].index; // Hard code
		auto INSERT_AT = 0;
		for (int x = 0; x < file_system.quick_dirs.size(); x++) {
			if (file_system.quick_dirs[x].index == TARGET_START_IDX) {
				INSERT_AT = file_system.quick_dirs[x].index + file_system.quick_dirs[x].count;
				file_system.quick_dirs[x].count++;
			}
			else if (file_system.quick_dirs[x].index > TARGET_START_IDX)
				file_system.quick_dirs[x].index++;
		}

		auto hashtoindex = HashToIndex::from_hash40(hash, file_system.stream_entries.size());
		file_system.stream_hash_to_entries.insert(file_system.stream_hash_to_entries.begin() + INSERT_AT, hashtoindex);
		// sorting meme
		std::sort(file_system.stream_hash_to_entries.begin() + TARGET_START_IDX, file_system.stream_hash_to_entries.begin() + INSERT_AT);

		auto streamentry = StreamEntry{ hash.crc, hash.len, (uint32_t)file_system.stream_file_indices.size(), 0 };
		file_system.stream_entries.push_back(streamentry);

		file_system.stream_file_indices.push_back(file_system.stream_datas.size());

		uint64_t NEW_OFFSET = file_system.stream_datas[-1].offset + file_system.stream_datas[-1].size;

		if (NEW_OFFSET % 8 != 0)
			NEW_OFFSET = ((NEW_OFFSET / 8) + 1)*8;

		auto streamdata = StreamData{ size, NEW_OFFSET };
		file_system.stream_datas.push_back(streamdata);
		file_system.stream_header.stream_hash_count++;
		file_system.stream_header.stream_file_index_count++;
		file_system.stream_header.stream_offset_entry_count++;
		// file_system.quick_dirs[0] is stream:/sound/bgm/
	}
	*/
	//  Works for files directly referenced, but not ones used as part of a directory where all of the files are grabbed at once.
	void addition(FileInfo& base_fileinfo, std::string full_path, std::string extension, std::string directory_path, std::string file_name) {
		auto base_file_info_flags = base_fileinfo.flags;
		auto& base_file_info_to_data = get_fileinfotodata(base_fileinfo);

		auto base_dir_offset_index = base_file_info_to_data.folder_offset_index;
		auto base_load_type = base_file_info_to_data.load_type;

		auto& base_filedata = fileinfo_to_filedata(base_fileinfo);

		auto base_offset_in_folder = base_filedata.offset_in_folder;
		auto base_comp_size = base_filedata.comp_size;
		auto base_decomp_size = base_filedata.decomp_size;
		auto base_file_data_flags = base_filedata.flags;

		FilePath file_path = {
			HashToIndex::from_hash40(Hash40::from_str(full_path), file_system.file_info_indices.size()),
			HashToIndex::from_hash40(Hash40::from_str(extension), 0),
			HashToIndex::from_hash40(Hash40::from_str(directory_path), 0),
			HashToIndex::from_hash40(Hash40::from_str(file_name), 0)
		};

		file_system.file_paths.push_back(file_path);

		// if it looks confusing why we subtract one its for a reason
		// subtract one from an array length bc we already push_backed, and thats the idx that will be referenced
		FileInfoIndex file_info_index = { base_dir_offset_index, (uint32_t)file_system.file_infos.size() };

		file_system.file_info_indices.push_back(file_info_index);

		FileInfo file_info = { file_system.file_paths.size() - 1, file_system.file_info_indices.size() - 1, file_system.file_info_to_datas.size(), base_file_info_flags };

		file_system.file_infos.push_back(file_info);

		FileInfoToFileData file_info_to_data = { base_dir_offset_index, file_system.file_datas.size(), file_system.file_infos.size() - 1, base_load_type };

		file_system.file_info_to_datas.push_back(file_info_to_data);

		FileData file_data = { base_offset_in_folder, base_comp_size, base_decomp_size, base_file_data_flags };

		file_system.file_datas.push_back(file_data);
		file_system.hash_index_group_count++;
		file_system.file_system_header.file_info_path_count++;
		file_system.file_system_header.file_info_index_count++;
		file_system.file_system_header.file_info_count++;
		file_system.file_system_header.file_info_sub_index_count++;
		file_system.file_system_header.file_data_count++;
		std::cout << "[ARCaveMan::Addition] Added " << full_path << " to data.arc using Generic file addition." << std::endl;
	}

	void file_in_dir_addition(std::string dir_info_name, FileInfo& base_fileinfo, std::string full_path, std::string extension, std::string directory_path, std::string file_name) {
		file_in_dir_addition(get_dir_info_from_path_hash(Hash40::from_str(dir_info_name)), base_fileinfo, full_path, extension, directory_path, file_name);
	}
	
	void file_in_dir_addition(DirInfo& dirinfo, FileInfo& base_fileinfo, std::string full_path, std::string extension, std::string directory_path, std::string file_name) {

		Hashes::push(full_path);
		auto& diroffset = get_dir_offset_from_dir_info(dirinfo);
		/*
		if (dirinfo.flags.redirected) {
			auto directory_index = file_system.folder_offsets[dirinfo.path.index].directory_index;
			if (directory_index != 0xFFFFFF) {
				
				if (dirinfo.flags.is_symlink) {
					dirinfo_cpy = file_system.dir_infos[directory_index]; // Intermediate DirInfo ?
				}
				
			}
		}
		*/

		dirinfo.file_count++;
		diroffset.file_count++;

		// iterate through DirInfos, incrementing their file_info_start_index if its greater than our target dir_info_index.
		// if the current DirInfo happens to be redirected (according to its DirInfoFlags),
		// then use the DirOffset pointed to by the path HashToIndex to get the real directory_index and associated DirOffset.
		// if the directory index isnt 0xFFFFFF and the current DirInfo isnt symlinked, 
		// increment the file_start_index (points to fileinfos) if its greater than our target.
		// if the current DirInfo is not redirected, then just use the DirOffset pointed to by the path HashToIndex directly,
		// once again increment the file_start_index (points to filedatas this time) if its greater than our target folder_offset_index.
		// NOTE: this code DOES account for redirection, but the above stuff DOES NOT.
		// also it straight up doesnt work lol
		for (auto& entry : file_system.dir_infos) {
			if (entry.file_info_start_index > dirinfo.file_info_start_index)
				entry.file_info_start_index++;
			auto directory_index = file_system.folder_offsets[entry.path.index].directory_index;
			if (entry.flags.redirected && directory_index != 0xFFFFFF && !entry.flags.is_symlink) {
				/*
				if (dir_info.flags.is_symlink) {
					return &file_system.dir_infos[directory_index]; // Intermediate DirInfo ?
				}
				*/
				if (file_system.folder_offsets[directory_index].file_start_index > dirinfo.file_info_start_index)
					file_system.folder_offsets[directory_index].file_start_index++;
			}
			else {
				auto& dir_offset_entry = get_dir_offset_from_dir_info(entry);
				if (dir_offset_entry.file_start_index > diroffset.file_start_index)
					dir_offset_entry.file_start_index++;
			}
		}

		auto fileinfo_insert_index = dirinfo.file_info_start_index + dirinfo.file_count - 1; // subtracted 1 since it was incremented above
		auto filedata_insert_index = diroffset.file_start_index + diroffset.file_count - 1; // subtracted 1 since it was incremented above


		auto base_file_info_flags = base_fileinfo.flags;
		auto& base_file_info_to_data = get_fileinfotodata(base_fileinfo);

		auto base_dir_offset_index = dirinfo.path.index;
		auto base_load_type = base_file_info_to_data.load_type;

		auto& base_filedata = fileinfo_to_filedata(base_fileinfo);

		auto base_offset_in_folder = base_filedata.offset_in_folder;
		auto base_comp_size = base_filedata.comp_size;
		auto base_decomp_size = base_filedata.decomp_size;
		auto base_file_data_flags = base_filedata.flags;

		FilePath file_path = {
			HashToIndex::from_hash40(Hash40::from_str(full_path), file_system.file_info_indices.size()),
			HashToIndex::from_hash40(Hash40::from_str(extension), 0),
			HashToIndex::from_hash40(Hash40::from_str(directory_path), 0),
			HashToIndex::from_hash40(Hash40::from_str(file_name), 0)
		};

		file_system.file_paths.push_back(file_path);

		// if it looks confusing why we subtract one its for a reason
		// subtract one from an array length bc we already pushed to the back, and thats the idx that will be referenced
		FileInfoIndex file_info_index = { base_dir_offset_index, fileinfo_insert_index };

		file_system.file_info_indices.push_back(file_info_index);

		FileInfo file_info = { file_system.file_paths.size() - 1, file_system.file_info_indices.size() - 1, file_system.file_info_to_datas.size(), base_file_info_flags };

		file_system.file_infos.insert(file_system.file_infos.begin() + fileinfo_insert_index, file_info);

		FileInfoToFileData file_info_to_data = { base_dir_offset_index, filedata_insert_index, fileinfo_insert_index, base_load_type };

		file_system.file_info_to_datas.push_back(file_info_to_data);

		FileData file_data = { base_offset_in_folder, base_comp_size, base_decomp_size, base_file_data_flags };

		file_system.file_datas.insert(file_system.file_datas.begin() + filedata_insert_index, file_data);

		file_system.hash_index_group_count++;
		file_system.file_system_header.file_info_path_count++;
		file_system.file_system_header.file_info_index_count++;
		file_system.file_system_header.file_info_count++;
		file_system.file_system_header.file_info_sub_index_count++;
		file_system.file_system_header.file_data_count++;

		for (auto& info_to_data : file_system.file_info_to_datas) {
			if (info_to_data.file_info_idx > fileinfo_insert_index)
				info_to_data.file_info_idx++;
			if (info_to_data.file_data_index > filedata_insert_index)
				info_to_data.file_data_index++;
		}
		
		for (auto& file_info_index : file_system.file_info_indices) {
			if (file_info_index.file_info_index > fileinfo_insert_index)
				file_info_index.file_info_index++;
		}

		std::cout << "[ARCaveMan::Addition] Added " << full_path << " to " << Hashes::unhash_from_labels(dirinfo.path.as_hash40()) << " in data.arc using File-In-Directory file addition." << std::endl;
	}
	
	void print_dirinfos(DirInfo dirinfo) {
		

		if (dirinfo.flags.redirected) {
			
			auto directory_index = file_system.folder_offsets[dirinfo.path.index].directory_index;
			if (directory_index != 0xFFFFFF) {
				if (dirinfo.flags.is_symlink) {
					std::cout << "turns out " << Hashes::unhash_from_labels(dirinfo.path.as_hash40()) << " is redirected.  its redirected path is: " << Hashes::unhash_from_labels(file_system.dir_infos[directory_index].path.as_hash40()) << std::endl;
					dirinfo = file_system.dir_infos[directory_index]; //Intermediate DirInfo ?
				}
			}
		}

		std::cout << "Directory: " << Hashes::unhash_from_labels(dirinfo.path.as_hash40()) << std::endl;

		for (int x = 0; x < dirinfo.file_count; x++) {
			auto path_hash = file_system.file_paths[file_system.file_infos[dirinfo.file_info_start_index + x].file_path_index].path.as_hash40();
			std::cout << "File: " << Hashes::unhash_from_labels(path_hash) << std::endl;
		}

		for (int x = 0; x < dirinfo.child_dir_count; x++) {
			print_dirinfos(file_system.dir_infos[dirinfo.child_dir_start_index + x]);
		}

		//std::cout << Hashes::unhash_from_labels(dirinfo.path.as_hash40()) << std::endl;
	}

	void directory_addition_recursive(std::string base_dir_path, std::string base_dir_name, std::string new_dir_path, std::string new_dir_name, std::string base_dir_to_replace, std::string new_dir_replace_with) {
	    // hype
	    // this DOES NOT WORK as it does not account for redirection which is like super important
	    // also it does not edit the parent hashes, which is also super important

		auto old_dirinfo = get_dir_info_from_path_hash(Hash40::from_str(base_dir_path));
		auto old_diroffset = get_dir_offset_from_dir_info(old_dirinfo);

		DirInfo new_dirinfo = old_dirinfo;
		DirectoryOffset new_diroffset = get_dir_offset_from_dir_info(old_dirinfo);

	    new_diroffset.file_start_index = file_system.file_datas.size(); // set start indices to end of fileinfo and filedata arrays
	    new_dirinfo.file_info_start_index = file_system.file_infos.size();

	    new_dirinfo.file_count = 0; // this is modified by file_in_dir_addition so we gucci

	    //file_system.dir_hash_to_info_index.push_back(HashToIndex::from_hash40(Hash40::from_str("fighter/pickel/c08"), (uint32_t)file_system.dir_infos.size()));

	    new_dirinfo.name = Hash40::from_str(new_dir_name);// "c08"
	    new_dirinfo.path = HashToIndex::from_hash40(Hash40::from_str(new_dir_path), (uint32_t)file_system.folder_offsets.size());
		new_dirinfo.parent = Hash40::from_str(new_dir_path.substr(0, new_dir_path.find_last_of(new_dir_name) + 1));

		if (old_dirinfo.child_dir_start_index > 0)
			new_dirinfo.child_dir_start_index = file_system.dir_infos.size() + 1; // set start index to end of dirinfo array

	    file_system.dir_infos.push_back(new_dirinfo);
	    file_system.folder_offsets.push_back(new_diroffset);

	    file_system.folder_child_hashes.push_back(HashToIndex::from_hash40(Hash40::from_str(new_dir_path), (uint32_t)file_system.dir_infos.size() - 1)); // potentially not needed? maybe

	    for (int iter = 0; iter < old_dirinfo.file_count; iter++) { // add all files in the original directory to the new directory.
			auto fileinfo = file_system.file_infos[(uint64_t)old_dirinfo.file_info_start_index + iter];
			auto& FP = file_system.file_paths[fileinfo.file_path_index];
			auto new_path = replace_all(Hashes::unhash_from_labels(FP.path.as_hash40()), base_dir_to_replace, new_dir_replace_with);
			auto new_parent = replace_all(Hashes::unhash_from_labels(FP.parent.as_hash40()), base_dir_to_replace, new_dir_replace_with);
			file_in_dir_addition(file_system.dir_infos[file_system.dir_infos.size()-1], fileinfo, new_path, Hashes::unhash_from_labels(FP.ext.as_hash40()), new_parent, Hashes::unhash_from_labels(FP.file_name.as_hash40()));
	    }

		for (int iter = 0; iter < old_dirinfo.child_dir_count; iter++) {
			auto child_dir_base_path = Hashes::unhash_from_labels(file_system.dir_infos[old_dirinfo.child_dir_start_index + iter].path.as_hash40());
			auto child_dir_base_name = Hashes::unhash_from_labels(file_system.dir_infos[old_dirinfo.child_dir_start_index + iter].name);
			directory_addition_recursive(child_dir_base_path, child_dir_base_name, replace_all(child_dir_base_path, base_dir_to_replace, new_dir_replace_with), replace_all(child_dir_base_name, base_dir_to_replace, new_dir_replace_with), base_dir_to_replace, new_dir_replace_with);
		}

		file_system.file_system_header.folder_offset_count_1++;
		file_system.file_system_header.folder_count++;
		file_system.file_system_header.hash_folder_count++;

	}

	void directory_addition(std::string base_dir_path, std::string base_dir_name, std::string new_dir_path, std::string new_dir_name, std::string base_dir_to_replace, std::string new_dir_replace_with) {
		// hype
		// this DOES NOT WORK as it does not account for redirection which is like super important
		// also it does not edit the parent hashes, which is also super important

		auto old_dirinfo = get_dir_info_from_path_hash(Hash40::from_str(base_dir_path));
		auto old_diroffset = get_dir_offset_from_dir_info(old_dirinfo);

		DirInfo new_dirinfo = old_dirinfo;
		DirectoryOffset new_diroffset = get_dir_offset_from_dir_info(old_dirinfo);

		new_diroffset.file_start_index = file_system.file_datas.size(); // set start indices to end of fileinfo and filedata arrays
		new_dirinfo.file_info_start_index = file_system.file_infos.size();

		new_dirinfo.file_count = 0; // this is modified by file_in_dir_addition so we gucci

		new_dirinfo.name = Hash40::from_str(new_dir_name);// "c08"
		new_dirinfo.path = HashToIndex::from_hash40(Hash40::from_str(new_dir_path), (uint32_t)file_system.folder_offsets.size());
		new_dirinfo.parent = Hash40::from_str(new_dir_path.substr(0, new_dir_path.find_last_of(new_dir_name) + 1));

		if (old_dirinfo.child_dir_start_index > 0)
			new_dirinfo.child_dir_start_index = file_system.dir_infos.size() + 1; // set start index to end of dirinfo array

		file_system.dir_infos.push_back(new_dirinfo);
		file_system.folder_offsets.push_back(new_diroffset);

		file_system.folder_child_hashes.push_back(HashToIndex::from_hash40(Hash40::from_str(new_dir_path), (uint32_t)file_system.dir_infos.size() - 1)); // potentially not needed? maybe

		for (int iter = 0; iter < old_dirinfo.file_count; iter++) { // add all files in the original directory to the new directory.
			auto fileinfo = file_system.file_infos[(uint64_t)old_dirinfo.file_info_start_index + iter];
			auto& FP = file_system.file_paths[fileinfo.file_path_index];
			auto new_path = replace_all(Hashes::unhash_from_labels(FP.path.as_hash40()), base_dir_to_replace, new_dir_replace_with);
			auto new_parent = replace_all(Hashes::unhash_from_labels(FP.parent.as_hash40()), base_dir_to_replace, new_dir_replace_with);
			//file_in_dir_addition(file_system.dir_infos[file_system.dir_infos.size() - 1], fileinfo, new_path, Hashes::unhash_from_labels(FP.ext.as_hash40()), new_parent, Hashes::unhash_from_labels(FP.file_name.as_hash40()));
			addition(fileinfo, new_path, Hashes::unhash_from_labels(FP.ext.as_hash40()), new_parent, Hashes::unhash_from_labels(FP.file_name.as_hash40()));
		}

		file_system.file_system_header.folder_offset_count_1++;
		file_system.file_system_header.folder_count++;
		file_system.file_system_header.hash_folder_count++;

	}

	void expand_compressed_size() {
		// fard
	}

	void add_c08_shared() {
		const auto old_dirinfo = get_dir_info_from_path_hash(Hash40::from_str("fighter/pickel/c00"));
		auto new_dirinfo = DirInfo{ HashToIndex::from_hash40(Hash40::from_str("fighter/pickel/c08"), old_dirinfo.path.index), Hash40::from_str("c08"), old_dirinfo.parent, 0, 0, 0, 0, 0, 0, DirInfoFlags{false, true, false, true, false} };
		file_system.dir_infos.push_back(new_dirinfo);

		for (int iter = 0; iter < old_dirinfo.file_count; iter++) { // copy all the old files' filepaths and just edit the hashes. the fileinfoindexindex will still point to the right file.
			const auto& fileinfo = file_system.file_infos[(uint64_t)old_dirinfo.file_info_start_index + iter];
			FilePath FP = file_system.file_paths[fileinfo.file_path_index];
			
			//auto& filepath = file_system.file_paths[get_shared_file(fileinfo)];

			auto new_path_hash = Hash40::from_str(replace_all(Hashes::unhash_from_labels(FP.path.as_hash40()), "c00", "c08"));
			auto new_parent_hash = Hash40::from_str(replace_all(Hashes::unhash_from_labels(FP.parent.as_hash40()), "c00", "c08"));

			FP.path.set_hash40(new_path_hash);
			FP.parent.set_hash40(new_parent_hash);

			file_system.file_paths.push_back(FP);
			file_system.file_system_header.file_info_path_count++;
		}
		file_system.file_system_header.folder_count++;
		
	}

	void print_all_symlinked_dirs() {
		for (const auto& elem : file_system.dir_infos) {
			if (elem.flags.is_symlink) {
				std::cout << "Directory: " << Hashes::unhash_from_labels(elem.path.as_hash40()) << " is symlinked with: "
					<< Hashes::unhash_from_labels(file_system.dir_infos[file_system.folder_offsets[elem.path.index].directory_index].path.as_hash40())
					<< std::endl;
			}

		}
	}
};
};
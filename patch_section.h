
namespace ARCaveMan {

	struct PatchSectionHeader {
		uint32_t table_filesize;
		uint64_t bucket_count;
		uint64_t unk2;
		uint64_t folder_offset_count_1;
		uint64_t file_info_index_count;
	};

	class PatchSection {
	public:
		PatchSectionHeader patch_section_header;
		//...

		std::vector<DirectoryOffset> folder_offsets;

		PatchSection(std::istream& stream) {
			std::cout << "[ARCaveMan::PatchSection] Parsing data from std::istream buffer..." << std::endl;
		}

		PatchSection(char* buf, uint64_t size) {
			std::cout << "[ARCaveMan::PatchSection] Parsing data from char array (buffer)..." << std::endl;
		}

		PatchSection() {}
	};
}
#pragma once
#include <cstdint>
#include <fstream>
#include <zlib.h>
#include <string>
#include <unordered_map>

struct Hash40 {
	uint32_t crc;
	uint32_t len;
	static Hash40 from_str(std::string str) {
		return { crc32(0, (unsigned char*)str.c_str(), str.size()), (uint32_t)str.size() };
	}
	//Hash40() { crc = 0; len = 0; }
	uint64_t as_u64()const {
		return *(uint64_t*)this;
	}
	bool operator ==(const Hash40& other) {
		return other.crc == crc && other.len == len;
	}
};

namespace Hashes {

	std::unordered_map<uint64_t, std::string> MAP;

	void init() {
		std::cout << "[ARCaveMan::Hashes] Initalizing hashmap from hashes.txt..." << std::endl;
		std::ifstream label_file("hashes.txt", std::ios::in);
		if (!label_file) {
			std::cout << "[ARCaveMan::Hashes] Could not find hashes.txt. Aborting..." << std::endl;
			return;
		}
		std::string line;
		while (std::getline(label_file, line)) {
			MAP.insert(std::make_pair(Hash40::from_str(line).as_u64(), line));
		}
		label_file.close();
		std::cout << "[ARCaveMan::Hashes] Done creating hashmap." << std::endl;
	}

	void push(std::string line) {
		MAP.insert(std::make_pair(Hash40::from_str(line).as_u64(), line));
	}

	std::string unhash_from_labels(const Hash40& hash) {
		
		try {
			MAP.at(hash.as_u64());
		}
		catch (std::exception& error) {
			return "UNCRACKED_HASH";
		}
		
		return MAP[hash.as_u64()];
	}

};
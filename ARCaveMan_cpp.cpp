// ARCaveMan_cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <chrono>

#include <zstd.h>
#include "arc.h"

// size_t dstCapacity = ZSTD_decompress(void* dst, size_t dstCapacity, const void* src, size_t compressedSize);

void addition_test(Arc& arc) {
    FileInfo base_chara_1 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx"));

    arc.addition(
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_08.bntx"
    );
    arc.addition(
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_09.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_09.bntx"
    );
    arc.addition(
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_10.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_10.bntx"
    );
    arc.addition(
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_11.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_11.bntx"
    );

    arc.REBUILD_FILEINFOBUCKETS();

    arc.file_system.write("output_fs_edited.tbl");

    arc.file_system.write_compressed("output_fs_compressed.tbl");
}

void file_in_dir_addition_test(Arc& arc) {
    auto base_file_info = arc.file_lookup_by_hash(Hash40::from_str("fighter/pickel/model/body/c00/def_pickel_001_col.nutexb"));

    arc.file_in_dir_addition(
        "fighter/pickel/c00",
        base_file_info,
        "fighter/pickel/model/body/c00/steve.nutexb",
        "nutexb",
        "fighter/pickel/model/body/c00/",
        "steve.nutexb"
    );

    arc.REBUILD_FILEINFOBUCKETS();

    arc.file_system.write("output_fs_edited.tbl");

    arc.file_system.write_compressed("output_fs_compressed.tbl");
}

int file_addition_script(int argc, char* argv[])
{
    if (argc < 5) {
        std::cout << "not enough args lol" << std::endl;
        std::cout << "example:" << std::endl;
        std::cout << argv[0] << " data.arc ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx ui/replace_patch/chara/chara_1/chara_1_pickel_08.bntx output_fs.tbl";
        return -1;
    }
    
    Arc arc(argv[1]);

    std::string base_filename = argv[2];

    std::string filename = argv[3];

    std::string extension = filename.substr(filename.find_last_of(".") + 1);

    std::string path = filename.substr(0,filename.find_last_of("/") - 1);

    std::string basename = filename.substr(filename.find_last_of("/")+1);

    FileInfo base_fileinfo = arc.file_lookup_by_hash(Hash40::from_str(base_filename));

    arc.addition(
        base_fileinfo,
        filename,
        extension,
        path,
        basename
    );

    arc.REBUILD_FILEINFOBUCKETS();

    arc.file_system.write_compressed(argv[4]);
}

int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    Arc arc("D:\\AppData (yuzu)\\Local\\yuzu\\yuzu-windows-msvc\\user\\dump\\Super Smash Bros. Ultimate ARCs\\data_1300.arc");
    //Arc arc("D:\\AppData (yuzu)\\Local\\yuzu\\yuzu-windows-msvc\\user\\sdmc\\atmosphere\\contents\\01006A800016E000\\romfs\\data.arc");

    Hashes::init();

    file_in_dir_addition_test(arc);


    //arc.print_dirinfos(arc.get_dir_info_from_path_hash(Hash40::from_str("fighter/pickel/c00")));

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count()/1000 << "ms" << std::endl;
}



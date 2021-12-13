// ARCaveMan_cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
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
        "steve"
    );

    arc.REBUILD_FILEINFOBUCKETS();

    arc.file_system.write("output_fs_edited.tbl");

    arc.file_system.write_compressed("output_fs_compressed.tbl");
}

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    Arc arc("D:\\AppData (yuzu)\\Local\\yuzu\\yuzu-windows-msvc\\user\\dump\\Super Smash Bros. Ultimate ARCs\\data_1300.arc");
    //Arc arc("D:\\AppData (yuzu)\\Local\\yuzu\\yuzu-windows-msvc\\user\\sdmc\\atmosphere\\contents\\01006A800016E000\\romfs\\data.arc");

    Hashes::init();

    FileInfo base_chara_1 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx"));

    arc.addition(
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_08.bntx"
    );

    arc.REBUILD_FILEINFOBUCKETS();

    auto& fileinfo = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_08.bntx"));

    auto hash40_ = arc.file_system.file_paths[fileinfo.file_path_index].path.as_hash40();

    std::cout << Hashes::unhash_from_labels(hash40_) << ", " << std::hex << hash40_.crc << std::endl;

    //file_in_dir_addition_test(arc);

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

// ARCaveMan_cpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <chrono>

#include <zstd.h>
#include "arc.h"

using namespace ARCaveMan;

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

    arc.file_system.REBUILD_FILEINFOBUCKETS();

    arc.file_system.write("output_fs_edited.tbl");

    arc.file_system.write_compressed("output_fs_compressed.tbl");
}

void addition_test_but_with_fid(Arc& arc) {
    FileInfo base_chara_1 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx"));

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel")),
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_08.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel")),
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_09.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_09.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel")),
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_10.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_10.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel")),
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_11.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_11.bntx"
    );

    arc.REBUILD_DIR_HASH_TO_INFO_INDEX();

    arc.file_system.REBUILD_FILEINFOBUCKETS();

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

    arc.file_system.REBUILD_FILEINFOBUCKETS();

    arc.file_system.write("output_fs_edited.tbl");

    arc.file_system.write_compressed("output_fs_compressed.tbl");
}

void directory_override(Arc& arc) {
    FileSystem new_filesystem{};

    new_filesystem.file_system_header = {
        0xcc,
        8,
        8,
        1,
        1,
        1,
        8,
        8,
        8,
        0,
        0,
        0,
        0x10,
        0x10,
        0,
        0,
        0,
        arc.file_system.file_system_header.version,
        0,
        0,
        {0,0},
        0,
        0
    };

    memset(new_filesystem.extra_data, 0, 0x50);

    new_filesystem.stream_header = {
        0,
        0,
        0,
        0
    };

    new_filesystem.hash_index_group_count = 8;
    new_filesystem.bucket_count = 2;

    new_filesystem.file_hash_to_path_index = {
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx"), 0),
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_01.bntx"), 1),
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_02.bntx"), 2),
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_03.bntx"), 3),
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_04.bntx"), 4),
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_05.bntx"), 5),
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_06.bntx"), 6),
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_07.bntx"), 7),
    };

    new_filesystem.file_paths = {
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx", 0),
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_01.bntx", 1),
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_02.bntx", 2),
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_03.bntx", 3),
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_04.bntx", 4),
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_05.bntx", 5),
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_06.bntx", 6),
        FilePath::from_full_path("ui/replace_patch/chara/chara_1/chara_1_pickel_07.bntx", 7),
    };

    new_filesystem.file_info_indices = {
        FileInfoIndex{ 0, 0 },
        FileInfoIndex{ 0, 1 },
        FileInfoIndex{ 0, 2 },
        FileInfoIndex{ 0, 3 },
        FileInfoIndex{ 0, 4 },
        FileInfoIndex{ 0, 5 },
        FileInfoIndex{ 0, 6 },
        FileInfoIndex{ 0, 7 },
    };

    new_filesystem.dir_hash_to_info_index = {
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel"), 0)
    };

    new_filesystem.dir_infos = {
        DirInfo{
            HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel"), 0),
            Hash40::from_str("pickel"),
            Hash40::from_str("ui/replace_patch/chara/chara_1"),
            0,
            0,
            0,
            8,
            0,
            0,
            DirInfoFlags{
                false,
                false,
                false,
                false,
                false,
            }
        }
    };

    auto old_dir_offset = arc.get_dir_offset_from_dir_info(arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel")));

    old_dir_offset.file_start_index = 0;
    old_dir_offset.directory_index = 0;

    new_filesystem.folder_offsets = {
        old_dir_offset
    };

    new_filesystem.folder_child_hashes = {
        HashToIndex::from_hash40(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel"), 0)
    };

    new_filesystem.file_infos = {
        FileInfo{
            0,
            0,
            0,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx")).flags
        },
        FileInfo{
            1,
            1,
            1,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_01.bntx")).flags
        },
        FileInfo{
            2,
            2,
            2,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_02.bntx")).flags
        },
        FileInfo{
            3,
            3,
            3,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_03.bntx")).flags
        },
        FileInfo{
            4,
            4,
            4,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_04.bntx")).flags
        },
        FileInfo{
            5,
            5,
            5,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_05.bntx")).flags
        },
        FileInfo{
            6,
            6,
            6,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_06.bntx")).flags
        },
        FileInfo{
            7,
            7,
            7,
            arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_07.bntx")).flags
        },
    };

    new_filesystem.file_info_to_datas = {
        FileInfoToFileData{
            0,
            0,
            0,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx"))).load_type,
        },
        FileInfoToFileData{
            0,
            1,
            1,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_01.bntx"))).load_type,
        },
        FileInfoToFileData{
            0,
            2,
            2,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_02.bntx"))).load_type,
        },
        FileInfoToFileData{
            0,
            3,
            3,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_03.bntx"))).load_type,
        },
        FileInfoToFileData{
            0,
            4,
            4,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_04.bntx"))).load_type,
        },
        FileInfoToFileData{
            0,
            5,
            5,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_05.bntx"))).load_type,
        },
        FileInfoToFileData{
            0,
            6,
            6,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_06.bntx"))).load_type,
        },
        FileInfoToFileData{
            0,
            7,
            7,
            arc.get_fileinfotodata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_07.bntx"))).load_type,
        },
    };

    new_filesystem.file_datas = {
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx"))),
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_01.bntx"))),
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_02.bntx"))),
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_03.bntx"))),
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_04.bntx"))),
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_05.bntx"))),
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_06.bntx"))),
        arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_07.bntx"))),
    };

    new_filesystem.REBUILD_FILEINFOBUCKETS();

    new_filesystem.write("patch_section.tbl");
    
    new_filesystem.write_compressed("patch_section_compressed.tbl");
}

void add_c08_steve(Arc& arc) {
    arc.directory_addition(
        "fighter/pickel/c00",
        "fighter/pickel/c08",
        "c00",
        "c08",
        "c00",
        "c08"
    );
    arc.REBUILD_DIR_HASH_TO_INFO_INDEX();
    arc.file_system.REBUILD_FILEINFOBUCKETS();
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

    arc.file_system.REBUILD_FILEINFOBUCKETS();

    arc.file_system.write_compressed(argv[4]);
}

void add_steve_ui_c08(Arc& arc) {

    FileInfo base_chara_0 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_0/chara_0_pickel_00.bntx"));

    FileInfo base_chara_1 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx"));

    FileInfo base_chara_2 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_2/chara_2_pickel_00.bntx"));

    FileInfo base_chara_3 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_3/chara_3_pickel_00.bntx"));

    FileInfo base_chara_4 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_4/chara_4_pickel_00.bntx"));

    FileInfo base_chara_6 = arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_6/chara_6_pickel_00.bntx"));

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_0/pickel")),
        base_chara_0,
        "ui/replace_patch/chara/chara_0/chara_0_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_0/",
        "chara_0_pickel_08.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/pickel")),
        base_chara_1,
        "ui/replace_patch/chara/chara_1/chara_1_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_1/",
        "chara_1_pickel_08.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_2/pickel")),
        base_chara_2,
        "ui/replace_patch/chara/chara_2/chara_2_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_2/",
        "chara_2_pickel_08.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_3/pickel")),
        base_chara_3,
        "ui/replace_patch/chara/chara_3/chara_3_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_3/",
        "chara_3_pickel_08.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_4/pickel")),
        base_chara_4,
        "ui/replace_patch/chara/chara_4/chara_4_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_4/",
        "chara_4_pickel_08.bntx"
    );

    arc.file_in_dir_addition(
        arc.get_dir_info_from_path_hash(Hash40::from_str("ui/replace_patch/chara/chara_6/pickel")),
        base_chara_6,
        "ui/replace_patch/chara/chara_6/chara_6_pickel_08.bntx",
        "bntx",
        "ui/replace_patch/chara/chara_6/",
        "chara_6_pickel_08.bntx"
    );
}

void new_test(Arc& arc) {
    arc.directory_addition("fighter/pickel/c00", "c00", "fighter/pickel/c08", "c08", "c00", "c08");
    auto old_dirinfo = arc.get_dir_info_from_path_hash(Hash40::from_str("fighter/pickel/c00/cmn"));
    auto cmn_idx = arc.get_dir_info_index_from_path_hash(Hash40::from_str("fighter/pickel/cmn"));
    auto new_dirinfo = DirInfo{ HashToIndex::from_hash40(Hash40::from_str("fighter/pickel/c08/cmn"), cmn_idx), Hash40::from_str("cmn"), Hash40::from_str("fighter/pickel/c08/"), 0, 0, 0, 0, 0, 0, old_dirinfo.flags };

    arc.file_system.folder_child_hashes.push_back(HashToIndex::from_hash40(Hash40::from_str("fighter/pickel/c08/cmn"), arc.file_system.dir_infos.size()));
    arc.file_system.dir_infos.push_back(new_dirinfo);

    auto camera_thingy_dirinfo = DirInfo{ {0, 0, 0}, {0, 0}, {0, 0}, 0, 0, 0, 0, 0, 0, {false, false, false, false, false} };
    arc.file_system.folder_child_hashes.push_back(HashToIndex::from_hash40(Hash40::from_str("fighter/pickel/c08/camera"), arc.file_system.dir_infos.size()));
    arc.file_system.dir_infos.push_back(camera_thingy_dirinfo);

    arc.file_system.file_system_header.folder_count += 2;
    arc.file_system.file_system_header.hash_folder_count += 2;

    arc.directory_addition("fighter/pickel/camera/c00", "c00", "fighter/pickel/camera/c08", "c08", "c00", "c08");

    old_dirinfo = arc.get_dir_info_from_path_hash(Hash40::from_str("fighter/pickel/c00/camera"));
    auto camera_idx = arc.get_dir_info_index_from_path_hash(Hash40::from_str("fighter/pickel/camera/c08"));
    arc.file_system.dir_infos[arc.file_system.dir_infos.size()-2] = DirInfo{ HashToIndex::from_hash40(Hash40::from_str("fighter/pickel/c08/camera"), camera_idx), Hash40::from_str("camera"), Hash40::from_str("fighter/pickel/c08/"), 0, 0, 0, 0, 0, 0, old_dirinfo.flags };

}

void test_cases(Arc& arc) {
    auto& among_us = arc.file_lookup_by_hash(Hash40::from_str("render/shader/posteffecteffectlibrary.nufxlb"));
    auto& fileinfo = arc.file_lookup_by_hash(Hash40::from_str("fighter/pickel/model/body/c00/def_pickel_001_col.nutexb"));
    
    auto filedata = arc.fileinfo_to_filedata(fileinfo);
    
    arc.file_lookup_by_hash(Hash40::from_str("fighter/pickel/model/body/c08/def_pickel_001_col.nutexb"));
    arc.get_dir_info_from_path_hash(Hash40::from_str("fighter/pickel/c08"));
    arc.get_dir_info_from_path_hash(Hash40::from_str("fighter/pickel/c08/cmn"));
}

int main() 
{
    auto start = std::chrono::high_resolution_clock::now();

    Arc arc("D:\\AppData (yuzu)\\Local\\yuzu\\yuzu-windows-msvc\\user\\dump\\Super Smash Bros. Ultimate ARCs\\data_1301.arc");
    // Arc arc("D:\\AppData (yuzu)\\Local\\yuzu\\yuzu-windows-msvc\\user\\sdmc\\atmosphere\\contents\\01006A800016E000\\romfs\\data.arc");
    // Arc arc("D:\\Emulators\\ryujinx-1.0.5553-win_x64\\user\\mods\\contents\\01006a800016e000\\romfs\\data.arc");

    //arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/param/database/ui_chara_db.prc"))).decomp_size = 35449;
    //arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/param/database/ui_chara_db.prc"))).comp_size = 7249;

    FileData& ref = arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_00.bntx")));

    FileData cpy = arc.fileinfo_to_filedata(arc.file_lookup_by_hash(Hash40::from_str("ui/replace_patch/chara/chara_1/chara_1_pickel_07.bntx")));

    ref = cpy;

    directory_override(arc);

    //Hashes::init();

    //test_cases(arc);

    //add_steve_ui_c08(arc);

    //new_test(arc);

    //arc.REBUILD_FILEINFOBUCKETS();

    //arc.REBUILD_DIR_HASH_TO_INFO_INDEX();
    
    //arc.file_system.write("output_fs_edited.tbl");
    //arc.file_system.write_compressed("output_fs_compressed.tbl");

    
    
    // arc.file_system.write_directly_to_arc("D:\\Emulators\\ryujinx-1.0.5553-win_x64\\user\\mods\\contents\\01006a800016e000\\romfs\\data.arc");
    // arc.file_system.write_directly_to_arc("D:\\AppData (yuzu)\\Local\\yuzu\\yuzu-windows-msvc\\user\\sdmc\\atmosphere\\contents\\01006A800016E000\\romfs\\data.arc");

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count()/1000 << "ms" << std::endl;
}
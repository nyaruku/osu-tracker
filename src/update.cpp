#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <miniz.h>
// Extracts update.zip in current working directory
bool extract_zip(const std::string& zip_path) {
    mz_zip_archive zip = {};
    memset(&zip, 0, sizeof(zip));
    // Init zip reader
    if (!mz_zip_reader_init_file(&zip, zip_path.c_str(), 0)) {
        std::cerr << "Failed to open ZIP archive: " << zip_path << "\n";
        return false;
    }
    int file_count = (int)mz_zip_reader_get_num_files(&zip);
    std::cout << "Extracting " << file_count << " files from " << zip_path << "\n";
    for (int i = 0; i < file_count; ++i) {
        mz_zip_archive_file_stat file_stat;
        if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
            std::cerr << "Failed to get file info for index " << i << "\n";
            mz_zip_reader_end(&zip);
            return false;
        }

        std::string filename = file_stat.m_filename;

        // Skip entries that start with "_" in any path segment
        bool skip = false;
        size_t pos = 0;
        while (pos < filename.length()) {
            size_t next = filename.find_first_of("/\\", pos);
            std::string segment = filename.substr(pos, next - pos);
            if (!segment.empty() && segment[0] == '_') {
                skip = true;
                break;
            }
            if (next == std::string::npos) break;
            pos = next + 1;
        }

        if (skip) {
            continue;
        }

        // Skip directories
        if (mz_zip_reader_is_file_a_directory(&zip, i)) {
            continue;
        }

        // Extract to disk (overwrite mode)
        if (!mz_zip_reader_extract_to_file(&zip, i, filename.c_str(), 0)) {
            std::cerr << "Failed to extract: " << filename << "\n";
            mz_zip_reader_end(&zip);
            return false;
        }
        std::cout << "Extracted: " << filename << "\n";
    }
    mz_zip_reader_end(&zip);
    return true;
}
int main() {
    const std::string zip_file = "update.zip";
    if (!extract_zip(zip_file)) {
        std::cerr << "Extraction failed.\n";
        return 1;
    }
    std::cout << "Extraction complete.\n";
    #if defined(_WIN32)
        system("start osu-tracker.exe");
    #elif defined(__linux__)
        system("./osu-tracker &");
    #endif
    return 0;
}
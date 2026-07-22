#include "disk_image.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

extern "C" {
#include <cocopath.h>
}

namespace toolshed {
namespace {

std::vector<char> mutable_path(const std::string& path)
{
    std::vector<char> value(path.begin(), path.end());
    value.push_back('\0');
    return value;
}

std::runtime_error image_error(const std::string& operation, error_code code)
{
    return std::runtime_error(operation + " failed with ToolShed error " +
                              std::to_string(code));
}

std::string normalize_image_path(std::string path)
{
    while (!path.empty() && path.front() == '/') {
        path.erase(path.begin());
    }
    while (!path.empty() && path.back() == '/') {
        path.pop_back();
    }
    std::size_t start = 0;
    while (start <= path.size()) {
        const auto end = path.find('/', start);
        const auto part = path.substr(start, end == std::string::npos
            ? std::string::npos : end - start);
        if (part == ".." || part == ".") {
            throw std::runtime_error("Relative traversal is not a valid image path");
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return path;
}

std::string join_image_path(const std::string& directory, const std::string& name)
{
    return directory.empty() ? name : directory + "/" + name;
}

std::string attributes_text(int attributes)
{
    struct AttributeName {
        int flag;
        const char* name;
    };
    static constexpr std::array<AttributeName, 7> names = {{
        {FAP_DIR, "d"}, {FAP_SINGLE, "s"}, {FAP_READ, "r"}, {FAP_WRITE, "w"},
        {FAP_EXEC, "e"}, {FAP_PREAD, "pr"}, {FAP_PWRITE, "pw"}
    }};

    std::string result;
    for (const auto& item : names) {
        if ((attributes & item.flag) != 0) {
            if (!result.empty()) {
                result += ' ';
            }
            result += item.name;
        }
    }
    if ((attributes & FAP_PEXEC) != 0) {
        result += result.empty() ? "pe" : " pe";
    }
    return result;
}

class CocoPath {
public:
    CocoPath(const std::string& path, int mode)
    {
        auto value = mutable_path(path);
        const error_code error = _coco_open(&path_, value.data(), mode);
        if (error != 0) {
            throw image_error("Open", error);
        }
    }

    ~CocoPath()
    {
        if (path_ != nullptr) {
            _coco_close(path_);
        }
    }

    CocoPath(const CocoPath&) = delete;
    CocoPath& operator=(const CocoPath&) = delete;

    [[nodiscard]] coco_path_id get() const noexcept { return path_; }

private:
    coco_path_id path_ = nullptr;
};

std::string type_name(const coco_dir_entry& entry)
{
    if (entry.type == OS9) {
        return "OS-9 file";
    }

    if (entry.type == DECB) {
        static constexpr std::array<const char*, 4> names = {
            "BASIC program", "BASIC data", "Machine language", "Text"
        };
        const auto file_type = entry.dentry.decb.file_type;
        return file_type < names.size() ? names[file_type] : "Unknown";
    }

    return "File";
}

std::string lowercase_extension(const std::filesystem::path& path)
{
    auto extension = path.extension().string();
    if (!extension.empty() && extension.front() == '.') {
        extension.erase(extension.begin());
    }
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    return extension;
}

}  // namespace

DiskImage::DiskImage(std::filesystem::path path, ImageFormat format)
    : path_(std::move(path)), format_(format)
{
}

DiskImage DiskImage::open(const std::filesystem::path& path)
{
    if (!std::filesystem::is_regular_file(path)) {
        throw std::runtime_error("The selected image does not exist or is not a regular file");
    }

    const std::string root = path.string() + ",";
    auto value = mutable_path(root);
    _path_type type = NATIVE;
    const error_code error = _coco_identify_image(value.data(), &type);
    if (error != 0) {
        throw image_error("Identify image", error);
    }

    switch (type) {
    case OS9:
        return DiskImage(path, ImageFormat::os9);
    case DECB:
        return DiskImage(path, ImageFormat::disk_basic);
    default:
        throw std::runtime_error("The selected file is not an OS-9 RBF or Disk BASIC image");
    }
}

std::string DiskImage::format_name() const
{
    return format_ == ImageFormat::os9 ? "OS-9 RBF" : "Disk BASIC";
}

std::string DiskImage::toolshed_path(const std::string& image_path) const
{
    return path_.string() + "," + normalize_image_path(image_path);
}

std::vector<Entry> DiskImage::list_directory(const std::string& requested_directory) const
{
    const std::string directory_name = normalize_image_path(requested_directory);
    if (format_ == ImageFormat::disk_basic && !directory_name.empty()) {
        throw std::runtime_error("Disk BASIC images do not contain subdirectories");
    }

    const std::string root = path_.string() + ",";
    const std::string directory_path = root + directory_name;
    const int mode = format_ == ImageFormat::os9 ? FAM_READ | FAM_DIR : FAM_READ;
    CocoPath directory(directory_path, mode);
    std::vector<Entry> entries;

    for (;;) {
        coco_dir_entry native_entry{};
        const error_code error = _coco_readdir(directory.get(), &native_entry);
        if (error != 0) {
            break;
        }

        std::array<u_char, 256> name_buffer{};
        if (_coco_ncpy_name(&native_entry, name_buffer.data(), name_buffer.size()) != 0) {
            continue;
        }

        const std::string name(reinterpret_cast<char*>(name_buffer.data()));
        if (name.empty() || static_cast<unsigned char>(name.front()) == 0xff ||
            name == "." || name == "..") {
            continue;
        }

        Entry entry;
        entry.name = name;
        entry.image_path = join_image_path(directory_name, name);
        entry.type = type_name(native_entry);
        if (format_ == ImageFormat::disk_basic) {
            entry.encoding = native_entry.dentry.decb.ascii_flag == 0xff
                ? "ASCII" : "Binary";
        }

        const std::string child = root + entry.image_path;
        coco_file_stat metadata{};
        if (format_ == ImageFormat::os9) {
            try {
                CocoPath child_directory(child, FAM_READ | FAM_DIR);
                entry.directory = true;
                entry.type = "Folder";
                _coco_gs_fd(child_directory.get(), &metadata);
            } catch (const std::runtime_error&) {
                // It is an ordinary file.
            }
        }

        if (!entry.directory) {
            try {
                CocoPath file(child, FAM_READ);
                u_int size = 0;
                if (_coco_gs_size(file.get(), &size) == 0) {
                    entry.size = size;
                }
                if (_coco_gs_fd(file.get(), &metadata) == 0) {
                    entry.attributes = attributes_text(metadata.attributes);
                    entry.user_id = metadata.user_id;
                    entry.group_id = metadata.group_id;
                    entry.modified_time = format_ == ImageFormat::os9
                        ? static_cast<std::int64_t>(metadata.last_modified_time) : 0;
                }
            } catch (const std::runtime_error&) {
                // Keep the directory entry visible even if its metadata is damaged.
            }
        } else {
            entry.attributes = attributes_text(metadata.attributes);
            entry.user_id = metadata.user_id;
            entry.group_id = metadata.group_id;
            entry.modified_time = static_cast<std::int64_t>(metadata.last_modified_time);
        }

        entries.push_back(std::move(entry));
    }

    return entries;
}

std::vector<std::uint8_t> DiskImage::read_file(const std::string& requested_path) const
{
    const std::string image_path = normalize_image_path(requested_path);
    if (image_path.empty()) {
        throw std::runtime_error("A file must be selected");
    }

    CocoPath file(path_.string() + "," + image_path, FAM_READ);
    u_int size = 0;
    const error_code size_error = _coco_gs_size(file.get(), &size);
    if (size_error != 0) {
        throw image_error("Read file size", size_error);
    }

    std::vector<std::uint8_t> bytes(size);
    std::size_t offset = 0;
    while (offset < bytes.size()) {
        const auto remaining = bytes.size() - offset;
        u_int chunk = static_cast<u_int>(
            std::min<std::size_t>(remaining, std::numeric_limits<u_int>::max()));
        const error_code read_error = _coco_read(file.get(), bytes.data() + offset, &chunk);
        if (read_error != 0 || chunk == 0) {
            throw image_error("Read file", read_error != 0 ? read_error : EOS_EOF);
        }
        offset += chunk;
    }
    return bytes;
}

void DiskImage::ensure_backup()
{
    if (backup_path_) {
        return;
    }

    auto candidate = std::filesystem::path(path_.string() + ".diskshed-backup");
    for (unsigned int suffix = 1; std::filesystem::exists(candidate); ++suffix) {
        candidate = std::filesystem::path(path_.string() + ".diskshed-backup-" +
                                          std::to_string(suffix));
    }

    std::error_code copy_error;
    if (!std::filesystem::copy_file(path_, candidate,
                                    std::filesystem::copy_options::none, copy_error)) {
        throw std::runtime_error("Could not create image backup: " + copy_error.message());
    }
    backup_path_ = std::move(candidate);
}

void DiskImage::import_file(const std::string& requested_directory,
                            const std::filesystem::path& host_file, bool replace)
{
    if (!std::filesystem::is_regular_file(host_file)) {
        throw std::runtime_error("Only regular files can be imported");
    }

    const auto host_size = std::filesystem::file_size(host_file);
    if (host_size > std::numeric_limits<u_int>::max()) {
        throw std::runtime_error("The host file is too large for ToolShed");
    }

    std::ifstream input(host_file, std::ios::binary);
    std::vector<std::uint8_t> bytes(static_cast<std::size_t>(host_size));
    input.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!input && !bytes.empty()) {
        throw std::runtime_error("Could not read the host file");
    }

    const std::string directory = normalize_image_path(requested_directory);
    const std::string image_path = join_image_path(directory, host_file.filename().string());
    ensure_backup();

    coco_file_stat metadata{};
    metadata.perms = FAP_READ | FAP_WRITE | FAP_PREAD;
    if (format_ == ImageFormat::disk_basic) {
        const std::string extension = lowercase_extension(host_file);
        metadata.file_type = extension == "bas" ? 0 : extension == "dat" ? 1
            : extension == "txt" || extension == "asc" ? 3 : 2;
        metadata.data_type = extension == "bas" || extension == "dat" ||
            extension == "txt" || extension == "asc" ? 0xff : 0;
    }

    auto path_value = mutable_path(toolshed_path(image_path));
    coco_path_id destination = nullptr;
    const int mode = FAM_WRITE | (replace ? 0 : FAM_NOCREATE);
    error_code error = _coco_create(&destination, path_value.data(), mode, &metadata);
    if (error != 0) {
        throw image_error("Create imported file", error);
    }

    std::size_t offset = 0;
    while (offset < bytes.size()) {
        u_int chunk = static_cast<u_int>(std::min<std::size_t>(
            bytes.size() - offset, std::numeric_limits<u_int>::max()));
        error = _coco_write(destination, bytes.data() + offset, &chunk);
        if (error != 0 || chunk == 0) {
            _coco_close(destination);
            throw image_error("Write imported file", error != 0 ? error : EOS_WRITE);
        }
        offset += chunk;
    }
    error = _coco_close(destination);
    if (error != 0) {
        throw image_error("Close imported file", error);
    }
}

void DiskImage::make_directory(const std::string& requested_directory, const std::string& name)
{
    if (format_ != ImageFormat::os9) {
        throw std::runtime_error("Disk BASIC images do not support directories");
    }
    if (name.empty() || name.find('/') != std::string::npos) {
        throw std::runtime_error("Enter a single directory name");
    }

    const std::string image_path = join_image_path(
        normalize_image_path(requested_directory), normalize_image_path(name));
    ensure_backup();
    auto value = mutable_path(toolshed_path(image_path));
    const error_code error = _coco_makdir(value.data());
    if (error != 0) {
        throw image_error("Create directory", error);
    }
}

void DiskImage::rename_entry(const std::string& requested_path, const std::string& new_name)
{
    if (new_name.empty() || new_name.find('/') != std::string::npos) {
        throw std::runtime_error("Enter a single new name");
    }

    ensure_backup();
    auto path_value = mutable_path(toolshed_path(requested_path));
    auto name_value = mutable_path(new_name);
    const error_code error = _coco_rename(path_value.data(), name_value.data());
    if (error != 0) {
        throw image_error("Rename entry", error);
    }
}

void DiskImage::delete_entry(const std::string& requested_path, bool directory)
{
    ensure_backup();
    auto value = mutable_path(toolshed_path(requested_path));
    const error_code error = directory ? _coco_delete_directory(value.data())
                                       : _coco_delete(value.data());
    if (error != 0) {
        throw image_error("Delete entry", error);
    }
}

}  // namespace toolshed

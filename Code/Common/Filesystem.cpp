// Author: Jake Rieger
// Created: 12/12/24.
//

#include "Filesystem.hpp"
#include <sstream>

#ifdef _WIN32
    // Windows does not define the S_ISREG and S_ISDIR macros in stat.h, so we do.
    // We have to define _CRT_INTERNAL_NONSTDC_NAMES 1 before #including sys/stat.h
    // in order for Microsoft's stat.h to define names like S_IFMT, S_IFREG, and S_IFDIR,
    // rather than just defining  _S_IFMT, _S_IFREG, and _S_IFDIR as it normally does.
    #define _CRT_INTERNAL_NONSTDC_NAMES 1
    #include <Windows.h>
    #include <sys/stat.h>
    #if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
        #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
    #endif
    #if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
        #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
    #endif
#else
    #include <sys/stat.h>
#endif

namespace x {
#pragma region FileReader
    std::vector<u8> FileReader::ReadBytes(const Path& path) {
        std::ifstream file(path.Str(), std::ios::binary | std::ios::ate);
        if (!file.is_open()) { return {}; }
        const std::streamsize fileSize = file.tellg();
        std::vector<u8> bytes(fileSize);
        file.seekg(0, std::ios::beg);
        if (!file.read(reinterpret_cast<char*>(bytes.data()), fileSize)) { return {}; }
        file.close();
        return bytes;
    }

    str FileReader::ReadText(const Path& path) {
        const std::ifstream file(path.Str());
        if (!file.is_open()) { return {}; }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::vector<str> FileReader::ReadLines(const Path& path) {
        std::ifstream file(path.Str());
        std::vector<str> lines;
        if (!file.is_open()) { return {}; }
        str line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    std::vector<u8> FileReader::ReadBlock(const Path& path, size_t size, u64 offset) {
        std::ifstream file(path.Str(), std::ios::binary | std::ios::ate);
        if (!file) { return {}; }
        const std::streamsize fileSize = file.tellg();
        if (offset >= (u64)fileSize || size == 0 || offset + size > (u64)fileSize) { return {}; }
        file.seekg((std::streamsize)offset, std::ios::beg);
        if (!file) { return {}; }
        std::vector<u8> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), (std::streamsize)size);
        if (!file) { return {}; }
        return buffer;
    }

    size_t FileReader::QueryFileSize(const Path& path) {
        std::ifstream file(path.Str(), std::ios::binary | std::ios::ate);
        if (!file.is_open()) { return 0; }
        const std::streamsize fileSize = file.tellg();
        return fileSize;
    }
#pragma endregion

#pragma region FileWriter
    bool FileWriter::WriteBytes(const Path& path, const std::vector<u8>& data) {
        std::ofstream file(path.Str(), std::ios::binary | std::ios::trunc);
        // Overwrite existing file
        if (!file) return false;
        file.write(RCAST<const char*>(data.data()), CAST<std::streamsize>(data.size()));
        return file.good();
    }

    bool FileWriter::WriteText(const Path& path, const str& text) {
        std::ofstream file(path.Str(), std::ios::out | std::ios::trunc);
        if (!file) return false;
        str outText = text;
        if (outText.empty()) { return false; }
        // Enforce newline for final text value
        if (outText.back() != '\n') { outText += '\n'; }
        file << outText;
        return file.good();
    }

    bool FileWriter::WriteLines(const Path& path, const std::vector<str>& lines) {
        std::ofstream file(path.Str(), std::ios::out | std::ios::trunc);
        if (!file) return false;
        for (const auto& line : lines) {
            file << line << '\n';
            if (!file.good()) { return false; }
        }
        return file.good();
    }

    bool FileWriter::WriteBlock(const Path& path, const std::span<const u8>& data, u64 offset) {
        std::ofstream file(path.Str(),
                           std::ios::binary | std::ios::in | std::ios::out);  // Open in binary read/write mode
        if (!file) return false;
        file.seekp(CAST<std::streampos>((std::streamoff)offset), std::ios::beg);
        // seek to offset
        if (!file) return false;  // Failed to seek
        file.write(RCAST<const char*>(data.data()), CAST<std::streamsize>(data.size()));
        return file.good();
    }

    std::future<std::vector<u8>> AsyncFileReader::ReadBytes(const Path& path) {
        return RunAsync([path]() { return FileReader::ReadBytes(path); });
    }

    std::future<str> AsyncFileReader::ReadText(const Path& path) {
        return RunAsync([path]() { return FileReader::ReadText(path); });
    }

    std::future<std::vector<str>> AsyncFileReader::ReadLines(const Path& path) {
        return RunAsync([path]() { return FileReader::ReadLines(path); });
    }

    std::future<std::vector<u8>> AsyncFileReader::ReadBlock(const Path& path, size_t size, u64 offset) {
        return RunAsync([path, size, offset]() { return FileReader::ReadBlock(path, size, offset); });
    }

    std::future<bool> AsyncFileWriter::WriteBytes(const Path& path, const std::vector<u8>& data) {
        return RunAsync([path, data]() { return FileWriter::WriteBytes(path, data); });
    }

    std::future<bool> AsyncFileWriter::WriteText(const Path& path, const str& text) {
        return RunAsync([path, text]() { return FileWriter::WriteText(path, text); });
    }

    std::future<bool> AsyncFileWriter::WriteLines(const Path& path, const std::vector<str>& lines) {
        return RunAsync([path, lines]() { return FileWriter::WriteLines(path, lines); });
    }

    std::future<bool> AsyncFileWriter::WriteBlock(const Path& path, const std::span<const u8>& data, u64 offset) {
        return RunAsync([path, data, offset]() { return FileWriter::WriteBlock(path, data, offset); });
    }
#pragma endregion

#pragma region Stream IO
    StreamReader::StreamReader(const Path& path) : mStream(path.Str(), std::ios::binary | std::ios::ate) {
        if (mStream.is_open()) {
            mSize = CAST<u64>(mStream.tellg());
            mStream.seekg(0, std::ios::beg);
        } else {
            mSize = 0;
        }
    }

    StreamReader::~StreamReader() {
        Close();
    }

    StreamReader::StreamReader(StreamReader&& other) noexcept : mStream(std::move(other.mStream)), mSize(other.mSize) {
        other.mSize = 0;
    }

    StreamReader& StreamReader::operator=(StreamReader&& other) noexcept {
        if (this != &other) {
            Close();
            mStream     = std::move(other.mStream);
            mSize       = other.mSize;
            other.mSize = 0;
        }
        return *this;
    }

    bool StreamReader::Read(std::vector<u8>& data, size_t size) {
        if (!IsOpen() || size == 0) return false;
        const auto currentPos = Position();
        if (currentPos + size > mSize) { size = CAST<size_t>(mSize - currentPos); }
        data.resize(size);
        mStream.read(RCAST<char*>(data.data()), (std::streamsize)size);
        return mStream.good();
    }

    bool StreamReader::ReadAll(std::vector<u8>& data) {
        if (!IsOpen()) return false;

        const auto size = Size();
        if (size == 0) {
            data.clear();
            return true;
        }

        Seek(0);
        data.resize(CAST<size_t>(size));
        mStream.read(RCAST<char*>(data.data()), (std::streamsize)size);
        return mStream.good();
    }

    bool StreamReader::ReadLine(str& line) {
        if (!IsOpen()) return false;
        return CAST<bool>(std::getline(mStream, line));
    }

    bool StreamReader::IsOpen() const {
        return mStream.is_open() && mStream.good();
    }

    bool StreamReader::Seek(u64 offset) {
        if (!IsOpen()) return false;
        mStream.seekg((std::streamoff)offset);
        return mStream.good();
    }

    u64 StreamReader::Position() {
        if (!IsOpen()) return 0;
        return CAST<u64>(mStream.tellg());
    }

    size_t StreamReader::Size() const {
        return mSize;
    }

    void StreamReader::Close() {
        if (mStream.is_open()) { mStream.close(); }
    }

    StreamWriter::StreamWriter(const Path& path, bool append)
        : mStream(path.Str(), std::ios::binary | (append ? std::ios::app : std::ios::trunc)) {}

    StreamWriter::~StreamWriter() {
        Close();
    }

    StreamWriter::StreamWriter(StreamWriter&& other) noexcept : mStream(std::move(other.mStream)) {}

    StreamWriter& StreamWriter::operator=(StreamWriter&& other) noexcept {
        if (this != &other) {
            Close();
            mStream = std::move(other.mStream);
        }
        return *this;
    }

    bool StreamWriter::Write(const std::vector<u8>& buffer) {
        return Write(buffer, buffer.size());
    }

    bool StreamWriter::Write(const std::vector<u8>& buffer, size_t size) {
        if (!IsOpen() || size == 0) return false;
        if (size > buffer.size()) size = buffer.size();
        mStream.write(RCAST<cstr>(buffer.data()), (std::streamsize)size);
        return mStream.good();
    }

    bool StreamWriter::WriteLine(const str& line) {
        if (!IsOpen()) return false;
        mStream << line << '\n';
        return mStream.good();
    }

    bool StreamWriter::Flush() {
        if (!IsOpen()) return false;
        mStream.flush();
        return mStream.good();
    }

    bool StreamWriter::IsOpen() const {
        return mStream.is_open() && mStream.good();
    }

    bool StreamWriter::Seek(u64 offset) {
        if (!IsOpen()) return false;
        mStream.seekp((std::streamoff)offset);
        return mStream.good();
    }

    u64 StreamWriter::Position() {
        if (!IsOpen()) return 0;
        return CAST<u64>(mStream.tellp());
    }

    void StreamWriter::Close() {
        if (mStream.is_open()) {
            mStream.flush();
            mStream.close();
        }
    }
#pragma endregion

#pragma region Path
    Path Path::Current() {
        char buffer[MAX_PATH];
        ::GetModuleFileNameA(nullptr, buffer, MAX_PATH);
        const str::size_type pos = str(buffer).find_last_of("\\/");
        return Path(str(buffer).substr(0, pos));
    }

    Path Path::Parent() const {
        const size_t lastSeparator = mPath.find_last_of(PATH_SEPARATOR);
        if (lastSeparator == std::string::npos || lastSeparator == 0) { return Path(std::to_string(PATH_SEPARATOR)); }
        return Path(mPath.substr(0, lastSeparator));
    }

    bool Path::Exists() const {
        struct stat info {};
        return stat(mPath.c_str(), &info) == 0;
    }

    bool Path::IsFile() const {
        struct stat info {};
        if (stat(mPath.c_str(), &info) != 0) {
            std::perror(mPath.c_str());
            return false;
        }
        return S_ISREG(info.st_mode);
    }

    bool Path::IsDirectory() const {
        struct stat info {};
        if (stat(mPath.c_str(), &info) != 0) {
            std::perror(mPath.c_str());
            return false;
        }
        return S_ISDIR(info.st_mode);
    }

    bool Path::HasExtension() const {
        const size_t pos = mPath.find_last_of('.');
        const size_t sep = mPath.find_last_of(PATH_SEPARATOR);
        return pos != str::npos && (sep == str::npos || pos > sep);
    }

    str Path::Extension() const {
        if (!HasExtension()) { return ""; }
        return mPath.substr(mPath.find_last_of('.') + 1);
    }

    Path Path::ReplaceExtension(const str& ext) const {
        if (!HasExtension()) return Path(mPath + "." + ext);
        return Path(mPath.substr(0, mPath.find_last_of('.')) + "." + ext);
    }

    Path Path::Join(const str& subPath) const {
        return Path(Join(mPath, subPath));
    }

    Path Path::operator/(const str& subPath) const {
        return Path(Join(mPath, subPath));
    }

    str Path::Str() const {
        return mPath;
    }

    const char* Path::CStr() const {
        return mPath.c_str();
    }

    str Path::Filename() const {
        size_t pos = mPath.rfind('\\');
        if (pos != str::npos) { return mPath.substr(pos + 1); }
        pos = mPath.rfind('/');
        if (pos != str::npos) { return mPath.substr(pos + 1); }
        return mPath;
    }

    Path Path::RelativeTo(const Path& basePath) const {
        const str thisStr = Str();
        str baseStr       = basePath.Str();

        if (!baseStr.empty() && baseStr.back() == PATH_SEPARATOR) { baseStr += PATH_SEPARATOR; }
        if (thisStr.substr(0, baseStr.length()) != baseStr) { return *this; }

        const str relativePath = thisStr.substr(baseStr.length());

        if (relativePath.empty()) { return Path("."); }

        return Path(relativePath);
    }

    str Path::BaseName() const {
        const str filename = Filename();
        return filename.substr(0, filename.find_last_of('.'));  // remove ext
    }

    Path& Path::Join(const str& subPath) {
        mPath = Join(Str(), subPath);
        return *this;
    }

    bool Path::operator==(const Path& other) const {
        return mPath == other.mPath;
    }

    bool Path::Create() const {
        if (Exists()) return true;

#ifdef _WIN32
        if (!CreateDirectoryA(mPath.c_str(), nullptr)) {
            const DWORD error = GetLastError();
            if (error != ERROR_ALREADY_EXISTS) { return false; }
        }
#else
        if (mkdir(path.c_str(), 0755) != 0) {
            if (errno != EEXIST) { return false; }
        }
#endif
        return true;
    }

    bool Path::CreateAll() const {
        if (Exists()) return true;

        if (mPath != str(1, PATH_SEPARATOR)) {
            Path parentPath = Parent();
            if (!parentPath.Exists()) {
                if (!parentPath.CreateAll()) return false;
            }
        }

        return Create();
    }

    bool Path::Copy(const Path& dest) const {
        X_ASSERT(IsFile());
        if (dest == *this) { return true; }
        if (!::CopyFileA(mPath.c_str(), dest.mPath.c_str(), FALSE)) { return false; }
        return true;
    }

    bool Path::CopyDirectory(const Path& dest) const {
        X_ASSERT(IsDirectory());

        const DWORD srcAttrs = ::GetFileAttributesA(mPath.c_str());
        if (srcAttrs == INVALID_FILE_ATTRIBUTES) { return false; }
        if (!(srcAttrs & FILE_ATTRIBUTE_DIRECTORY)) { return false; }

        if (!::CreateDirectoryA(dest.CStr(), nullptr) && ::GetLastError() != ERROR_ALREADY_EXISTS) { return false; }

        const str searchPattern = mPath + "\\*";
        WIN32_FIND_DATAA findData;
        const HANDLE hFind = ::FindFirstFileA(searchPattern.c_str(), &findData);
        if (hFind == INVALID_HANDLE_VALUE) { return false; }

        bool success {true};
        do {
            if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) { continue; }

            Path srcPath  = Join(findData.cFileName);
            Path destPath = dest / findData.cFileName;

            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                if (!srcPath.CopyDirectory(destPath)) { success = false; }
            } else {
                if (!srcPath.Copy(destPath)) { success = false; }
            }
        } while (::FindNextFileA(hFind, &findData));

        const DWORD lastError = ::GetLastError();
        ::FindClose(hFind);

        if (lastError != ERROR_NO_MORE_FILES) { success = false; }

        return success;
    }

    DirectoryEntries Path::Entries() const {
        return DirectoryEntries(*this);
    }

    str Path::Join(const str& lhs, const str& rhs) {
        if (lhs.empty()) { return lhs; }
        if (rhs.empty()) { return rhs; }
        if (lhs.back() == PATH_SEPARATOR) return lhs + rhs;
        return lhs + PATH_SEPARATOR + rhs;
    }

    str Path::Normalize(const str& rawPath) {
        str result;
        std::vector<str> parts;
        size_t start = 0;
        while (start < rawPath.size()) {
            size_t end = rawPath.find(PATH_SEPARATOR, start);
            if (end == std::string::npos) { end = rawPath.size(); }
            str part = rawPath.substr(start, end - start);
            if (part == ".." && !parts.empty() && parts.back() != "..") {
                parts.pop_back();
            } else if (!part.empty() && part != ".") {
                parts.push_back(part);
            }
            start = end + 1;
        }
        for (const auto& part : parts) {
            result += PATH_SEPARATOR + part;
        }

#ifdef _WIN32
        // Remove the first '/' if Windows path
        result = result.substr(1, result.size() - 1);
#endif

        return result.empty() ? str(1, PATH_SEPARATOR) : result;
    }

    FindHandleWrapper::FindHandleWrapper() : mHandle(INVALID_HANDLE_VALUE) {}

    FindHandleWrapper::FindHandleWrapper(HANDLE handle) : mHandle(handle) {}

    FindHandleWrapper::~FindHandleWrapper() {
        if (mHandle != INVALID_HANDLE_VALUE) { ::FindClose(mHandle); }
    }

    FindHandleWrapper::FindHandleWrapper(FindHandleWrapper&& other) noexcept : mHandle(other.mHandle) {
        other.mHandle = INVALID_HANDLE_VALUE;
    }

    FindHandleWrapper& FindHandleWrapper::operator=(FindHandleWrapper&& other) noexcept {
        if (this != &other) {
            if (mHandle != INVALID_HANDLE_VALUE) { ::FindClose(mHandle); }
            mHandle       = other.mHandle;
            other.mHandle = INVALID_HANDLE_VALUE;
        }
        return *this;
    }

    HANDLE FindHandleWrapper::Get() const {
        return mHandle;
    }

    bool FindHandleWrapper::IsValid() const {
        return mHandle != INVALID_HANDLE_VALUE;
    }

    DirectoryEntries::DirectoryEntries(const Path& path) : mPath(path) {}

    DirectoryIterator::DirectoryIterator() : mIsEnd(true) {}

    DirectoryIterator::DirectoryIterator(const Path& path) : mIsEnd(false), mRoot(path) {
        if (!mRoot.IsDirectory()) {
            mIsEnd = true;
            return;
        }

        str searchPattern = mRoot.Str() + "\\*";
        WIN32_FIND_DATA findData;
        mFindHandle = FindHandleWrapper(::FindFirstFileA(searchPattern.c_str(), &findData));

        if (!mFindHandle.IsValid()) {
            mIsEnd = true;
            return;
        }

        ProcessCurrentEntry(findData);
    }

    DirectoryIterator::reference DirectoryIterator::operator*() const {
        return mCurrent;
    }

    DirectoryIterator::pointer DirectoryIterator::operator->() const {
        return &mCurrent;
    }

    DirectoryIterator& DirectoryIterator::operator++() {
        if (mIsEnd) { return *this; }

        WIN32_FIND_DATAA findData;
        if (::FindNextFileA(mFindHandle.Get(), &findData) == 0) {
            mIsEnd = true;
            return *this;
        }

        ProcessCurrentEntry(findData);
        return *this;
    }

    bool DirectoryIterator::operator==(const DirectoryIterator& other) const {
        if (mIsEnd && other.mIsEnd) { return true; }
        if (mIsEnd || other.mIsEnd) { return false; }
        return mCurrent == other.mCurrent;
    }

    bool DirectoryIterator::operator!=(const DirectoryIterator& other) const {
        return !(*this == other);
    }

    void DirectoryIterator::ProcessCurrentEntry(const WIN32_FIND_DATAA& findData) {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            this->operator++();
            return;
        }

        mCurrent = mRoot / findData.cFileName;
    }

    DirectoryIterator DirectoryEntries::begin() {
        return DirectoryIterator(mPath);
    }

    DirectoryIterator DirectoryEntries::end() {
        return DirectoryIterator();
    }

    std::ostream& operator<<(std::ostream& os, const Path& path) {
        os << path.Str();
        return os;
    }
#pragma endregion
}  // namespace x
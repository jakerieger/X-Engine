// Author: Jake Rieger
// Created: 12/12/24.
//

#pragma once

#include "Types.hpp"
#include <fstream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <future>
#include <stack>

#ifdef _WIN32
    #include <Windows.h>
    #include <direct.h>
    #define getcwd _getcwd
    #define PATH_SEPARATOR '\\'
#else
    #include <unistd.h>
    #define PATH_SEPARATOR '/'
#endif

namespace x {
    namespace Filesystem {
        class Path;

        class FileReader {
        public:
            static std::vector<u8> ReadAllBytes(const Path& path);
            static str ReadAllText(const Path& path);
            static std::vector<str> ReadAllLines(const Path& path);
            static std::vector<u8> ReadBlock(const Path& path, size_t size, u64 offset = 0);
            static size_t QueryFileSize(const Path& path);
        };

        class FileWriter {
        public:
            static bool WriteAllBytes(const Path& path, const std::vector<u8>& data);
            static bool WriteAllText(const Path& path, const str& text);
            static bool WriteAllLines(const Path& path, const std::vector<str>& lines);
            static bool WriteBlock(const Path& path, const std::vector<u8>& data, u64 offset = 0);
        };

        class AsyncFileReader {
        public:
            static std::future<std::vector<u8>> ReadAllBytes(const Path& path);
            static std::future<str> ReadAllText(const Path& path);
            static std::future<std::vector<str>> ReadAllLines(const Path& path);
            static std::future<std::vector<u8>> ReadBlock(const Path& path, size_t size, u64 offset = 0);

        private:
            template<typename Func>
            static auto runAsync(Func&& func) -> std::future<decltype(func())> {
                using ReturnType = decltype(func());
                auto task        = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(func));
                std::future<ReturnType> future = task->get_future();
                std::thread([task]() { (*task)(); }).detach();
                return future;
            }
        };

        class AsyncFileWriter {
        public:
            static std::future<bool> WriteAllBytes(const Path& path, const std::vector<u8>& data);
            static std::future<bool> WriteAllText(const Path& path, const str& text);
            static std::future<bool> WriteAllLines(const Path& path, const std::vector<str>& lines);
            static std::future<bool> WriteBlock(const Path& path, const std::vector<u8>& data, u64 offset = 0);

        private:
            template<typename Func>
            static auto runAsync(Func&& func) -> std::future<decltype(func())> {
                using ReturnType = decltype(func());
                auto task        = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(func));
                std::future<ReturnType> future = task->get_future();
                std::thread([task]() { (*task)(); }).detach();
                return future;
            }
        };

        class StreamReader {
        public:
            explicit StreamReader(const Path& path);
            ~StreamReader();

            StreamReader(const StreamReader&)            = delete;
            StreamReader& operator=(const StreamReader&) = delete;

            StreamReader(StreamReader&&) noexcept;
            StreamReader& operator=(StreamReader&&) noexcept;

            bool Read(vector<u8>& data, size_t size);
            bool ReadAll(vector<u8>& data);
            bool ReadLine(str& line);

            bool IsOpen() const;
            bool Seek(u64 offset);
            u64 Position();
            u64 Size() const;
            void Close();

        private:
            std::ifstream _stream;
            u64 _size = 0;
        };

        class StreamWriter {
        public:
            explicit StreamWriter(const Path& path, bool append = false);
            ~StreamWriter();

            StreamWriter(const StreamWriter&)            = delete;
            StreamWriter& operator=(const StreamWriter&) = delete;

            StreamWriter(StreamWriter&&) noexcept;
            StreamWriter& operator=(StreamWriter&&) noexcept;

            bool Write(const vector<u8>& buffer);
            bool Write(const vector<u8>& buffer, size_t size);
            bool WriteLine(const str& line);
            bool Flush();

            bool IsOpen() const;
            bool Seek(u64 offset);
            u64 Position();
            void Close();

        private:
            std::ofstream _stream;
        };

        class DirectoryIterator;

        class Path {
        public:
            Path() = default;
            explicit Path(const str& path) : path(Normalize(path)) {}
            static Path Current();

            [[nodiscard]] Path Parent() const;
            [[nodiscard]] bool Exists() const;
            [[nodiscard]] bool IsFile() const;
            [[nodiscard]] bool IsDirectory() const;
            [[nodiscard]] bool HasExtension() const;
            [[nodiscard]] str Extension() const;
            [[nodiscard]] Path ReplaceExtension(const str& ext) const;
            [[nodiscard]] Path Join(const str& subPath) const;
            [[nodiscard]] Path operator/(const str& subPath) const;
            [[nodiscard]] str Str() const;
            [[nodiscard]] const char* CStr() const;
            [[nodiscard]] str Filename() const;

            [[nodiscard]] bool operator==(const Path& other) const;

            [[nodiscard]] bool Create() const;
            [[nodiscard]] bool CreateAll() const;

            // Iterator methods
            [[nodiscard]] DirectoryIterator begin() const;
            [[nodiscard]] DirectoryIterator end() const;
            [[nodiscard]] DirectoryIterator recursive_begin() const;
            [[nodiscard]] DirectoryIterator recursive_end() const;

        private:
            str path;
            static str Join(const str& lhs, const str& rhs);
            static str Normalize(const str& rawPath);
        };

        namespace detail {
            struct IteratorState {
                virtual ~IteratorState()                             = default;
                virtual bool MoveNext()                              = 0;
                virtual Path Current() const                         = 0;
                virtual std::unique_ptr<IteratorState> Clone() const = 0;
            };

            // No recursive directory iterator
            class FlatIteratorState final : public IteratorState {
            public:
                explicit FlatIteratorState(const Path& directory);
                bool MoveNext() override;
                Path Current() const override;
                std::unique_ptr<IteratorState> Clone() const override;

            private:
                Path mRootDir;
                Path mCurrent;
                bool mInitialized {false};

#ifdef _WIN32
                HANDLE mFileHandle {INVALID_HANDLE_VALUE};
                WIN32_FIND_DATAA mFindData {};
#else
                DIR* mDir {nullptr};
#endif

                void Initialize();
                void Cleanup();
            };

            class RecursiveIteratorState final : public IteratorState {
            public:
                explicit RecursiveIteratorState(const Path& directory);
                bool MoveNext() override;
                Path Current() const override;
                std::unique_ptr<IteratorState> Clone() const override;

            private:
                Path mRootDir;
                Path mCurrent;
                std::stack<Path> mDirectories;
                bool mInitialized {false};

#ifdef _WIN32
                HANDLE mFileHandle {INVALID_HANDLE_VALUE};
                WIN32_FIND_DATAA mFindData {};
#else
                DIR* mDir {nullptr};
                std::string mCurrentDirName;
#endif

                void Initialize();
                void Cleanup();
                bool OpenNextDirectory();
            };

            class EndIteratorState final : public IteratorState {
            public:
                bool MoveNext() override;
                Path Current() const override;
                std::unique_ptr<IteratorState> Clone() const override {
                    return std::make_unique<EndIteratorState>();
                }
            };
        }  // namespace detail

        class DirectoryIterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type        = Path;
            using difference_type   = std::ptrdiff_t;
            using pointer           = const Path*;
            using reference         = const Path&;

            explicit DirectoryIterator(const Path& directory, bool recursive = false);
            DirectoryIterator();

            DirectoryIterator(const DirectoryIterator& other);
            DirectoryIterator& operator=(const DirectoryIterator& other);

            DirectoryIterator(DirectoryIterator&& other) noexcept;
            DirectoryIterator& operator=(DirectoryIterator&& other) noexcept;

            ~DirectoryIterator() = default;

            reference operator*() const;
            pointer operator->() const;
            DirectoryIterator& operator++();
            DirectoryIterator operator++(int);
            bool operator==(const DirectoryIterator& other) const;
            bool operator!=(const DirectoryIterator& other) const;

        private:
            std::unique_ptr<detail::IteratorState> mState;
            mutable Path mCurrent;
        };
    }  // namespace Filesystem
}  // namespace x
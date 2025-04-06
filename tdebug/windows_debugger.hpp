#pragma once
#include <Windows.h>
#include <cstring>
#include <iostream>
#include <string>
#include "../teenyat.h"

namespace os {
    struct Process {
        private:
        PROCESS_INFORMATION processInfo {};

        public:
        int spawn(const char* process, char* argv[]) {
            STARTUPINFO startupInfo {};

            auto started = CreateProcess(
                process, 
                strstr(GetCommandLine(), process), 
                nullptr,
                nullptr,
                false, 
                NORMAL_PRIORITY_CLASS,
                nullptr,
                nullptr,
                &startupInfo,
                &processInfo
            );

            if (started) {
                return 0;
            } else {
                return GetLastError();
            }
        }

        bool isRunning() {
            return exitCode() == STILL_ACTIVE;
        }

        bool exited() {
            return !isRunning();
        }

        int exitCode() {
            DWORD exitCode = 0;
            auto success = GetExitCodeProcess(processInfo.hProcess, &exitCode);

            if (success) {
                return exitCode;
            } else {
                std::cerr << "Error while trying to get process status: " << exitCode << "\n";
                return exitCode;
            }
        }
    };
    
    struct SharedMemory {
        private: 
        HANDLE memory;
        HANDLE mutex;
        teenyat* t;
        bool closed;

        public:
        ~SharedMemory() {
            if (!closed) {
                std::cerr << "Shared memory was not closed! Attempting to close automatically, but this should be fixed in the future! Close status: " << release() << "\n";
            }
        }

        int create() {
            const TCHAR name[] = TEXT("Global\\t_debug");
            const TCHAR mName[] = TEXT("Global\\t_debug_mutex");

            memory = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(teenyat), name);

            if (memory == nullptr) {
                return GetLastError();
            }

            t = (teenyat*) MapViewOfFile(memory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(teenyat));

            if (t == nullptr) {
                // TODO: Release created mapping if this fails
                return GetLastError();
            }

            mutex = CreateMutexA(nullptr, true, mName);
            if (mutex == nullptr) {
                // TODO: Release created mapping AND unmap view
                return GetLastError();
            }
            
            return 0;
        }

        int lockTeenyAT() {
            return WaitForSingleObject(mutex, INFINITE);
        }

        teenyat* getTeenyAT() {
            return t;
        }

        int releaseTeenyAT() {
            return ReleaseMutex(mutex);
        }

        int release() {
            if (!UnmapViewOfFile(t)) {
                return GetLastError();
            }
            if (!CloseHandle(memory)) {
                return GetLastError();
            }
            if (!CloseHandle(mutex)) {
                return GetLastError();
            }
            return 0;
        }
    };

    inline std::string error_to_string(int error) {
        char errBuf[512] {};
        DWORD len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, 0, errBuf, 512, nullptr);

        if (len == 0) return "Unknown error!";

        return std::string(errBuf, len);
    }
}

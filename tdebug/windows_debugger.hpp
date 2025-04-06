#pragma once
#include <Windows.h>
#include <string>
#include <winnt.h>
#include "../teenyat.h"

namespace os {
    struct Process {
        private:
        HANDLE pid;

        public:
        int spawn(const char* process, char* argv[]) {
            return 0;
        }

        bool isRunning() {
            return false;
        }

        int exited() {
            return 0;
        }

        int exitCode() {
            return 0;
        }
    };
    
    struct SharedMemory {
        private: 
        HANDLE memory;

        public:
        int create() {
            return 0;
        }

        int lockTeenyAT() {
            return 0;
        }

        teenyat* getTeenyAT() {
            return nullptr;
        }

        int releaseTeenyAT() {
            return 0;
        }

        int release() {
            return 0;
        }
    };

    inline std::string error_to_string(int error) {
        return "";
    }
}


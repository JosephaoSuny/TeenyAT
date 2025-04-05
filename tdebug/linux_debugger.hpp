#pragma once
#include "../teenyat.h"
#include <cstring>
#include <iostream>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <sys/mman.h>

namespace os {
    typedef pid_t process_id;

    struct Process {
        private: 
        int pid;
        int status{};

        public:
        Process() {
            this->pid = 0;
        };

        int spawn(const char* process, char* argv[]) {
            posix_spawnattr_t attr {};
            auto result = 0;
        
            posix_spawnattr_init(&attr);
            result = posix_spawn(&pid, process, nullptr, &attr, argv, environ);
        
            return result;
        }

        bool isRunning() {
            return waitpid(this->pid, nullptr, WNOHANG) == 0;
        }

        bool exited() {
            return WIFEXITED(status);
        }

        int exitCode() {
            return WEXITSTATUS(status);
        }
    };

    // This represents operations on shared memory
    struct SharedMemory {
        private:
        teenyat* t = nullptr;
        int fd = 0;
        // This is used because we can't use a destructor
        bool closed;

        public:
            SharedMemory() {

            }

            ~SharedMemory() {
                if (!closed) {
                    int res = this->release();

                    std::cerr << "Memory was not closed, attempting to close automatically, but this should be fixed in the future: " << res <<"\n";
                }
            }

            int open() {
                return this->open_or_create(false);
            }

            int create() {
                return this->open_or_create(true);
            }

            int release() {
                const auto name = "/t_debug_" + std::to_string(getpid());
                int result = close(fd);
                if (result < 0) {
                    return errno;
                }
                closed = true;
                return 0;
            }

            int lockTeenyAT() {
                return flock(fd, LOCK_SH);
            }

            int releaseTeenyAT() {
                return flock(fd, LOCK_UN);
            }

            teenyat* getTeenyAT() {
                return this->t;
            }

        private:
            int open_or_create(bool create) {
                const auto flags = create ? O_RDWR | O_CREAT : O_RDWR;

                const auto name = "/t_debug_" + std::to_string(getpid());
                const auto fd = shm_open(name.c_str(), flags, 0644);
            
                if (fd < 0) {
                    return errno;
                }
                if (ftruncate(fd, sizeof(teenyat)) == -1) {
                    return errno;
                }
            
                auto shared_mem = mmap(nullptr, sizeof(teenyat), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
                memset(shared_mem, 0, sizeof(teenyat));
            
                this->t = (teenyat*) shared_mem;
                this->fd = fd;
            
                return 0;
            }
    };

    inline std::string error_to_string(int error) {
        return strerror(error);
    }
}
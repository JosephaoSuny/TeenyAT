#include "../teenyat.h"

#if defined (_WIN32) | defined (WIN32)
    inline int getSharedMemory() {
        return 0;
    }

    inline int update_shared_mem(teenyat* teeny) {
        return 0;
    }
#else
    #include <cstring>
    #include <unistd.h>
    #include <sys/mman.h>
    #include <sys/file.h>
    #include <cerrno>

    static int fd = 0;

    inline int getSharedMemory() {
        const auto flags = O_WRONLY;
        const char* name_start = "/t_debug_";
        const int name_start_len = 9;
        char name[name_start_len + 11];
        memcpy(name, name_start, 9);
        int ppid = getppid();
        sprintf(name + 9, "%d", ppid);

        const auto shared_fd = shm_open(name, flags, 0644);
    
        if (shared_fd < 0) {
            return errno;
        }

        fd = shared_fd;
    
        return 0;
    }

    inline int update_shared_mem(teenyat* teeny) {
        int result = 0;

        result = flock(fd, LOCK_SH);
        if (result != 0) {
            return errno;
        };

        result = pwrite(fd, teeny, sizeof(teenyat), 0);
        if (result < 0) {
            return errno;
        };

        result = flock(fd, LOCK_UN);
        if (result != 0) {
            return errno;
        };

        return 0;
    }
#endif

// 0 is being used as true, that might need to be fixed
inline int debugging() {
    return getSharedMemory();
}

inline int update_debugger(teenyat* teeny) {
    return update_shared_mem(teeny);
}
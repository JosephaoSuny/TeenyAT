#include "../teenyat.h"
#include <cstdio>
#include <fcntl.h>

#ifdef WIN32

#else
    #include <sys/stat.h>
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
            printf("%s\n", strerror(errno));
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

        result = write(fd, teeny, sizeof(teenyat));
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
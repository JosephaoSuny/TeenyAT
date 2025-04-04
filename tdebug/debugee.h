#include "../teenyat.h"
#include <cstddef>
#include <cstdio>
#include <cstring>

static teenyat* mem;

#ifdef WIN32

#else
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/file.h>
    #include <cerrno>
    #include <cstdlib>

    static int fd = 0;

    inline int getSharedMemory() {
        const auto flags = O_RDWR;
        const char* name_start = "/t_debug_";
        const int name_start_len = 9;
        char name[name_start_len + 11];
        memcpy(name, name_start, 9);
        int ppid = getppid();
        sprintf(name + 9, "%d", ppid);

        const auto fd = shm_open(name, flags, 0644);
    
        if (fd < 0) {
            printf("%s\n", strerror(errno));
            return errno;
        }
    
        auto shared_mem = mmap(NULL, sizeof(teenyat), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (shared_mem == MAP_FAILED) {
            printf("%s\n", strerror(errno));
            return -1;
        }
        mem = (teenyat*) shared_mem;
    
        return 0;
    }

    inline int update_shared_mem(teenyat* teeny) {
        int result = 0;

        if (fd == 0) return 0;

        result = flock(fd, LOCK_SH);
        if (result != 0) {
            return errno;
        };

        memcpy(mem, teeny, sizeof(teenyat));
        mem->initialized = true;

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
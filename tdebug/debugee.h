#include "../teenyat.h"

#if defined (_WIN32) | defined (WIN32)
    #include <Windows.h>

    static HANDLE mutex = nullptr;
    static HANDLE memory = nullptr;
    static teenyat* mapping = nullptr;

    inline int getSharedMemory() {
        const TCHAR name[] = TEXT("Global\\t_debug");
        const TCHAR mName[] = TEXT("Global\\t_debug_mutex");

        memory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);

        if (memory == nullptr) {
            int error = GetLastError();
            printf("%d\n", error);
            return error;
        }

        printf("Memory opened\n");

        mapping = (teenyat*) MapViewOfFile(memory, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(teenyat));

        if (mapping == nullptr) {
            // TODO: Release created mapping if this fails
            return GetLastError();
        }

        printf("Mapping created\n");

        mutex = CreateMutexA(nullptr, false, mName);
        if (mutex == nullptr) {
            // TODO: Release created mapping AND unmap view
            return GetLastError();
        }

        printf("Mutex opened\n");
        
        return 0;
    }

    inline int update_shared_mem(teenyat* teeny) {
        int result = 0;
        
        result = WaitForSingleObject(mutex, INFINITE);

        if (result != WAIT_OBJECT_0) return GetLastError();

        *mapping = *teeny;

        result = ReleaseMutex(mutex);

        if (result != 0) return GetLastError();

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
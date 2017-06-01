/*
 * gcc -ldl -fPIC -shared -o exec.so exec.c
 * LD_PRELOAD = exec.so
 * 
 */
#include <unistd.h>
#include <dlfcn.h>

typedef int(*EXEC)(
        const char *path,
        char *const argv[],
        char *const envp[]
        );

void *_handle = NULL;
void *_execve = NULL;

int execve(
        const char *path,
        char *const argv[],
        char *const envp[]) 
{
    if (!_handle) {
        _handle = dlopen("libc.so.6", RTLD_LAZY);
        _execve = (EXEC)dlsym(_handle, "execve");
    }

    if (_execve) {
        return ((EXEC)_execve)(path, argv, envp);
    } else {
        return -1;
    }
}


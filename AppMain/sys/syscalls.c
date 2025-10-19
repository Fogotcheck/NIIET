#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

/* Variables */
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));
static inline uint32_t pseudo_random();

char *__env[1] = { 0 };
char **environ = __env;

static uint32_t seed = 0x12345678; // Начальное значение

int _getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	(void)pid;
	(void)sig;
	errno = EINVAL;
	return -1;
}

void _exit(int status)
{
	_kill(status, -1);
	while (1) {
	} /* Make sure we hang here */
}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
	(void)file;
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		*ptr++ = __io_getchar();
	}

	return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
	(void)file;
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++) {
		__io_putchar(*ptr++);
	}
	return len;
}

int _close(int file)
{
	(void)file;
	return -1;
}

int _fstat(int file, struct stat *st)
{
	(void)file;
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	(void)file;
	return 1;
}

int _lseek(int file, int ptr, int dir)
{
	(void)file;
	(void)ptr;
	(void)dir;
	return 0;
}

int _open(char *path, int flags, ...)
{
	(void)path;
	(void)flags;
	/* Pretend like we always fail */
	return -1;
}

int _wait(int *status)
{
	(void)status;
	errno = ECHILD;
	return -1;
}

int _unlink(char *name)
{
	(void)name;
	errno = ENOENT;
	return -1;
}

int _times(struct tms *buf)
{
	(void)buf;
	return -1;
}

int _stat(char *file, struct stat *st)
{
	(void)file;
	st->st_mode = S_IFCHR;
	return 0;
}

int _link(char *old, char *new)
{
	(void)old;
	(void)new;
	errno = EMLINK;
	return -1;
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	(void)name;
	(void)argv;
	(void)env;
	errno = ENOMEM;
	return -1;
}

void seed_random(uint32_t new_seed)
{
	seed = new_seed;
}

static inline uint32_t pseudo_random()
{
	seed ^= (seed << 13);
	seed ^= (seed >> 17);
	seed ^= (seed << 5);
	return seed;
}

int _getentropy(void *buffer, size_t length)
{
	if (length == 0 || buffer == NULL) {
		return -1;
	}

	uint8_t *ptr = (uint8_t *)buffer;
	for (size_t i = 0; i < length; i++) {
		ptr[i] = (uint8_t)(pseudo_random() & 0xFF);
	}

	return 0;
}

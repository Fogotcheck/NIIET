#include <stddef.h>
#include <stdint.h>

extern char end;
extern char _end;
extern char __end;

extern char _Min_Heap_Size;

static char *heap_end = 0;

void *_sbrk(ptrdiff_t incr)
{
	char *prev_heap_end;
	char *heap_limit = &end + (size_t)&_Min_Heap_Size;

	if (heap_end == 0) {
		heap_end = &end;
	}

	prev_heap_end = heap_end;

	if ((heap_end + incr) > heap_limit) {
		return (void *)-1;
	}

	heap_end += incr;

	return (void *)prev_heap_end;
}

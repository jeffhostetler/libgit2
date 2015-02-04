#include "clar_libgit2.h"
#include "helper__perf__timer.h"

#if defined(GIT_WIN32)

void perf__timer__start(perf_timer *t)
{
	QueryPerformanceCounter(&t->time_started);
}

void perf__timer__stop(perf_timer *t)
{
	LARGE_INTEGER time_now;
	QueryPerformanceCounter(&time_now);

	t->sum.QuadPart += (time_now.QuadPart - t->time_started.QuadPart);
}

void perf__timer__report(perf_timer *t, const char *fmt, ...)
{
	va_list arglist;
	LARGE_INTEGER freq;
	double fraction;

	QueryPerformanceFrequency(&freq);

	fraction = ((double)t->sum.QuadPart) / ((double)freq.QuadPart);

	printf("%10.3f: ", fraction);

	va_start(arglist, fmt);
	vprintf(fmt, arglist);
	va_end(arglist);

	printf("\n");
}

#else

void perf__timer__start(perf_timer *t)
{
	/* TODO */
}

void perf__timer__stop(perf_timer *t)
{
	/* TODO */
}

void perf__timer__report(perf_timer *t, const char *fmt, ...)
{
	/* TODO */
}

#endif

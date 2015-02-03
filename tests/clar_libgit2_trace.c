#include "clar_libgit2.h"
#include "clar_libgit2_trace.h"
#include "trace.h"

#if defined(GIT_TRACE)
static void _trace_printf_cb(git_trace_level_t level, const char *msg)
{
	/* TODO Use level to print a per-message prefix. */
	GIT_UNUSED(level);

	printf("%s\n", msg);
}

#if 0 && defined(GIT_WIN32)
static void _trace_debug_cb(git_trace_level_t level, const char *msg)
{
	/* TODO Use level to print a per-message prefix. */
	GIT_UNUSED(level);

	OutputDebugString(msg);
	OutputDebugString("\n");

	printf("%s\n", msg);
}
#else
#define _trace_debug_cb _trace_printf_cb
#endif

static int s_trace_loaded = 0;
static int s_trace_level = GIT_TRACE_NONE;
static void (*s_trace_cb)(git_trace_level_t level, const char *message) = NULL;

static void _load_trace_params(void)
{
	char *sz_level;
	char *sz_method;

	s_trace_loaded = 1;

	sz_level = cl_getenv("CLAR_TRACE_LEVEL");
	if (!sz_level || !*sz_level) {
		s_trace_level = GIT_TRACE_NONE;
		s_trace_cb = NULL;
		return;
	}

	/* TODO Parse sz_level and set s_trace_level. */
	s_trace_level = GIT_TRACE_TRACE;

	s_trace_cb = _trace_printf_cb;
	sz_method = cl_getenv("CLAR_TRACE_METHOD");
	if (sz_method && *sz_method) {
		if (strcmp(sz_method, "printf") == 0)
			s_trace_cb = _trace_printf_cb;
		else if (strcmp(sz_method, "debug") == 0)
			s_trace_cb = _trace_debug_cb;
		else
			printf("Unknown trace method: '%s'\n", sz_method);
	}
}

void cl_maybe_enable_trace(const char *sz_test_name)
{
	if (!s_trace_loaded)
		_load_trace_params();

	git_trace_set(s_trace_level, s_trace_cb);

	if (s_trace_level != GIT_TRACE_NONE)
		git_trace(GIT_TRACE_TRACE, "Beginning test: %s", sz_test_name);
}
#endif /*GIT_TRACE*/

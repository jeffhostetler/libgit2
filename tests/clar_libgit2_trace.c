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

#if defined(GIT_WIN32)
static void _trace_debug_cb(git_trace_level_t level, const char *msg)
{
	/* TODO Use level to print a per-message prefix. */
	GIT_UNUSED(level);

	OutputDebugString(msg);
	OutputDebugString("\n");

	printf("%s\n", msg);
}
#endif

//static int s_trace_loaded = 0;
//static int s_trace_level = GIT_TRACE_NONE;
//static void (*s_trace_cb)(git_trace_level_t level, const char *message) = NULL;
static int s_trace_loaded = 1;
static int s_trace_level = GIT_TRACE_TRACE;
static void (*s_trace_cb)(git_trace_level_t level, const char *message) = _trace_debug_cb;

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

	sz_method = cl_getenv("CLAR_TRACE_METHOD");
	/* TODO Parse sz_method and allow alternate cb methods (if/when we define others). */
	cl_assert(!sz_method || !*sz_method || (strcmp(sz_method, "default") == 0));
	s_trace_cb = _trace_printf_cb;
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

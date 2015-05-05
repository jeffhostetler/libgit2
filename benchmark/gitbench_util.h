/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#ifndef H_GITBENCH_UTIL_H
#define H_GITBENCH_UTIL_H

/*****************************************************************/

#if defined(_WIN32)
#define BM_GIT_EXE "git.exe"
#else
#define BM_GIT_EXE "/usr/bin/git"
#endif

/*****************************************************************/


#define GITBENCH_EARGUMENTS (INT_MIN+1)

extern int gitbench_util_create_tempdir(char **out);

#endif

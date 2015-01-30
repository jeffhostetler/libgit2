#include "clar_libgit2.h"
#include "helper__perf__do_merge.h"
#include "trace.h"

static git_repository * g_repo;

void perf__do_merge(const char *fixture,
					const char *test_name,
					const char *id_a,
					const char *id_b)
{
	git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
	git_clone_options clone_opts = GIT_CLONE_OPTIONS_INIT;
	git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
	git_oid oid_a;
	git_oid oid_b;
	git_reference *ref_branch_a = NULL;
	git_reference *ref_branch_b = NULL;
	git_commit *commit_a = NULL;
	git_commit *commit_b = NULL;
	git_annotated_commit *annotated_commits[1] = { NULL };

	checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE_CREATE;
	clone_opts.checkout_opts = checkout_opts;
	cl_git_pass(git_signature_now(&clone_opts.signature, "Me", "foo@example.com"));

	git_trace(GIT_TRACE_TRACE, "perf__do_merge: clone beginning (%s,%s)",
			  fixture, test_name);
	cl_git_pass(git_clone(&g_repo, fixture, test_name, &clone_opts));
	git_trace(GIT_TRACE_TRACE, "perf__do_merge: clone finished");
	
	git_oid_fromstr(&oid_a, id_a);
	cl_git_pass(git_commit_lookup(&commit_a, g_repo, &oid_a));
	cl_git_pass(git_branch_create(&ref_branch_a, g_repo,
								  "A", commit_a,
								  0, NULL, NULL));

	git_trace(GIT_TRACE_TRACE, "perf__do_merge: checkout beginning (%s)", id_a);
	cl_git_pass(git_checkout_tree(g_repo, (git_object*)commit_a, &checkout_opts));
	git_trace(GIT_TRACE_TRACE, "perf__do_merge: checkout finished");

	cl_git_pass(git_repository_set_head(g_repo,
										git_reference_name(ref_branch_a),
										NULL, NULL));

	git_oid_fromstr(&oid_b, id_b);
	cl_git_pass(git_commit_lookup(&commit_b, g_repo, &oid_b));
	cl_git_pass(git_branch_create(&ref_branch_b, g_repo,
								  "B", commit_b,
								  0, NULL, NULL));

	cl_git_pass(git_annotated_commit_lookup(&annotated_commits[0], g_repo, &oid_b));

	git_trace(GIT_TRACE_TRACE, "perf__do_merge: merge beginning (%s)", id_b);
	cl_git_pass(git_merge(g_repo,
						  (const git_annotated_commit **)annotated_commits, 1,
						  &merge_opts, &checkout_opts));
	git_trace(GIT_TRACE_TRACE, "perf__do_merge: merge finished");

	git_reference_free(ref_branch_a);
	git_reference_free(ref_branch_b);
	git_commit_free(commit_a);
	git_commit_free(commit_b);
	git_annotated_commit_free(annotated_commits[0]);
}

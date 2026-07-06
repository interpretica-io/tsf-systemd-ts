/** @file
 * @brief systemd Group
 *
 * Enumerate the agent's systemd units and read one service's hardening
 * properties. The host decides what units exist, so this asserts no
 * particular unit; it checks the snapshot is non-empty and
 * self-consistent and that a hardening read of a present service
 * returns. A host without systemd is a clean skip.
 *
 * Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved.
 */

#define TE_TEST_NAME    "systemd/list"

#include "te_config.h"
#include "tapi_test.h"
#include "te_string.h"
#include "te_vector.h"

#include "tapi_systemd.h"
#include "tsapi_systemd.h"

int
main(int argc, char **argv)
{
    tsapi_systemd_session sess;
    te_vec units = TE_VEC_INIT(tapi_systemd_unit);
    const tapi_systemd_unit *u;
    unsigned int n;

    TEST_START;

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_systemd_session_init(&sess, "pco_systemd_list"));

    TEST_STEP("Snapshot the systemd units");
    rc = tapi_systemd_list(sess.pco, &units);
    if (rc != 0)
        TEST_SKIP("systemd is not available on the agent (%r)", rc);

    n = te_vec_size(&units);
    RING("the agent lists %u systemd unit(s)", n);
    if (n == 0)
        TEST_SKIP("no systemd units listed - treating as no systemd");

    TEST_STEP("Every listed unit is self-consistent; find a service");
    const char *a_service = NULL;
    TE_VEC_FOREACH(&units, u)
    {
        if (u->name == NULL || u->name[0] == '\0')
            TEST_VERDICT("a listed unit has no name");
        if (a_service == NULL && strstr(u->name, ".service") != NULL &&
            u->active != NULL && strcmp(u->active, "active") == 0)
            a_service = u->name;
    }
    tapi_systemd_unit_log(te_vec_get(&units, 0));

    TEST_STEP("Read one active service's hardening properties");
    if (a_service == NULL)
    {
        RING("no active *.service to inspect - list check alone passes");
    }
    else
    {
        tapi_systemd_hardening h;

        rc = tapi_systemd_hardening_get(sess.pco, a_service, &h);
        if (rc != 0)
            TEST_VERDICT("hardening read of %s failed (%r)", a_service, rc);
        if (!h.present)
            TEST_VERDICT("%s was listed but reports not present", a_service);
        RING("%s: NoNewPrivileges=%d ProtectSystem=%s caps=0x%llx", a_service,
             h.no_new_privileges,
             h.protect_system != NULL ? h.protect_system : "?",
             (unsigned long long)h.capability_bounding_set);
        tapi_systemd_hardening_free(&h);
    }

    TEST_SUCCESS;

cleanup:
    tapi_systemd_list_free(&units);
    tsapi_systemd_session_fini(&sess);
    TEST_END;
}

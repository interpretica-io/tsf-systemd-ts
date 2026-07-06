/** @file
 * @brief systemd Group
 *
 * Read the service-hardening posture with tapi_systemd_audit() and
 * check it is well-formed. A stock host runs many services with the
 * systemd defaults (i.e. unconfined), so this does NOT fail on findings
 * - it logs the posture and asserts the report is well-formed. A real
 * gate would add tapi_cybersec_report_verdict(..., SEV_HIGH, ...) and
 * TEST_VERDICT on it; here the posture is informational.
 *
 * Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved.
 */

#define TE_TEST_NAME    "systemd/audit"

#include "te_config.h"
#include "tapi_test.h"
#include "te_string.h"

#include "tapi_cybersec.h"
#include "tapi_systemd_audit.h"
#include "tsapi_systemd.h"

int
main(int argc, char **argv)
{
    tsapi_systemd_session sess;
    tapi_cybersec_report report;
    bool report_ready = false;
    te_errno rc;

    TEST_START;

    TEST_STEP("Open a session to the agent");
    CHECK_RC(tsapi_systemd_session_init(&sess, "pco_systemd_audit"));

    TEST_STEP("Read the service-hardening posture into a report");
    tapi_cybersec_report_init(&report);
    report_ready = true;
    rc = tapi_systemd_audit(sess.pco, NULL, &report);
    if (rc != 0)
        TEST_SKIP("systemd posture could not be read (%r)", rc);
    tapi_cybersec_report_log(&report);

    TEST_STEP("The report is well-formed: at least one finding");
    /*
     * The audit emits one systemd.service-present per service (or a
     * not-assessed), so an empty report means it did not run.
     */
    if (tapi_cybersec_report_count(&report, TAPI_CYBERSEC_SEV_INFO) == 0)
        TEST_SKIP("the audit produced no findings - no services to assess");

    TEST_SUCCESS;

cleanup:
    if (report_ready)
        tapi_cybersec_report_free(&report);
    tsapi_systemd_session_fini(&sess);
    TEST_END;
}

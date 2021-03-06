/*
 * Copyright © 2012-2015 VMware, Inc.  All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the “License”); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an “AS IS” BASIS, without
 * warranties or conditions of any kind, EITHER EXPRESS OR IMPLIED.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */



#include "srp_util.h"
#include "gssapi_alloc.h"

#include <gssapi/gssapi_krb5.h>
#include <errno.h>
#include <string.h>


OM_uint32
srp_gss_acquire_cred(
    OM_uint32 *minor_status,
    gss_name_t desired_name,
    OM_uint32 time_req,
    gss_OID_set desired_mechs,
    gss_cred_usage_t cred_usage,
    gss_cred_id_t *output_cred_handle,
    gss_OID_set *actual_mechs,
    OM_uint32 *time_rec)
{
    OM_uint32 major = 0;
    OM_uint32 minor = 0;
    srp_gss_cred_id_t srp_cred = NULL;
    gss_name_t gss_krb5_name_buf = NULL;

    /* Allocate the cred structure */
    srp_cred = (srp_gss_cred_id_t) gssalloc_malloc(sizeof(*srp_cred));
    if (!srp_cred)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }
    memset(srp_cred, 0, sizeof(*srp_cred));

    /* Allocate/set the mech OID; must be SRP for this method to be called */
    srp_cred->srp_mech_oid = (gss_OID) gssalloc_malloc(sizeof(*srp_cred->srp_mech_oid));
    if (!srp_cred->srp_mech_oid)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }
    memset(srp_cred->srp_mech_oid, 0, sizeof(*srp_cred->srp_mech_oid));
    srp_cred->srp_mech_oid->elements = (void *) gssalloc_malloc(GSS_SRP_MECH_OID_LEN);
    if (!srp_cred->srp_mech_oid->elements)
    {
        minor = ENOMEM;
        major = GSS_S_FAILURE;
        goto error;
    }
    srp_cred->srp_mech_oid->length = GSS_SRP_MECH_OID_LEN;
    memcpy(srp_cred->srp_mech_oid->elements, SRP_OID, GSS_SRP_MECH_OID_LEN);

    if (desired_name)
    {
        /* Really, use krb5 mech OID for name, as the desired output is a UPN */
        major = gss_canonicalize_name(&minor,
                                      desired_name,
                                      (gss_OID) gss_mech_krb5,
                                      &gss_krb5_name_buf);
        if (major)
        {
            goto error;
        }

        srp_cred->name = gss_krb5_name_buf, gss_krb5_name_buf = NULL;
    }
    *output_cred_handle = (gss_cred_id_t) srp_cred;

error:
    if (major || minor)
    {
        *minor_status = minor;
        if (srp_cred)
        {
            if (srp_cred->srp_mech_oid)
            {
                if (srp_cred->srp_mech_oid->elements)
                {
                    gssalloc_free(srp_cred->srp_mech_oid->elements);
                }
                gssalloc_free(srp_cred->srp_mech_oid);
            }
            gssalloc_free(srp_cred);
        }
    }

    return major;
}

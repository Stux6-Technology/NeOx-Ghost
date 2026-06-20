/* ==========================================================================
 *   PROJECT: NeOx Ghost (Advanced Microkernel Architecture Project)
 *   COPYRIGHT: (C) 2020 - 2026 Stux6 Technology Team. All Rights Reserved.
 *   DEVELOPER: Stux6 Tech. Lead Eng. Alperen ERKAN <erkanalperen54 [at] gmail.com> 
 *              or <stux6.team@gmail.com>
 * ==========================================================================
 *   LICENSE SUMMARY (STUX6 GENERAL PRIVATE PROJECT LICENSE - SGPPL-v1.0)
 * 
 *   1. This software and its kernel architecture are officially registered 
 *      intellectual property of the STUX6 TECHNOLOGY team.
 *   2. This code is made available strictly under "source-available" status 
 *      for personal research and local laboratory development only.
 *   3. ANY DISTRIBUTION, FORKING, OR RE-PUBLISHING ON ANY INTERNET PLATFORM 
 *      (INCLUDING GITHUB, GITLAB, BITBUCKET) IS STRICTLY PROHIBITED.
 *   4. Commercial enterprise, government network, or military deployment 
 *      requires express, hand-signed written authorization from the team captain.
 *   5. This header, copyright notices, and license text MUST remain untouched.
 * 
 *   FOR THE FULL TERMS AND CONDITIONS, REFER TO THE 'LICENSE' FILE.
 * ========================================================================== */
#ifndef __TERM_MIG_DECLS_H__
#define __TERM_MIG_DECLS_H__

#include <hurd/ports.h>

#include "term.h"

/* Called by server stub functions.  */

static inline struct port_info * __attribute__ ((unused))
begin_using_ctty_port (mach_port_t port)
{
  return ports_lookup_port (term_bucket, port, cttyid_class);
}

static inline struct port_info * __attribute__ ((unused))
begin_using_ctty_payload (uintptr_t payload)
{
  return ports_lookup_payload (term_bucket, payload, cttyid_class);
}

static inline void __attribute__ ((unused))
end_using_ctty (struct port_info *p)
{
  if (p)
    ports_port_deref (p);
}

#endif /* __TERM_MIG_DECLS_H__ */

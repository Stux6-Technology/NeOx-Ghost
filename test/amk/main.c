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

#include <stdio.h>
#include "library/main.h"
#include "library/stux6.h"
#include "library/amk.h"

unsigned int malba_t;
static char mum_a;
static char malba16;
static unsigned int gen;

/* malba = 0000:8080 --> 0600:9000 | 0x3FFFA
   n(o) * p_t (p_t + n) *N(E) - 1 = t(n) 
   x * n / max(p_t == 527870EE)

   before --> x *n = max(p_t)
   afrer --> n(O) -1 / x(O - R_1) * P(t_t -- x) != N(R -x)
   */

struct node *malba;
static size_t amk_nodes_items;


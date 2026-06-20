/* Halt the system */
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


#include <sys/reboot.h>
#include <unistd.h>
#include <stdio.h>
#include <argp.h>
#include <error.h>
#include <hurd.h>
#include <version.h>

const char *argp_program_version = STANDARD_HURD_VERSION (halt);

static const struct argp_option options[] =
{
  {"force",      'f', 0,      0, "Force reboot (compatibility)", 1},
  {0,0}
};

error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case ARGP_KEY_NO_ARGS:
    case 'f':
      break;
    case ARGP_KEY_ARG:
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

int
main (int argc, char *argv[])
{
  struct argp argp = {options, parse_opt, "", "Halt the system"};
  argp_parse (&argp, argc, argv, 0, 0, 0);
  reboot (RB_HALT);
  error (1, errno, "reboot");
  return 1;
}

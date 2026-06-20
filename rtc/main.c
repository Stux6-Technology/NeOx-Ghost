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

#include <version.h>

#include <error.h>
#include <stdbool.h>
#include <argp.h>
#include <hurd/trivfs.h>
#include <hurd/ports.h>
#include <hurd/rtc.h>
#include <sys/io.h>

#include "rtc_pioctl_S.h"

const char *argp_program_version = STANDARD_HURD_VERSION (rtc);

static struct trivfs_control *rtccntl;

int trivfs_fstype = FSTYPE_DEV;
int trivfs_fsid = 0;
int trivfs_support_read = 0;
int trivfs_support_write = 0;
int trivfs_support_exec = 0;
int trivfs_allow_open = O_READ | O_WRITE;

static const struct argp rtc_argp =
{ NULL, NULL, NULL, "Real-Time Clock device" };

static int
demuxer (mach_msg_header_t *inp, mach_msg_header_t *outp)
{
  mig_routine_t routine;
  if ((routine = rtc_pioctl_server_routine (inp)) ||
      (routine = NULL, trivfs_demuxer (inp, outp)))
    {
      if (routine)
        (*routine) (inp, outp);
      return TRUE;
    }
  else
    return FALSE;
}

int
main (int argc, char **argv)
{
  error_t err;
  mach_port_t bootstrap;

  argp_parse (&rtc_argp, argc, argv, 0, 0, 0);

  task_get_bootstrap_port (mach_task_self (), &bootstrap);
  if (bootstrap == MACH_PORT_NULL)
    error (1, 0, "Must be started as a translator");

  /* Request for permission to do i/o on port numbers 0x70 and 0x71 for
     accessing RTC registers.  Do this before replying to our parent, so
     we don't end up saying "I'm ready!" and then immediately exit with
     an error.  */
  err = ioperm (0x70, 2, true);
  if (err)
    error (1, err, "Request IO permission failed");

  /* Reply to our parent.  */
  err = trivfs_startup (bootstrap, O_NORW, NULL, NULL, NULL, NULL, &rtccntl);
  mach_port_deallocate (mach_task_self (), bootstrap);
  if (err)
    error (1, err, "trivfs_startup failed");

  /* Launch.  */
  ports_manage_port_operations_one_thread (rtccntl->pi.bucket, demuxer,
					   2 * 60 * 1000);

  return 0;
}

void
trivfs_modify_stat (struct trivfs_protid *cred, struct stat *st)
{
}

error_t
trivfs_goaway (struct trivfs_control *fsys, int flags)
{
  exit (EXIT_SUCCESS);
}

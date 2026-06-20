/* A translator for fifos*/
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

#include <hurd.h>
#include <stdio.h>
#include <error.h>
#include <sys/socket.h>
#include <hurd/paths.h>
#include <hurd/socket.h>
#include <hurd/fsys.h>
#include "fsys_S.h"

/* ---------------------------------------------------------------- */

extern int fsys_server (mach_msg_header_t *, mach_msg_header_t *);

/* The actual fifo, which is just a socket connected to itself.  */
static socket_t fifo;

void
main (int argc, char **argv)
{
  error_t err;
  char pflocal_name[1024];
  mach_port_t pflocal;
  mach_port_t bootstrap, fsys, realnode;
  
  if (argc != 1)
    {
      fprintf(stderr, "Usage: %s", program_invocation_name);
      exit(-1);
    }
  
  task_get_bootstrap_port (mach_task_self (), &bootstrap);
  if (bootstrap == MACH_PORT_NULL)
    error(3, 0, "Must be started as a translator");

  /* Reply to our parent */
  mach_port_allocate (mach_task_self (), MACH_PORT_RIGHT_RECEIVE, &fsys);
  err = fsys_startup (bootstrap, fsys, MACH_MSG_TYPE_MAKE_SEND, &realnode);
  mach_port_deallocate (mach_task_self (), bootstrap);
  if (err)
    error(1, err, "starting translator");

  /* Try and connect to the pflocal server */
  sprintf (pflocal_name, "%s/%d", _SERVERS_SOCKET, PF_LOCAL);
  pflocal = file_name_lookup (pflocal_name, 0, 0);
  if (pflocal == MACH_PORT_NULL)
    error (2, errno, "%s", pflocal_name);

  /* Make a local domain socket to use for our data buffering.  */
  err = socket_create (pflocal, SOCK_STREAM, 0, &fifo);
  if (err)
    error (3, err, "%s: socket_create", pflocal_name);

  /* Connect the socket to itself, yielding a fifo!  */
  err = socket_connect2 (fifo, fifo);
  if (err)
    error (3, err, "%s: socket_connect2", pflocal_name);

  for (;;)
    /* We don't ever time out.  The problem is, you only want to exit when
       (1) the pipe is empty (which we can check), and (2) there are no other
       users (which we can't).  If we just drop our ref to the pipe, there
       still could be a writer holding a ref to it.  */
    mach_msg_server_timeout (fsys_server, 0, fsys, 0, 0);
}

/* ---------------------------------------------------------------- */

error_t
S_fsys_getroot (mach_port_t fsys, mach_port_t parent,
		const id_t *uids, unsigned num_uids, const id_t *gids, unsigned num_gids,
		int flags,
		retry_type *do_retry, char *retry_name,
		mach_port_t *result, mach_msg_type_name_t *result_type)
{
  /* Give back a handle on our fifo.  */
  *do_retry = FS_RETRY_NORMAL;
  *retry_name = '\0';
  *result = fifo;
  *result_type = MACH_MSG_TYPE_COPY_SEND;
  return 0;
}

error_t
S_fsys_startup (mach_port_t bootstrap, mach_port_t fsys,
		mach_port_t *real, mach_msg_type_name_t *real_type)
{
  return EOPNOTSUPP;
}

error_t
S_fsys_goaway (mach_port_t fsys, int flags)
{
  /* If there are refs to the fifo left besides ours, it will
     stay around after we're gone.  */
  exit (0);
}

error_t
S_fsys_syncfs (mach_port_t fsys, int wait, int recurse)
{
  return 0;
}

error_t
S_fsys_set_options (mach_port_t fsys,
		    const char *data, mach_msg_type_number_t data_len, int recurse)
{
  return EOPNOTSUPP;
}

error_t
S_fsys_getfile (mach_port_t fsys,
		const uid_t *uids, unsigned num_uids, const gid_t *gids, unsigned num_gids,
		const char *handle, unsigned handle_len,
		mach_port_t *port, mach_msg_type_name_t *port_type)
{
  return EOPNOTSUPP;
}

error_t
S_fsys_getpriv (mach_port_t fsys,
		mach_port_t *hostpriv, mach_port_t *devmaster, task_t *fstask)
{
  return EOPNOTSUPP;
}

error_t
S_fsys_init (mach_port_t fsys,
	     mach_port_t reply, mach_msg_type_name_t reply_type,
	     mach_port_t proc, auth_t auth)
{
  return EOPNOTSUPP;
}

error_t
S_fsys_forward (mach_port_t server, mach_port_t requestor,
		const char *argz, mach_msg_type_number_t argz_len)
{
  return EOPNOTSUPP;
}

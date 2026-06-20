/* remap -- a translator for changing paths */
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

#include <hurd/trivfs.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <error.h>
#include <string.h>

#include <version.h>

int trivfs_fstype = FSTYPE_MISC;
int trivfs_fsid = 0;
int trivfs_support_read = 0;
int trivfs_support_write = 0;
int trivfs_support_exec = 0;
int trivfs_allow_open = 0;

void
trivfs_modify_stat (struct trivfs_protid *cred, struct stat *st)
{
  /* Don't care */  
}

error_t
trivfs_goaway (struct trivfs_control *cntl, int flags)
{
  exit (0);
}

struct remap
{
  char *from;
  char *to;
  struct remap *next;
};

static struct remap *remaps;

error_t
trivfs_S_dir_lookup (struct trivfs_protid *diruser,
		     mach_port_t reply, mach_msg_type_name_t reply_type,
		     const_string_t filename,
		     int flags,
		     mode_t mode,
		     retry_type *do_retry,
		     char *retry_name,
		     mach_port_t *retry_port,
		     mach_msg_type_name_t *retry_port_type)
{
  struct remap *remap;
  string_t dest = { };
  size_t prefix_size;

  if (!diruser)
    return EOPNOTSUPP;

  for (remap = remaps; remap; remap = remap->next)
    {
      prefix_size = strlen (remap->from);
      if (!strncmp (remap->from, filename, prefix_size)
	  && (filename[prefix_size] == '\0' || filename[prefix_size] == '/'))
	{
	  snprintf (dest, sizeof (dest), "%s%s", remap->to,
		    filename + prefix_size);

#ifdef DEBUG
	  fprintf (stderr, "replacing %s with %s\n", filename, dest);
	  fflush (stderr);
#endif

	  filename = dest;
	  break;
	}
    }

  *do_retry = FS_RETRY_REAUTH;
  *retry_port = getcrdir ();
  *retry_port_type = MACH_MSG_TYPE_COPY_SEND;
  strcpy (retry_name, filename);

  return 0;
}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  static char *remap_from;

  switch (key)
  {
    case ARGP_KEY_ARG:
      if (arg[0] != '/')
	{
	  argp_error (state, "remap only works with absolute paths: %s",
		      arg);
	  return EINVAL;
	}

      /* Skip heading slashes */
      while (arg[0] == '/')
        arg++;

      if (!remap_from)
	/* First of a pair */
	remap_from = strdup (arg);
      else
	{
	  /* Second of a pair */
	  struct remap *remap = malloc (sizeof (*remap));
	  remap->from = remap_from;
	  remap->to = strdup (arg);
	  remap->next = remaps;
#ifdef DEBUG
	  fprintf (stderr, "adding remap %s->%s\n", remap->from, remap->to);
#endif
	  remaps = remap;
	  remap_from = NULL;
	}

      break;
  }
  return 0;
}

const char *argp_program_version = STANDARD_HURD_VERSION (remap);

int
main (int argc, char **argv)
{
  error_t err;
  mach_port_t bootstrap;

  struct argp argp = { NULL, parse_opt, "[ FROM1 TO1 [ FROM2 TO2 [ ... ] ] ]", "\
A translator for remapping directories.\v\
This translator is to be used as a chroot, within which paths point to the\
same files as the original root, except a given set of paths, which are\
remapped to given paths." };

  argp_parse (&argp, argc, argv, ARGP_IN_ORDER, 0, 0);

  task_get_bootstrap_port (mach_task_self (), &bootstrap);
  struct trivfs_control *fsys;

  err = trivfs_startup (bootstrap, 0, 0, 0, 0, 0, &fsys);
  if (err)
    error (1, err, "trivfs_startup failed");
  ports_manage_port_operations_one_thread (fsys->pi.bucket, trivfs_demuxer, 0);

  /*NOTREACHED*/
  return 0;
}

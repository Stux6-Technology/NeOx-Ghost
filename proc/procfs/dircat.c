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

#include <stdlib.h>
#include <string.h>
#include "procfs.h"

struct dircat_node
{
  int num_dirs;
  struct node *dirs[0];
};

static error_t
dircat_get_contents (void *hook, char **contents, ssize_t *contents_len)
{
  struct dircat_node *dcn = hook;
  int i, sz, pos;
  error_t err;

  pos = 0;
  *contents = malloc (sz = 512);

  for (i=0; i < dcn->num_dirs; i++)
    {
      char *subcon;
      ssize_t sublen;

      /* Make sure we're not getting some old stuff.  */
      procfs_refresh (dcn->dirs[i]);

      err = procfs_get_contents (dcn->dirs[i], &subcon, &sublen);
      if (err)
        {
	  free (*contents);
	  *contents = NULL;
	  return err;
	}

      while (pos + sublen > sz)
	*contents = realloc (*contents, sz *= 2);

      memcpy (*contents + pos, subcon, sublen);
      pos += sublen;
    }

  *contents_len = pos;
  return 0;
}

static error_t
dircat_lookup (void *hook, const char *name, struct node **np)
{
  struct dircat_node *dcn = hook;
  error_t err;
  int i;

  err = ENOENT;
  for (i=0; err && i < dcn->num_dirs; i++)
    err = procfs_lookup (dcn->dirs[i], name, np);

  return err;
}

static void
dircat_release_dirs (struct node *const *dirs, int num_dirs)
{
  int i;

  for (i=0; i < num_dirs; i++)
    if (dirs[i])
      netfs_nrele (dirs[i]);
}

static void
dircat_cleanup (void *hook)
{
  struct dircat_node *dcn = hook;

  dircat_release_dirs (dcn->dirs, dcn->num_dirs);
  free (dcn);
}

struct node *
dircat_make_node (struct node *const *dirs, int num_dirs)
{
  static struct procfs_node_ops ops = {
    .get_contents = dircat_get_contents,
    .cleanup_contents = procfs_cleanup_contents_with_free,
    .lookup = dircat_lookup,
    .cleanup = dircat_cleanup,
  };
  struct dircat_node *dcn;
  int i;

  for (i=0; i < num_dirs; i++)
    if (! dirs[i])
      goto fail;

  dcn = malloc (sizeof *dcn + num_dirs * sizeof dcn->dirs[0]);
  if (! dcn)
    goto fail;

  dcn->num_dirs = num_dirs;
  memcpy (dcn->dirs, dirs, num_dirs * sizeof dcn->dirs[0]);
  return procfs_make_node (&ops, dcn);

fail:
  dircat_release_dirs (dirs, num_dirs);
  return NULL;
}


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


 #include <fcntl.h>

#include "hostmux.h"

/* Node maintenance.  */

/* Node NP is all done; free all its associated storage. */
void
netfs_node_norefs (struct node *node)
{
  if (node->nn->name)
    /* Remove our name's pointer to us; the name itself will eventually be
       freed by another party.  */
    node->nn->name->node = 0;
  free (node->nn);
  free (node);
}

/* Attempt to create a file named NAME in DIR for USER with MODE.  Set *NODE
   to the new node upon return.  On any error, clear *NODE.  *NODE should be
   locked on success; no matter what, unlock DIR before returning.  */
error_t
netfs_attempt_create_file (struct iouser *user, struct node *dir,
			   const char *name, mode_t mode, struct node **node)
{
  *node = 0;
  pthread_mutex_unlock (&dir->lock);
  return EOPNOTSUPP;
}

/* Node NODE is being opened by USER, with FLAGS.  NEWNODE is nonzero if we
   just created this node.  Return an error if we should not permit the open
   to complete because of a permission restriction. */
error_t
netfs_check_open_permissions (struct iouser *user, struct node *node,
			      int flags, int newnode)
{
  error_t err = 0;
  if (flags & O_READ)
    err = fshelp_access (&node->nn_stat, S_IREAD, user);
  if (!err && (flags & O_WRITE))
    err = fshelp_access (&node->nn_stat, S_IWRITE, user);
  if (!err && (flags & O_EXEC))
    err = fshelp_access (&node->nn_stat, S_IEXEC, user);
  return err;
}

/* This should attempt a utimes call for the user specified by CRED on node
   NODE, to change the atime to ATIME and the mtime to MTIME. */
error_t

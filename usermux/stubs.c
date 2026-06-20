/* Stub routines for usermux */
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

#include <hurd/netfs.h>

/* Attempt to turn NODE (user CRED) into a symlink with target NAME. */
error_t
netfs_attempt_mksymlink (struct iouser *cred, struct node *node, const char *name)
{
  return EOPNOTSUPP;
}

/* Attempt to turn NODE (user CRED) into a device.  TYPE is either S_IFBLK or
   S_IFCHR. */
error_t
netfs_attempt_mkdev (struct iouser *cred, struct node *node,
		     mode_t type, dev_t indexes)
{
  return EOPNOTSUPP;
}

/* Attempt to set the passive translator record for FILE to ARGZ (of length
   ARGZLEN) for user CRED. */
error_t
netfs_set_translator (struct iouser *cred, struct node *node,
		      const char *argz, mach_msg_type_number_t argzlen)
{
  return EOPNOTSUPP;
}

/* This should attempt a chflags call for the user specified by CRED on node
   NODE, to change the flags to FLAGS. */
error_t
netfs_attempt_chflags (struct iouser *cred, struct node *node, int flags)
{
  return EOPNOTSUPP;
}

/* This should attempt to set the size of the file NODE (for user CRED) to
   SIZE bytes long. */
error_t
netfs_attempt_set_size (struct iouser *cred, struct node *node, off_t size)
{
  return EOPNOTSUPP;
}

/* This should attempt to fetch filesystem status information for the remote
   filesystem, for the user CRED. */
error_t
netfs_attempt_statfs (struct iouser *cred, struct node *node,
		      struct statfs *st)
{
  return EOPNOTSUPP;
}

/* Delete NAME in DIR for USER. */
error_t
netfs_attempt_unlink (struct iouser *user, struct node *dir, const char *name)
{
  return EOPNOTSUPP;
}

/* Note that in this one call, neither of the specific nodes are locked. */
error_t
netfs_attempt_rename (struct iouser *user, struct node *fromdir,
		      const char *fromname, struct node *todir,
		      const char *toname, int excl)
{
  return EOPNOTSUPP;
}

/* Attempt to create a new directory named NAME in DIR for USER with mode
   MODE.  */
error_t
netfs_attempt_mkdir (struct iouser *user, struct node *dir,
		     const char *name, mode_t mode)
{
  return EOPNOTSUPP;
}

/* Attempt to remove directory named NAME in DIR for USER. */
error_t
netfs_attempt_rmdir (struct iouser *user,
		     struct node *dir, const char *name)
{
  return EOPNOTSUPP;
}

/* Create a link in DIR with name NAME to FILE for USER.  Note that neither
   DIR nor FILE are locked.  If EXCL is set, do not delete the target, but
   return EEXIST if NAME is already found in DIR.  */
error_t
netfs_attempt_link (struct iouser *user, struct node *dir,
		    struct node *file, const char *name, int excl)
{
  return EOPNOTSUPP;
}

/* Attempt to create an anonymous file related to DIR for USER with MODE.
   Set *NODE to the returned file upon success.  No matter what, unlock DIR. */
error_t
netfs_attempt_mkfile (struct iouser *user, struct node *dir,
		      mode_t mode, struct node **node)
{
  return EOPNOTSUPP;
}

/* Read from the file NODE for user CRED starting at OFFSET and continuing for
   up to *LEN bytes.  Put the data at DATA.  Set *LEN to the amount
   successfully read upon return.  */
error_t
netfs_attempt_read (struct iouser *cred, struct node *node,
		    off_t offset, size_t *len, void *data)
{
  return EOPNOTSUPP;
}

/* Write to the file NODE for user CRED starting at OFSET and continuing for up
   to *LEN bytes from DATA.  Set *LEN to the amount seccessfully written upon
   return. */
error_t
netfs_attempt_write (struct iouser *cred, struct node *node,
		     off_t offset, size_t *len, const void *data)
{
  return EOPNOTSUPP;
}

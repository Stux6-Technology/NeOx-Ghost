/* Private data structures for tmpfs. */
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

#ifndef _tmpfs_h
#define _tmpfs_h 1

#include <hurd/diskfs.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdint.h>

struct disknode
{
  uint_fast8_t type;		/* DT_REG et al */

  unsigned int gen;
  off_t size;
  mode_t mode;
  nlink_t nlink;
  uid_t uid, author;
  gid_t gid;
  struct timespec atime, mtime, ctime;
  unsigned int flags;

  char *trans;
  size_t translen;

  union
  {
    char *lnk;			/* malloc'd symlink target */
    struct
    {
      mach_port_t memobj, ro_memobj;
      vm_address_t memref;
      unsigned int allocpages;	/* largest size while memobj was live */
    } reg;
    struct
    {
      struct tmpfs_dirent *entries;
      struct disknode *dotdot;
    } dir;
    dev_t chr, blk;
  } u;

  struct node *hnext, **hprevp;
};

struct tmpfs_dirent
{
  struct tmpfs_dirent *next;
  struct disknode *dn;
  uint8_t namelen;
  char name[0];
};

extern off_t tmpfs_page_limit;
extern mach_port_t default_pager;

/* These two must be accessed using atomic operations.  */
extern unsigned int num_files;
extern off_t tmpfs_space_used;

/* Convenience function to adjust tmpfs_space_used.  */
static inline void
adjust_used (off_t change)
{
  __atomic_add_fetch (&tmpfs_space_used, change, __ATOMIC_RELAXED);
}

/* Convenience function to get tmpfs_space_used.  */
static inline off_t
get_used (void)
{
  return __atomic_load_n (&tmpfs_space_used, __ATOMIC_RELAXED);
}
 
#endif

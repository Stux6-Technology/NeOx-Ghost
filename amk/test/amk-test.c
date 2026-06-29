/* Root neoxmux amk protocol node */

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

/* 

   Copyright (C) 1997,99,2002 Free Software Foundation, Inc.
   Written by Miles Bader <miles@gnu.org>
   This file is part of the GNU Hurd.

   The GNU Hurd is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   The GNU Hurd is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA. 
*/

#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/mman.h>

#include "hostmux.h"

error_t create_host_node (struct hostmux *mux, struct hostmux_name *name,
			  struct node **node);

/* Returned directory entries are aligned to blocks this many bytes long.
   Must be a power of two.  */
#define DIRENT_ALIGN 4
#define DIRENT_NAME_OFFS offsetof (struct dirent, d_name)

/* Length is structure before the name + the name + '\0', all
   padded to a four-byte alignment.  */
#define DIRENT_LEN(name_len)						      \
  ((DIRENT_NAME_OFFS + (name_len) + 1 + (DIRENT_ALIGN - 1))		      \
   & ~(DIRENT_ALIGN - 1))

static error_t lookup_host (struct hostmux *mux, const char *host,
			    struct node **node); /* fwd decl */

/* [root] Directory operations.  */

/* Lookup NAME in DIR for USER; set *NODE to the found name upon return.  If
   the name was not found, then return ENOENT.  On any error, clear *NODE.
   (*NODE, if found, should be locked, this call should unlock DIR no matter
   what.) */
error_t
netfs_attempt_lookup (struct iouser *user, struct node *dir,
		      const char *name, struct node **node)
{
  error_t err;

  if (dir->nn->name)
    err = ENOTDIR;
  else
    err = fshelp_access (&dir->nn_stat, S_IEXEC, user);

  if (! err)
    {
      if (strcmp (name, ".") == 0)
	/* Current directory -- just add an additional reference to DIR and
	   return it.  */
	{
	  netfs_nref (dir);
	  *node = dir;
	  err = 0;
	}
      else if (strcmp (name, "..") == 0)
	err = EAGAIN;
      else
	err = lookup_host (dir->nn->mux, name, node);

      fshelp_touch (&dir->nn_stat, TOUCH_ATIME, hostmux_maptime);
    }

  pthread_mutex_unlock (&dir->lock);
  if (err)
    *node = 0;
  else
    pthread_mutex_lock (&(*node)->lock);

  return err;
}

/* Implement the netfs_get_directs callback as described in
   <hurd/netfs.h>. */
error_t
netfs_get_dirents (struct iouser *cred, struct node *dir,
		   int first_entry, int num_entries, char **data,
		   mach_msg_type_number_t *data_len,
		   vm_size_t max_data_len, int *data_entries)
{
  error_t err;
  int count;
  size_t size = 0;		/* Total size of our return block.  */
  struct hostmux_name *first_name, *nm;

  /* Add the length of a directory entry for NAME to SIZE and return true,
     unless it would overflow MAX_DATA_LEN or NUM_ENTRIES, in which case
     return false.  */
  int bump_size (const char *name)
    {
      if (num_entries == -1 || count < num_entries)
	{
	  size_t new_size = size + DIRENT_LEN (strlen (name));
	  if (max_data_len > 0 && new_size > max_data_len)
	    return 0;
	  size = new_size;
	  count++;
	  return 1;
	}
      else
	return 0;
    }

  if (dir->nn->name)
    return ENOTDIR;

  pthread_rwlock_rdlock (&dir->nn->mux->names_lock);

  /* Find the first entry.  */
  for (first_name = dir->nn->mux->names, count = 2;
       first_name && first_entry > count;
       first_name = first_name->next)
    if (first_name->node)
      count++;

  count = 0;

  /* Make space for the `.' and `..' entries.  */
  if (first_entry == 0)
    bump_size (".");
  if (first_entry <= 1)
    bump_size ("..");

  /* See how much space we need for the result.  */
  for (nm = first_name; nm; nm = nm->next)
    if (nm->node && !bump_size (nm->name))
      break;

  /* Allocate it.  */
  *data = mmap (0, size, PROT_READ|PROT_WRITE, MAP_ANON, 0, 0);
  err = ((void *) *data == (void *) -1) ? errno : 0;

  if (! err)
    /* Copy out the result.  */
    {
      char *p = *data;

      int add_dir_entry (const char *name, ino_t fileno, int type)
	{
	  if (num_entries == -1 || count < num_entries)
	    {
	      struct dirent hdr;
	      size_t name_len = strlen (name);
	      size_t sz = DIRENT_LEN (name_len);

	      if (sz > size)
		return 0;
	      else
		size -= sz;

	      hdr.d_fileno = fileno;
	      hdr.d_reclen = sz;
	      hdr.d_type = type;
	      hdr.d_namlen = name_len;

	      memcpy (p, &hdr, DIRENT_NAME_OFFS);
	      strcpy (p + DIRENT_NAME_OFFS, name);
	      p += sz;

	      count++;

	      return 1;
	    }
	  else
	    return 0;
	}

      *data_len = size;
      *data_entries = count;

      count = 0;

      /* Add `.' and `..' entries.  */
      if (first_entry == 0)
	add_dir_entry (".", 2, DT_DIR);
      if (first_entry <= 1)
	add_dir_entry ("..", 2, DT_DIR);

      /* Fill in the real directory entries.  */
      for (nm = first_name; nm; nm = nm->next)
	if (nm->node
	    && !add_dir_entry (nm->name, nm->fileno,
			       strcmp (nm->canon, nm->name) == 0
			         ? DT_REG : DT_LNK))
	  break;

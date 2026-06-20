/* Multiplexing filesystems by user */
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
#ifndef __USERMUX_H__
#define __USERMUX_H__

#include <hurd/netfs.h>
#include <pthread.h>
#include <maptime.h>

struct passwd;

/* Filenos (aka inode numbers) for user nodes are the uid + this.  */
#define USERMUX_FILENO_UID_OFFSET	10

/* Handy source of time.  */
extern volatile struct mapped_time_value *usermux_maptime;

/* The state associated with a user multiplexer translator.  */
struct usermux
{
  /* The user nodes in this mux.  */
  struct usermux_name *names;
  pthread_rwlock_t names_lock;

  /* A template argz, which is used to start each user-specific translator
     with the user name appropriately added.  */
  char *trans_template;
  size_t trans_template_len;

  /* What string to replace in TRANS_TEMPLATE with the name of the various
     user params; if none occur in the template, the user's home dir is
     appended as an additional argument.  */
  char *user_pat;		/* User name */
  char *home_pat;		/* Home directory */
  char *uid_pat;		/* Numeric user id */

  /* Constant fields for user stat entries.  */
  struct stat stat_template;

  /* The file that this translator is sitting on top of; we inherit various
     characteristics from it.  */
  file_t underlying;
};

/* The name of a recently looked up user entry.  */
struct usermux_name
{
  const char *name;		/* Looked up name.  */

  /* A filesystem node associated with NAME.  */
  struct node *node;

  struct usermux_name *next;
};

/* The fs specific storage that libnetfs associates with each filesystem
   node.  */
struct netnode
{
  /* The mux this node belongs to (the node can either be the mux root, or
     one of the users served by it).  */
  struct usermux *mux;

  /* For mux nodes, 0, and for leaf nodes, the name under which the node was
     looked up. */
  struct usermux_name *name;

  /* The translator associated with node, or if its a symlink, just the link
     target.  */
  char *trans;
  size_t trans_len;
};

error_t create_user_node (struct usermux *mux, struct usermux_name *name,
			  struct passwd *pw, struct node **node);

#ifndef USERMUX_EI
# define USERMUX_EI __extern_inline
#endif

#endif /* __USERMUX_H__ */

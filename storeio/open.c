/* Per-open information for storeio */
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

#include "open.h"
#include "dev.h"

/* Returns a new per-open structure for the device DEV in OPEN.  If an error
   occurs, the error-code is returned, otherwise 0.  */
error_t
open_create (struct dev *dev, struct open **open)
{
  *open = malloc (sizeof (struct open));
  if (*open == NULL)
    return ENOMEM;

  (*open)->dev = dev;
  (*open)->offs = 0;
  pthread_mutex_init (&(*open)->lock, NULL);

  return 0;
}

/* Free OPEN and any resources it holds.  */
void
open_free (struct open *open)
{
  free (open);
}

/* Writes up to LEN bytes from BUF to OPEN's device at device offset OFFS
   (which may be ignored if the device doesn't support random access),
   and returns the number of bytes written in AMOUNT.  If no error occurs,
   zero is returned, otherwise the error code is returned.  */
error_t
open_write (struct open *open, off_t offs, const void *buf, size_t len,
	    vm_size_t *amount)
{
  error_t err;
  if (offs < 0)
    /* Use OPEN's offset.  */
    {
      pthread_mutex_lock (&open->lock);
      err = dev_write (open->dev, open->offs, buf, len, amount);
      if (! err)
	open->offs += *amount;
      pthread_mutex_unlock (&open->lock);
    }
  else
    err = dev_write (open->dev, offs, buf, len, amount);
  return err;
}    

/* Reads up to AMOUNT bytes from the device into BUF and LEN using the
   standard mach out-array convention.  If no error occurs, zero is returned,
   otherwise the error code is returned.  */
error_t
open_read (struct open *open, off_t offs, vm_size_t amount,
	   void **buf, vm_size_t *len)
{
  error_t err;
  if (offs < 0)
    /* Use OPEN's offset.  */
    {
      pthread_mutex_lock (&open->lock);
      err = dev_read (open->dev, open->offs, amount, buf, len);
      if (! err)
	open->offs += *len;
      pthread_mutex_unlock (&open->lock);
    }
  else
    err = dev_read (open->dev, offs, amount, buf, len);
  return err;
}   

/* Set OPEN's location to OFFS, interpreted according to WHENCE as by seek.
   The new absolute location is returned in NEW_OFFS (and may not be the same
   as OFFS).  If no error occurs, zero is returned, otherwise the error code
   is returned.  */
error_t
open_seek (struct open *open, off_t offs, int whence, off_t *new_offs)
{
  error_t err = 0;

  pthread_mutex_lock (&open->lock);

  switch (whence)
    {
    case SEEK_CUR:
      offs += open->offs;
      goto check;
    case SEEK_END:
      offs += open->dev->store->size;
    case SEEK_SET:
    check:
      if (offs >= 0)
	{
	  *new_offs = open->offs = offs;
	  break;
	}
    default:
      err = EINVAL;
    }

  pthread_mutex_unlock (&open->lock);

  return err;
}

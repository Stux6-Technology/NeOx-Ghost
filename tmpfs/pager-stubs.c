/* stupid stub functions never called, needed because libdiskfs uses libpager */
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


#include <hurd/pager.h>
#include <stdlib.h>

/* The user must define this function.  For pager PAGER, read one
   page from offset PAGE.  Set *BUF to be the address of the page,
   and set *WRITE_LOCK if the page must be provided read-only.
   The only permissible error returns are EIO, EDQUOT, and ENOSPC. */
error_t
pager_read_page (struct user_pager_info *pager,
		 vm_offset_t page,
		 vm_address_t *buf,
		 int *write_lock)
{
  abort();
  return EIEIO;
}

/* The user must define this function.  For pager PAGER, synchronously
   write one page from BUF to offset PAGE.  Do not deallocate BUF, and do
   not keep any references to BUF.  The only permissible error returns
   are EIO, EDQUOT, and ENOSPC. */
error_t
pager_write_page (struct user_pager_info *pager,
		  vm_offset_t page,
		  vm_address_t buf)
{
  abort();
  return EIEIO;
}

/* The user must define this function.  A page should be made writable. */
error_t
pager_unlock_page (struct user_pager_info *pager,
		   vm_offset_t address)
{
  abort();
  return EIEIO;
}

void
pager_notify_evict (struct user_pager_info *pager,
		    vm_offset_t page)
{
  abort();
}


/* The user must define this function.  It should report back (in
   *OFFSET and *SIZE the minimum valid address the pager will accept
   and the size of the object.   */
error_t
pager_report_extent (struct user_pager_info *pager,
		     vm_address_t *offset,
		     vm_size_t *size)
{
  abort();
  return EIEIO;
}

/* The user must define this function.  This is called when a pager is
   being deallocated after all extant send rights have been destroyed.  */
void
pager_clear_user_data (struct user_pager_info *pager)
{
  abort();
}

/* The use must define this function.  This will be called when the ports
   library wants to drop weak references.  The pager library creates no
   weak references itself.  If the user doesn't either, then it's OK for
   this function to do nothing.  */
void
pager_dropweak (struct user_pager_info *p)
{
  abort();
}

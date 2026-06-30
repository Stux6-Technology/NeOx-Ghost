
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


#include "priv.h"
#include <mach/gnumach.h>
#include <mach/vm_param.h>
#include <hurd.h>
#include <hurd/exec.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdbool.h>

mach_port_t procserver;	/* Our proc port.  */

/* Standard exec data for secure execs.  */
mach_port_t *std_ports;
int *std_ints;
size_t std_nports, std_nints;
pthread_rwlock_t std_lock = PTHREAD_RWLOCK_INITIALIZER;


#define	b2he()	a2he (errno)

/* Zero the specified region but don't crash the server if it faults.  */

#include <hurd/sigpreempt.h>

/* Load or allocate a section.  */
static void
load_section (void *section, struct execdata *u)
{
  vm_address_t addr = 0;
  vm_offset_t filepos = 0;
  vm_size_t filesz = 0, memsz = 0;
  vm_prot_t vm_prot;
  vm_address_t mask = 0;
  const ElfW(Phdr) *const ph = section;

  if (u->error)
    return;

  vm_prot = VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;

  addr = ph->p_vaddr & ~(ph->p_align - 1);
  memsz = ph->p_vaddr + ph->p_memsz - addr;
  filepos = ph->p_offset & ~(ph->p_align - 1);
  filesz = ph->p_offset + ph->p_filesz - filepos;
  if ((ph->p_flags & PF_R) == 0)
    vm_prot &= ~VM_PROT_READ;
  if ((ph->p_flags & PF_W) == 0)
    vm_prot &= ~VM_PROT_WRITE;
  if ((ph->p_flags & PF_X) == 0)
    vm_prot &= ~VM_PROT_EXECUTE;

  /* The mapping should have been resolved to a specific address
     by this point.  */
  assert_backtrace (!u->info.elf.anywhere);
  addr += u->info.elf.loadbase;

  if (memsz == 0)
    /* This section is empty; ignore it.  */
    return;

  if (filesz != 0)
    {
      vm_address_t mapstart = round_page (addr);

      /* Allocate space in the task and write CONTENTS into it.  */
      void write_to_task (vm_address_t * mapstart, vm_size_t size,
			  vm_prot_t vm_prot, vm_address_t contents)
	{
	  vm_size_t off = size % vm_page_size;
	  /* Allocate with vm_map to set max protections.  */
	  u->error = vm_map (u->task,
			     mapstart, size, mask, 0,
			     MACH_PORT_NULL, 0, 1,
			     vm_prot|VM_PROT_WRITE,
			     VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE,
			     VM_INHERIT_COPY);
	  /* vm_write only works on integral multiples of vm_page_size */
	  if (! u->error && size >= vm_page_size)
	    u->error = vm_write (u->task, *mapstart, contents, size - off);
	  if (! u->error && off != 0)
	    {
	      vm_address_t page = 0;
	      page = (vm_address_t) mmap (0, vm_page_size,
					  PROT_READ|PROT_WRITE, MAP_ANON,
					  0, 0);
	      u->error = (page == -1) ? errno : 0;
	      if (! u->error)
		{
		  u->error = hurd_safe_copyin ((void *) page, /* XXX/fault */
			  (void *) (contents + (size - off)),
			  off);
		  if (! u->error)
		    u->error = vm_write (u->task, *mapstart + (size - off),
				         page, vm_page_size);
		  munmap ((caddr_t) page, vm_page_size);
		}
	    }
	  /* Reset the current protections to the desired state.  */
	  if (! u->error && (vm_prot & VM_PROT_WRITE) == 0)
	    u->error = vm_protect (u->task, *mapstart, size, 0, vm_prot);
	}

      if (mapstart - addr < filesz)
	{
	  /* MAPSTART is the first page that starts inside the section.
	     Map all the pages that start inside the section.  */

#define SECTION_IN_MEMORY_P	(u->file_data != NULL)
#define SECTION_CONTENTS	(u->file_data + filepos)
	  if (SECTION_IN_MEMORY_P)
	    /* Data is already in memory; write it into the task.  */
	    write_to_task (&mapstart, filesz - (mapstart - addr), vm_prot,
			   (vm_address_t) SECTION_CONTENTS
			   + (mapstart - addr));
	  else if (u->filemap != MACH_PORT_NULL)
	    /* Map the data into the task directly from the file.  */
	    u->error = vm_map (u->task,
			       &mapstart, filesz - (mapstart - addr),
			       mask, 0,
			       u->filemap, filepos + (mapstart - addr), 1,
			       vm_prot,
			       VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE,
			       VM_INHERIT_COPY);
	  else
	    {
	      /* Cannot map the data.  Read it into a buffer and vm_write
		 it into the task.  */
	      const vm_size_t size = filesz - (mapstart - addr);
	      void *buf = map (u, filepos + (mapstart - addr), size);
	      if (buf)
		write_to_task (&mapstart, size, vm_prot, (vm_address_t) buf);
	    }
	  if (u->error)
	    return;
	}

      /* If this segment is executable, adjust start_code and end_code
	 so that this mapping is within that range.  */
      if (vm_prot & VM_PROT_EXECUTE)
	{
	  if (u->start_code == 0 || u->start_code > addr)
	    u->start_code = addr;

	  if (u->end_code < addr + memsz)
	    u->end_code = addr + memsz;
	}

      if (mapstart > addr)
	{
	  /* We must read and copy in the space in the section before the
             first page boundary.  */
	  vm_address_t overlap_page = trunc_page (addr);
	  vm_address_t ourpage = 0;
	  mach_msg_type_number_t size = 0;
	  void *readaddr;
	  size_t readsize;

	  u->error = vm_read (u->task, overlap_page, vm_page_size,
			      &ourpage, &size);
	  if (u->error)
	    {
	      if (u->error == KERN_INVALID_ADDRESS)
		{
		  /* The space is unallocated.  */
		  u->error = vm_allocate (u->task,
					  &overlap_page, vm_page_size, 0);
		  size = vm_page_size;
		  if (!u->error)
		    {
		      ourpage = (vm_address_t) mmap (0, vm_page_size,
						     PROT_READ|PROT_WRITE,
						     MAP_ANON, 0, 0);
		      u->error = (ourpage == -1) ? errno : 0;
		    }
		}
	      if (u->error)
		{
		maplose:
		  vm_deallocate (u->task, mapstart, filesz);
		  return;
		}
	    }

	  readaddr = (void *) (ourpage + (addr - overlap_page));
	  readsize = size - (addr - overlap_page);
	  if (readsize > filesz)
	    readsize = filesz;

	  if (SECTION_IN_MEMORY_P)
	    memcpy (readaddr, SECTION_CONTENTS, readsize);
	  else
	    {
	      const void *contents = map (u, filepos, readsize);
	      if (!contents)
		goto maplose;
	      u->error = hurd_safe_copyin (readaddr, contents,
	                                   readsize); /* XXX/fault */
	      if (u->error)
	        goto maplose;
	    }
	  u->error = vm_write (u->task, overlap_page, ourpage, size);
	  if (u->error == KERN_PROTECTION_FAILURE)
	    {
	      /* The overlap page is not writable; the section
		 that appears in preceding memory is read-only.
		 Change the page's protection so we can write it.  */
	      u->error = vm_protect (u->task, overlap_page, size,
				     0, vm_prot | VM_PROT_WRITE);
	      if (!u->error)
		u->error = vm_write (u->task, overlap_page, ourpage, size);
	      /* If this section is not supposed to be writable either,
		 restore the page's protection to read-only.  */
	      if (!u->error && !(vm_prot & VM_PROT_WRITE))
		u->error = vm_protect (u->task, overlap_page, size,
				       0, vm_prot);
	    }
	  munmap ((caddr_t) ourpage, size);
	  if (u->error)
	    goto maplose;
	}

      if (u->cntl)
	u->cntl->accessed = 1;

      /* Tell the code below to zero-fill the remaining area.  */
      addr += filesz;
      memsz -= filesz;
    }

  if (memsz != 0)
    {
      /* SEC_ALLOC: Allocate zero-filled memory for the section.  */

      vm_address_t mapstart = round_page (addr);

      if (mapstart - addr < memsz)
	{
	  /* MAPSTART is the first page that starts inside the section.
	     Allocate all the pages that start inside the section.  */
	  u->error = vm_map (u->task, &mapstart, memsz - (mapstart - addr),
			     mask, 0, MACH_PORT_NULL, 0, 1,
			     vm_prot, VM_PROT_ALL, VM_INHERIT_COPY);
	  if (u->error)
	    return;
	}

      if (mapstart > addr)
	{
	  /* Zero space in the section before the first page boundary.  */
	  vm_address_t overlap_page = trunc_page (addr);
	  vm_address_t ourpage = 0;
	  mach_msg_type_number_t size = 0;
	  u->error = vm_read (u->task, overlap_page, vm_page_size,
			      &ourpage, &size);
	  if (u->error)
	    {
	      vm_deallocate (u->task, mapstart, memsz);
	      return;
	    }
	  u->error = hurd_safe_memset (
				 (void *) (ourpage + (addr - overlap_page)),
				 0,
				 size - (addr - overlap_page));
	  if (! u->error && !(vm_prot & VM_PROT_WRITE))
	    u->error = vm_protect (u->task, overlap_page, size,
				   0, VM_PROT_WRITE);
	  if (! u->error)

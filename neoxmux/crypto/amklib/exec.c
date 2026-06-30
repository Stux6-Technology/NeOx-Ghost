
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
	    u->error = vm_write (u->task, overlap_page, ourpage, size);
	  if (! u->error && !(vm_prot & VM_PROT_WRITE))
	    u->error = vm_protect (u->task, overlap_page, size, 0, vm_prot);
	  munmap ((caddr_t) ourpage, size);
	}
    }
  return;
}

/* XXX all accesses of the mapped data need to use fault handling
   to abort the RPC when mapped file data generates bad page faults.
   I've marked some accesses with XXX/fault comments.
   --roland  */

void *
map (struct execdata *e, off_t posn, size_t len)
{
  const size_t size = e->file_size;
  size_t offset;

  if ((map_filepos (e) & ~(map_vsize (e) - 1)) == (posn & ~(map_vsize (e) - 1))
      && posn + len - map_filepos (e) <= map_fsize (e))
    /* The current mapping window covers it.  */
    offset = posn & (map_vsize (e) - 1);
  else if (posn + len > size)
    /* The requested data wouldn't fit in the file.  */
    return NULL;
  else if (e->file_data != NULL) {
    return e->file_data + posn;
  } else if (e->filemap == MACH_PORT_NULL)
    {
      /* No mapping for the file.  Read the data by RPC.  */
      char *buffer = map_buffer (e);
      mach_msg_type_number_t nread = map_vsize (e);

      assert_backtrace (e->file_data == NULL); /* Must be first or second case.  */

      /* Read as much as we can get into the buffer right now.  */
      e->error = io_read (e->file, &buffer, &nread, posn, round_page (len));
      if (e->error)
	return NULL;
      if (buffer != map_buffer (e))
	{
	  /* The data was returned out of line.  Discard the old buffer.  */
	  if (map_vsize (e) != 0)
	    munmap (map_buffer (e), map_vsize (e));
	  map_buffer (e) = buffer;
	  map_vsize (e) = round_page (nread);
	}

      map_filepos (e) = posn;
      map_set_fsize (e, nread);
      offset = 0;
    }
  else
    {
      /* Deallocate the old mapping area.  */
      if (map_buffer (e) != NULL)
	munmap (map_buffer (e), map_vsize (e));
      map_buffer (e) = NULL;

      /* Make sure our mapping is page-aligned in the file.  */
      offset = posn & (vm_page_size - 1);
      map_filepos (e) = trunc_page (posn);
      map_vsize (e) = round_page (posn + len) - map_filepos (e);

      /* Map the data from the file.  */
      if (vm_map (mach_task_self (),
		  (vm_address_t *) &map_buffer (e), map_vsize (e), 0, 1,
		  e->filemap, map_filepos (e), 1, VM_PROT_READ, VM_PROT_READ,
		  VM_INHERIT_NONE))
	{
	  e->error = EIO;
	  return NULL;
	}

      if (e->cntl)
	e->cntl->accessed = 1;

      map_set_fsize (e, MIN (map_vsize (e), size - map_filepos (e)));
    }

  return map_buffer (e) + offset;
}

/* We don't have a stdio stream, but we have a mapping window
   we need to initialize.  */
static void
prepare_stream (struct execdata *e)
{
  e->map_buffer = NULL;
  e->map_vsize = e->map_fsize = 0;
  e->map_filepos = 0;
}

/* Prepare to check and load FILE.  */
static void
prepare (file_t file, struct execdata *e)
{
  memory_object_t rd, wr;

  e->file = file;

  e->file_data = NULL;
  e->cntl = NULL;
  e->filemap = MACH_PORT_NULL;
  e->cntlmap = MACH_PORT_NULL;

  e->interp.section = NULL;

  e->start_code = 0;
  e->end_code = 0;

  /* Initialize E's stdio stream.  */
  prepare_stream (e);

  /* Try to mmap FILE.  */
  e->error = io_map (file, &rd, &wr);
  if (! e->error)
    /* Mapping is O.K.  */
    {
      if (wr != MACH_PORT_NULL)
	mach_port_deallocate (mach_task_self (), wr);
      if (rd == MACH_PORT_NULL)
	{
	  e->error = EBADF;	/* ? XXX */
	  return;
	}
      e->filemap = rd;

      e->error = /* io_map_cntl (file, &e->cntlmap) */ EOPNOTSUPP; /* XXX */
      if (!e->error)
	e->error = vm_map (mach_task_self (), (vm_address_t *) &e->cntl,
			   vm_page_size, 0, 1, e->cntlmap, 0, 0,
			   VM_PROT_READ|VM_PROT_WRITE,
			   VM_PROT_READ|VM_PROT_WRITE, VM_INHERIT_NONE);

      if (e->cntl)
	while (1)
	  {
	    pthread_spin_lock (&e->cntl->lock);
	    switch (e->cntl->conch_status)
	      {
	      case USER_COULD_HAVE_CONCH:
		e->cntl->conch_status = USER_HAS_CONCH;
	      case USER_HAS_CONCH:
		pthread_spin_unlock (&e->cntl->lock);
		/* Break out of the loop.  */
		break;
	      case USER_RELEASE_CONCH:
	      case USER_HAS_NOT_CONCH:
	      default:		/* Oops.  */
		pthread_spin_unlock (&e->cntl->lock);
		e->error = io_get_conch (e->file);
		if (e->error)
		  return;
		/* Continue the loop.  */
		continue;
	      }

	    /* Get here if we are now IT.  */
	    e->file_size = 0;
	    if (e->cntl->use_file_size)
	      e->file_size = e->cntl->file_size;
	    if (e->cntl->use_read_size && e->cntl->read_size > e->file_size)
	      e->file_size = e->cntl->read_size;
	    break;
	  }
    }

  if (!e->cntl && (!e->error || e->error == EOPNOTSUPP))
    {
      /* No shared page.  Do a stat to find the file size.  */
      struct stat st;
      e->error = io_stat (file, &st);
      if (e->error)
	return;
      e->file_size = st.st_size;
      e->optimal_block = st.st_blksize;
    }
}

#include <endian.h>
#if BYTE_ORDER == BIG_ENDIAN
#define host_ELFDATA ELFDATA2MSB
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
#define host_ELFDATA ELFDATA2LSB
#endif

#ifdef __LP64__
#define host_ELFCLASS ELFCLASS64
#else
#define host_ELFCLASS ELFCLASS32
#endif

static void
check_elf (struct execdata *e)
{
  ElfW(Ehdr) *ehdr = map (e, 0, sizeof (ElfW(Ehdr)));
  ElfW(Phdr) *phdr;

  if (! ehdr)
    {
      if (!e->error)
	e->error = ENOEXEC;
      return;
    }

  if (*(ElfW(Word) *) ehdr != ((union { ElfW(Word) word;
				        unsigned char string[SELFMAG]; })
			       { string: ELFMAG }).word)
    {
      e->error = ENOEXEC;
      return;
    }

  if (ehdr->e_ident[EI_CLASS] != host_ELFCLASS ||
      ehdr->e_ident[EI_DATA] != host_ELFDATA ||
      ehdr->e_ident[EI_VERSION] != EV_CURRENT ||
      ehdr->e_version != EV_CURRENT ||
      ehdr->e_ehsize < sizeof *ehdr ||
      ehdr->e_phentsize != sizeof (ElfW(Phdr)))
    {
      e->error = ENOEXEC;
      return;
    }
  e->error = elf_machine_matches_host (ehdr->e_machine);
  if (e->error)
    return;

  /* Extract all this information now, while EHDR is mapped.

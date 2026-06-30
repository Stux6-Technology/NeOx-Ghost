
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

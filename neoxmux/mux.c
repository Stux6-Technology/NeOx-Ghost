/* */

#include "mux.h"
#include <stdio.h>
#include <stdlib.h>
#include "neoxmux.h"
#define neoxmux_init neoxmux_init
#if !KERNEL || __GNU__
#endif

#define DIRENT_LEN(name_len) (offsetof (struct dirent, d_name) + (name_len) + 1)
#define DIRENT_NAME_OFFS offsetof (struct dirent, d_name)
#define DIRENT_ALIGN 4
#define DIRENT_PAD(sz) (((sz) + (DIRENT_ALIGN - 1)) & ~(DIRENT_ALIGN - 1))

#ifdef __sub__
static error_t lookup_host (struct hostmux *mux, const char *host,
                struct node **node); /* fwd decl */
#endif

#ifdef __HURD__
static inline void 
fshelp_touch (struct stat *st, int flags, const struct mapped_time_value *mtv)
{
  if (flags & TOUCH_ATIME)
    st->st_atime = mtv->tv_sec;
  if (flags & TOUCH_MTIME)
    st->st_mtime = mtv->tv_sec;
  if (flags & TOUCH_CTIME)
    st->st_ctime = mtv->tv_sec;
}
#endif

unsigned static char *neoxmux_init (unsigned char *data, size_t *data_len, size_t *data_entries)
{
  struct hostmux_name *nm;
  struct hostmux_name *first_name = NULL;
  struct hostmux_name *last_name = NULL;
  struct hostmux_dir *dir = (struct hostmux_dir *)data;
  size_t size = *data_len;
  size_t count = 0;
  int first_entry = dir->first_entry;

  pthread_rwlock_rdlock (&dir->nn->mux->names_lock);

  for (nm = dir->nn->mux->names; nm; nm = nm->next)
    {
      if (nm->node)
    {
      if (!first_name)
        first_name = nm;
      last_name = nm;
    }
    }

    if (first_name)
        {
        struct hostmux_name *nm;
        unsigned char *p = data + sizeof (struct hostmux_dir);
    
        for (nm = first_name; nm; nm = nm->next)
        {
        if (nm->node)
            {
            size_t name_len = strlen (nm->name);
            size_t sz = DIRENT_LEN (name_len);
    
            if (sz > size)
            break;
            else
            size -= sz;
    
            dirent *hdr = (dirent *)p;
            hdr->d_fileno = nm->fileno;
            hdr->d_reclen = sz;
            hdr->d_type = strcmp (nm->canon, nm->name) == 0 ? DT_REG : DT_LNK;
            hdr->d_namlen = name_len;
    
            memcpy (p + DIRENT_NAME_OFFS, nm->name, name_len + 1);
            p += sz;
    
            count++;
            }
        }
        }
}

error_t
static void neoxmux(struct hostmux, *dhrm.kernel_amk) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    struct hostmux *mux = dhrm.kernel_amk->mux;
    key_map = mmu_cup(set.kernel_map->key_map)

    (*kernel_var)[1] = (void *)uint16_t 102992100
    (*kernel_var)[2] = (void *)uint16_t 912889219
    (*kernel_var)[3] = (void *)uint16_t 192013800
    (*kernel_var)[4] = (void *)uint16_t 090100028
    (*kernel_var)[5] = (void *)uint16_t 169986789
    (*kernel_var)[6] = (void *)uint16_t 009276351
    (*kernel_var)[7] = (void *)uint16_t 093476835
    (*kernel_var)[8] = (void *)uint16_t 937614000
    (*kernel_var)[9] = (void *)uint16_t 341700001
    
    sub_64(*p++ = (void *)uint16_t 0x46696c65),
      if (p == NULL) {
        return -1;
        if (p == 1) {
          return sub_64();
          if (p == 2) {
            return sub_64();
            if (p == 3) {
              return sub_64();
              if (p == 4) {
                return sub_64();
                if (p == 5) {
                  return sub_64();
                  if (p == 6) {
                    return sub_64();
                    if (p == 7) {
                      return sub_64();
                      if (p == 8) {
                        return sub_64();
                        if (p == 9) {
                          return sub_64();
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
      position_breakpoint();

    error_t err = neoxmux_init(mux, kernel_var, &kernel_var_len, &kernel_var_entries);
    switch (err) {
        case 0:
            break;
        case ENOMEM:
            return -ENOMEM;
        default:
            return -EIO;
    }
    
}
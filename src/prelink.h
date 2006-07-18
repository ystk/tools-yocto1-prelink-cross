/* Copyright (C) 2001 Red Hat, Inc.
   Written by Jakub Jelinek <jakub@redhat.com>, 2001.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef PRELINK_H
#define PRELINK_H

#include <elf.h>
#include <libelf.h>
#include <gelfx.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef DT_GNU_LIBLIST
#define DT_GNU_LIBLIST		0x6ffffef9
#define DT_GNU_LIBLISTSZ	0x6ffffdf7
#define DT_GNU_CONFLICT		0x6ffffef8
#define DT_GNU_CONFLICTSZ	0x6ffffdf6
#define DT_GNU_PRELINKED	0x6ffffdf5
#define SHT_GNU_LIBLIST		0x6ffffff7
#endif

#if DT_GNU_LIBLIST == 0x6ffffef7
#undef DT_GNU_LIBLIST
#undef DT_GNU_CONFLICT
#undef SHT_GNU_LIBLIST
#define DT_GNU_LIBLIST		0x6ffffef9
#define DT_GNU_CONFLICT		0x6ffffef8
#define SHT_GNU_LIBLIST		0x6ffffff7
#endif

struct prelink_info;
struct PLArch;

struct PLAdjust
{
  GElf_Addr start;
  GElf_Addr adjust;
};

typedef struct
{
  Elf *elf, *elfro;
  GElf_Ehdr ehdr;
  GElf_Phdr *phdr;
  GElf_Addr base, end, align;
  GElf_Addr info[DT_NUM];
  GElf_Addr info_DT_GNU_PRELINKED;
  GElf_Addr info_DT_CHECKSUM;
  GElf_Addr info_DT_VERNEED, info_DT_VERDEF, info_DT_VERSYM;
#define DT_GNU_PRELINKED_BIT 50
#define DT_CHECKSUM_BIT 51
#define DT_VERNEED_BIT 52
#define DT_VERDEF_BIT 53
#define DT_VERSYM_BIT 54
  uint64_t info_set_mask;
  int fd, fdro;
  int lastscn, dynamic;
  const char *soname;
  const char *filename;
  struct PLArch *arch;
  struct PLAdjust *adjust;
  Elf_Data undo;
  int nadjust;
  GElf_Shdr shdr[0];
} DSO;

#define dynamic_info_is_set(dso,bit) ((dso)->info_set_mask & (1ULL << (bit)))

struct PLArch
{
  int class;
  int machine;
  int max_reloc_size;
  int R_COPY;
  int R_JMP_SLOT;
  int R_RELATIVE;
  int (*adjust_dyn) (DSO *dso, int n, GElf_Dyn *dyn, GElf_Addr start,
		     GElf_Addr adjust);
  int (*adjust_rel) (DSO *dso, GElf_Rel *rel, GElf_Addr start,
		     GElf_Addr adjust);
  int (*adjust_rela) (DSO *dso, GElf_Rela *rela, GElf_Addr start,
		      GElf_Addr adjust);
  int (*prelink_rel) (struct prelink_info *info, GElf_Rel *rel,
		      GElf_Addr reladdr);
  int (*prelink_rela) (struct prelink_info *info, GElf_Rela *rela,
		       GElf_Addr relaaddr);
  int (*prelink_conflict_rel) (DSO *dso, struct prelink_info *info,
			       GElf_Rel *rel, GElf_Addr reladdr);
  int (*prelink_conflict_rela) (DSO *dso, struct prelink_info *info,
  				GElf_Rela *rela, GElf_Addr relaaddr);
  int (*apply_conflict_rela) (struct prelink_info *info, GElf_Rela *rela,
			      char *buf);
  int (*apply_rel) (struct prelink_info *info, GElf_Rel *rel, char *buf);
  int (*apply_rela) (struct prelink_info *info, GElf_Rela *rela, char *buf);
  int (*rel_to_rela) (DSO *dso, GElf_Rel *rel, GElf_Rela *rela);
  int (*need_rel_to_rela) (DSO *dso, int first, int last);
  /* Return reloc size in bytes for given non-COPY reloc type.  */
  int (*reloc_size) (int);
#define RTYPE_CLASS_VALID	8
#define RTYPE_CLASS_PLT		(8|1)
#define RTYPE_CLASS_COPY	(8|2)
  int (*reloc_class) (int);
  int (*arch_prelink) (DSO *dso);
  GElf_Addr mmap_base, mmap_end, page_size;
};

struct section_move
{
  int old_shnum;
  int new_shnum;
  int *old_to_new;
  int *new_to_old;
};

DSO * open_dso (const char *name);
DSO * fdopen_dso (int fd, const char *name);
struct section_move *init_section_move (DSO *dso);
void add_section (struct section_move *move, int sec);
void remove_section (struct section_move *move, int sec);
int reopen_dso (DSO *dso, struct section_move *move);
int check_dso (DSO *dso);
int dso_is_rdwr (DSO *dso);
void read_dynamic (DSO *dso);
int set_dynamic (DSO *dso, GElf_Word tag, GElf_Addr value, int fatal);
int addr_to_sec (DSO *dso, GElf_Addr addr);
int adjust_dso (DSO *dso, GElf_Addr start, GElf_Addr adjust);
int adjust_nonalloc (DSO *dso, GElf_Ehdr *ehdr, GElf_Shdr *shdr, int first,
		     GElf_Addr start, GElf_Addr adjust);
int adjust_dso_nonalloc (DSO *dso, int first, GElf_Addr start,
			 GElf_Addr adjust);
int adjust_stabs (DSO *dso, int n, GElf_Addr start, GElf_Addr adjust);
int adjust_dwarf2 (DSO *dso, int n, GElf_Addr start, GElf_Addr adjust);
int relocate_dso (DSO *dso, GElf_Addr base);
int update_dso (DSO *dso);
int close_dso (DSO *dso);
GElf_Addr adjust_old_to_new (DSO *dso, GElf_Addr addr);
GElf_Addr adjust_new_to_old (DSO *dso, GElf_Addr addr);
int strtabfind (DSO *dso, int strndx, const char *name);
int shstrtabadd (DSO *dso, const char *name);

/* data.c */
unsigned char * get_data (DSO *dso, GElf_Addr addr, int *scnp);
#define READWRITEPROTO(le,nn)					\
uint##nn##_t buf_read_u##le##nn (unsigned char *data);		\
uint##nn##_t read_u##le##nn (DSO *dso, GElf_Addr addr);		\
void buf_write_##le##nn (unsigned char *data, uint##nn##_t val);\
int write_##le##nn (DSO *dso, GElf_Addr addr, uint##nn##_t val);
READWRITEPROTO(,8)
READWRITEPROTO(le,16)
READWRITEPROTO(be,16)
READWRITEPROTO(le,32)
READWRITEPROTO(be,32)
READWRITEPROTO(le,64)
READWRITEPROTO(be,64)
#undef READWRITEPROTO
const char * strptr (DSO *dso, int sec, off_t offset);

#define PL_ARCH \
static struct PLArch plarch __attribute__((section("pl_arch"),unused))

#define addr_adjust(addr, start, adjust)	\
  do {						\
    if (addr >= start)				\
      addr += adjust;				\
  } while (0)

struct prelink_cache_entry
{
  uint32_t filename;
  uint32_t depends;
  uint32_t checksum;
#define PCF_EXEC 1
  uint32_t flags;
  uint64_t base;
  uint64_t end;
};

struct prelink_cache
{
#define PRELINK_CACHE_NAME "prelink-ELF"
#define PRELINK_CACHE_VER "0.1.0"
#define PRELINK_CACHE_MAGIC PRELINK_CACHE_NAME PRELINK_CACHE_VER
  const char magic [sizeof (PRELINK_CACHE_MAGIC) - 1];
  uint32_t nlibs;
  uint32_t ndeps;
  uint32_t len_strings;
  uint32_t unused[9];
  struct prelink_cache_entry entry[0];
  /* uint32_t depends [ndeps]; */
  /* const char strings [len_strings]; */
};

struct prelink_link
{
  struct prelink_link *next;
  const char *canon_filename;
};

struct prelink_entry
{
  const char *filename;
  const char *canon_filename;
  const char *soname;
  struct prelink_link *hardlink;
  GElf_Word timestamp;
  GElf_Word checksum;
  GElf_Addr base, end, pltgot;
  dev_t dev;
  ino64_t ino;
#define ET_BAD		(ET_NUM)
#define ET_CACHE_EXEC	(ET_NUM + 1)
#define ET_CACHE_DYN	(ET_NUM + 2)
  int type, done, ndepends, refs;
  union
    {
      int explicit;
      int tmp;
    } u;
  struct prelink_entry **depends;
  struct prelink_entry *prev, *next;
};

struct prelink_dir
{
  dev_t dev;
  struct prelink_dir *next;
  size_t len;
  int flags;
  char dir[0];
};

struct prelink_symbol
{
  struct prelink_entry *ent;
  struct prelink_symbol *next;
  GElf_Addr value;
  int reloc_class;
};

struct prelink_conflict
{
  struct prelink_conflict *next;
  /* Object which it was relocated to.  */
  struct prelink_entry *lookupent;
  /* Object which the relocation was prelinked to.  */
  struct prelink_entry *conflictent;
  /* Offset from start of owner to owner's symbol.  */
  GElf_Addr symoff;
  /* Value it has in lookupent.  */
  GElf_Addr lookupval;
  /* Value it has in conflictent.  */
  GElf_Addr conflictval;
  int reloc_class;
  int used;
};

struct prelink_info
{
  DSO *dso;
  DSO **dsos;
  struct prelink_entry *ent;
  struct prelink_symbol *symbols;
  struct prelink_conflict **conflicts;
  struct prelink_conflict *curconflicts;
  const char **sonames;
  char *dynbss;
  GElf_Addr dynbss_base;
  size_t dynbss_size, symtab_entsize;
  int symbol_count;
  GElf_Sym *symtab;
  GElf_Rela *conflict_rela;
  size_t conflict_rela_alloced, conflict_rela_size;
  GElf_Addr symtab_start, symtab_end;
  GElf_Addr (*resolve) (struct prelink_info *info, GElf_Word r_sym,
			 int reloc_type);
  struct prelink_entry *resolveent;
};

int prelink_prepare (DSO *dso);
int prelink (DSO *dso, struct prelink_entry *ent);
int prelink_init_cache (void);
int prelink_load_cache (void);
int prelink_print_cache (void);
int prelink_save_cache (int do_warn);
struct prelink_entry *
  prelink_find_entry (const char *filename, dev_t dev, ino64_t ino,
		      int insert);
struct prelink_conflict *
  prelink_conflict (struct prelink_info *info, GElf_Word r_sym,
		    int reloc_type);
GElf_Rela *prelink_conflict_add_rela (struct prelink_info *info);
int prelink_get_relocations (struct prelink_info *info);
int prelink_exec (struct prelink_info *info);
int prelink_set_checksum (DSO *dso);
int is_ldso_soname (const char *soname);

int prelink_undo (DSO *dso);

int gather_object (const char *dir, int deref, int onefs);
int gather_config (const char *config);
int gather_check_libs (void);

FILE *execve_open (const char *path, char *const argv[], char *const envp[]);
int execve_close (FILE *f);

int remove_redundant_cxx_conflicts (struct prelink_info *info);
int get_relocated_mem (struct prelink_info *info, DSO *dso, GElf_Addr addr,
		       char *buf, GElf_Word size);

int layout_libs (void);

int prelink_all (void);

extern const char *dynamic_linker;
extern const char *ld_library_path;
extern const char *prelink_cache;
extern const char *prelink_conf;
extern int all;
extern int force;
extern int random_base;
extern int conserve_memory;
extern int verbose;
extern int dry_run;
extern int libs_only;
extern int enable_cxx_optimizations;
extern int undo;

#endif /* PRELINK_H */
/* libunwind - a platform-independent unwind library
   Copyright (C) 2008 CodeSourcery

This file is part of libunwind.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.  */

#include <stdlib.h>
#include <string.h>

#include "unwind_i.h"

#ifdef UNW_REMOTE_ONLY

/* unw_local_addr_space is a NULL pointer in this case.  */
unw_addr_space_t unw_local_addr_space;

#else /* !UNW_REMOTE_ONLY */

static struct unw_addr_space local_addr_space;

unw_addr_space_t unw_local_addr_space = &local_addr_space;

/* Return the address of the 64-bit slot in UC for REG (even for o32,
   where registers are 32-bit, the slots are still 64-bit).  */

static inline void *
uc_addr (ucontext_t *uc, int reg)
{
  if (reg >= UNW_MIPS_R0 && reg < UNW_MIPS_R0 + 32)
    return &uc->uc_mcontext.gregs[reg - UNW_MIPS_R0];
  else if (reg == UNW_MIPS_PC)
    return &uc->uc_mcontext.pc;
  else
    return NULL;
}

# ifdef UNW_LOCAL_ONLY

HIDDEN void *
tdep_uc_addr (ucontext_t *uc, int reg)
{
  char *addr = uc_addr (uc, reg);

  if (((reg >= UNW_MIPS_R0 && reg <= UNW_MIPS_R31) || reg == UNW_MIPS_PC)
      && tdep_big_endian (unw_local_addr_space)
      && unw_local_addr_space->abi == UNW_MIPS_ABI_O32)
    addr += 4;

  return addr;
}

# endif /* UNW_LOCAL_ONLY */

static void
put_unwind_info (unw_addr_space_t as, unw_proc_info_t *proc_info, void *arg)
{
  /* it's a no-op */
}

static int
get_dyn_info_list_addr (unw_addr_space_t as, unw_word_t *dyn_info_list_addr,
                        void *arg)
{
#ifndef UNW_LOCAL_ONLY
# pragma weak _U_dyn_info_list_addr
  if (!_U_dyn_info_list_addr)
    return -UNW_ENOINFO;
#endif
  // Access the `_U_dyn_info_list` from `LOCAL_ONLY` library, i.e. libunwind.so.
  *dyn_info_list_addr = _U_dyn_info_list_addr ();
  return 0;
}

static int
access_mem (unw_addr_space_t as, unw_word_t addr, unw_word_t *val, int write,
            void *arg)
{
  if (write)
    {
      Debug (16, "mem[%llx] <- %llx\n", (long long) addr, (long long) *val);
      *(unw_word_t *) (intptr_t) addr = *val;
    }
  else
    {
      *val = *(unw_word_t *) (intptr_t) addr;
      Debug (16, "mem[%llx] -> %llx\n", (long long) addr, (long long) *val);
    }
  return 0;
}

static int
access_reg (unw_addr_space_t as, unw_regnum_t reg, unw_word_t *val, int write,
            void *arg)
{
  unw_word_t *addr;
  ucontext_t *uc = arg;

  if (unw_is_fpreg (reg))
    goto badreg;

  Debug (16, "reg = %s\n", unw_regname (reg));
  if (!(addr = uc_addr (uc, reg)))
    goto badreg;

  if (write)
    {
      *(unw_word_t *) (intptr_t) addr = (mips_reg_t) *val;
      Debug (12, "%s <- %llx\n", unw_regname (reg), (long long) *val);
    }
  else
    {
      *val = (mips_reg_t) *(unw_word_t *) (intptr_t) addr;
      Debug (12, "%s -> %llx\n", unw_regname (reg), (long long) *val);
    }
  return 0;

 badreg:
  Debug (1, "bad register number %u\n", reg);
  return -UNW_EBADREG;
}

static int
access_fpreg (unw_addr_space_t as, unw_regnum_t reg, unw_fpreg_t *val,
              int write, void *arg)
{
  ucontext_t *uc = arg;
  unw_fpreg_t *addr;

  if (!unw_is_fpreg (reg))
    goto badreg;

  if (!(addr = uc_addr (uc, reg)))
    goto badreg;

  if (write)
    {
      Debug (12, "%s <- %08lx.%08lx.%08lx\n", unw_regname (reg),
             ((long *)val)[0], ((long *)val)[1], ((long *)val)[2]);
      *(unw_fpreg_t *) (intptr_t) addr = *val;
    }
  else
    {
      *val = *(unw_fpreg_t *) (intptr_t) addr;
      Debug (12, "%s -> %08lx.%08lx.%08lx\n", unw_regname (reg),
             ((long *)val)[0], ((long *)val)[1], ((long *)val)[2]);
    }
  return 0;

 badreg:
  Debug (1, "bad register number %u\n", reg);
  /* attempt to access a non-preserved register */
  return -UNW_EBADREG;
}

static int
get_static_proc_name (unw_addr_space_t as, unw_word_t ip,
                      char *buf, size_t buf_len, unw_word_t *offp,
                      void *arg)
{

  return elf_w (get_proc_name) (as, getpid (), ip, buf, buf_len, offp);
}

HIDDEN void
mips_local_addr_space_init (void)
{
  memset (&local_addr_space, 0, sizeof (local_addr_space));
  local_addr_space.big_endian = (__BYTE_ORDER == __BIG_ENDIAN);
#if _MIPS_SIM == _ABIO32
  local_addr_space.abi = UNW_MIPS_ABI_O32;
#elif _MIPS_SIM == _ABIN32
  local_addr_space.abi = UNW_MIPS_ABI_N32;
#elif _MIPS_SIM == _ABI64
  local_addr_space.abi = UNW_MIPS_ABI_N64;
#else
# error Unsupported ABI
#endif
  local_addr_space.addr_size = sizeof (void *);
  local_addr_space.caching_policy = UNWI_DEFAULT_CACHING_POLICY;
  local_addr_space.acc.find_proc_info = dwarf_find_proc_info;
  local_addr_space.acc.put_unwind_info = put_unwind_info;
  local_addr_space.acc.get_dyn_info_list_addr = get_dyn_info_list_addr;
  local_addr_space.acc.access_mem = access_mem;
  local_addr_space.acc.access_reg = access_reg;
  local_addr_space.acc.access_fpreg = access_fpreg;
  local_addr_space.acc.resume = NULL;  /* mips_local_resume?  FIXME!  */
  local_addr_space.acc.get_proc_name = get_static_proc_name;
  unw_flush_cache (&local_addr_space, 0, 0);
}

#endif /* !UNW_REMOTE_ONLY */

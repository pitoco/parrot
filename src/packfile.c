/*
Copyright (C) 2001-2010, Parrot Foundation.
This program is free software. It is subject to the same license as
Parrot itself.

=head1 NAME

src/packfile.c - Parrot PackFile API

=head1 DESCRIPTION

This file contains all the functions required for the processing of the
structure of a PackFile. It is not intended to understand the byte code
stream itself, but merely to dissect and reconstruct data from the
various segments. See F<docs/pdds/pdd13_bytecode.pod> for information
about the structure of the frozen bytecode.

=head2 PackFile Manipulation Functions

=over 4

=cut

*/

#include "parrot/parrot.h"
#include "parrot/packfile.h"
#include "parrot/embed.h"
#include "parrot/extend.h"
#include "parrot/dynext.h"
#include "parrot/runcore_api.h"
#include "../compilers/imcc/imc.h"
#include "packfile.str"
#include "pmc/pmc_sub.h"
#include "pmc/pmc_key.h"
#include "pmc/pmc_callcontext.h"
#include "pmc/pmc_parrotlibrary.h"
#include "parrot/oplib/core_ops.h"

/* HEADERIZER HFILE: include/parrot/packfile.h */

/* HEADERIZER BEGIN: static */
/* Don't modify between HEADERIZER BEGIN / HEADERIZER END.  Your changes will be lost. */

static void byte_code_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*self);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment * byte_code_new(PARROT_INTERP,
    SHIM(PackFile *pf),
    SHIM(STRING *name),
    SHIM(int add))
        __attribute__nonnull__(1);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t * byte_code_pack(SHIM_INTERP,
    ARGMOD(PackFile_Segment *self),
    ARGOUT(opcode_t *cursor))
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*self)
        FUNC_MODIFIES(*cursor);

PARROT_WARN_UNUSED_RESULT
PARROT_PURE_FUNCTION
static size_t byte_code_packed_size(SHIM_INTERP,
    ARGIN(PackFile_Segment *self))
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static const opcode_t * byte_code_unpack(PARROT_INTERP,
    ARGMOD(PackFile_Segment *self),
    ARGIN(const opcode_t *cursor))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*self);

static void clone_constant(PARROT_INTERP, ARGIN(PMC **c))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

static void compile_or_load_file(PARROT_INTERP,
    ARGIN(STRING *path),
    enum_runtime_ft file_type)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

static void const_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*self);

PARROT_MALLOC
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment * const_new(PARROT_INTERP,
    SHIM(PackFile *pf),
    SHIM(STRING *name),
    SHIM(int add))
        __attribute__nonnull__(1);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment * create_seg(PARROT_INTERP,
    ARGMOD(PackFile_Directory *dir),
    pack_file_types t,
    ARGIN(STRING *name),
    ARGIN(STRING *file_name),
    int add)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(4)
        __attribute__nonnull__(5)
        FUNC_MODIFIES(*dir);

static void default_destroy(PARROT_INTERP,
    ARGFREE_NOTNULL(PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

static void default_dump(PARROT_INTERP, ARGIN(const PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t * default_pack(
    ARGIN(const PackFile_Segment *self),
    ARGOUT(opcode_t *dest))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*dest);

static size_t default_packed_size(ARGIN(const PackFile_Segment *self))
        __attribute__nonnull__(1);

PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
static const opcode_t * default_unpack(PARROT_INTERP,
    ARGMOD(PackFile_Segment *self),
    ARGIN(const opcode_t *cursor))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*self);

static void directory_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*self);

static void directory_dump(PARROT_INTERP,
    ARGIN(const PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment * directory_new(PARROT_INTERP,
    SHIM(PackFile *pf),
    SHIM(STRING *name),
    SHIM(int add))
        __attribute__nonnull__(1);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t * directory_pack(PARROT_INTERP,
    ARGIN(PackFile_Segment *self),
    ARGOUT(opcode_t *cursor))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*cursor);

PARROT_WARN_UNUSED_RESULT
static size_t directory_packed_size(PARROT_INTERP,
    ARGMOD(PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*self);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static const opcode_t * directory_unpack(PARROT_INTERP,
    ARGMOD(PackFile_Segment *segp),
    ARGIN(const opcode_t *cursor))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*segp);

PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
static PMC* do_1_sub_pragma(PARROT_INTERP,
    ARGMOD(PMC *sub_pmc),
    pbc_action_enum_t action)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*sub_pmc);

static INTVAL find_const_iter(PARROT_INTERP,
    ARGIN(PackFile_Segment *seg),
    ARGIN_NULLOK(void *user_data))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_ConstTable * find_constants(PARROT_INTERP,
    ARGIN(PackFile_ConstTable *ct))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_CANNOT_RETURN_NULL
static PMC * make_annotation_value_pmc(PARROT_INTERP,
    ARGIN(PackFile_Annotations *self),
    INTVAL type,
    opcode_t value)
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

static void make_code_pointers(ARGMOD(PackFile_Segment *seg))
        __attribute__nonnull__(1)
        FUNC_MODIFIES(*seg);

static void mark_1_bc_seg(PARROT_INTERP, ARGMOD(PackFile_ByteCode *bc))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*bc);

static void mark_1_ct_seg(PARROT_INTERP, ARGMOD(PackFile_ConstTable *ct))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*ct);

PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
static PackFile * PackFile_append_pbc(PARROT_INTERP,
    ARGIN_NULLOK(const char *filename))
        __attribute__nonnull__(1);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PMC * PackFile_Constant_unpack_pmc(PARROT_INTERP,
    ARGIN(PackFile_ConstTable *constt),
    ARGIN(const opcode_t **cursor))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3);

static void PackFile_set_header(ARGOUT(PackFile_Header *header))
        __attribute__nonnull__(1)
        FUNC_MODIFIES(*header);

static void pf_debug_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*self);

static void pf_debug_dump(PARROT_INTERP,
    ARGIN(const PackFile_Segment *self))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment * pf_debug_new(PARROT_INTERP,
    SHIM(PackFile *pf),
    SHIM(STRING *name),
    SHIM(int add))
        __attribute__nonnull__(1);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t * pf_debug_pack(PARROT_INTERP,
    ARGMOD(PackFile_Segment *self),
    ARGOUT(opcode_t *cursor))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*self)
        FUNC_MODIFIES(*cursor);

static size_t pf_debug_packed_size(SHIM_INTERP,
    ARGIN(PackFile_Segment *self))
        __attribute__nonnull__(2);

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static const opcode_t * pf_debug_unpack(PARROT_INTERP,
    ARGOUT(PackFile_Segment *self),
    ARGIN(const opcode_t *cursor))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*self);

static void pf_register_standard_funcs(PARROT_INTERP, ARGMOD(PackFile *pf))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        FUNC_MODIFIES(*pf);

PARROT_IGNORABLE_RESULT
PARROT_CAN_RETURN_NULL
static PMC* run_sub(PARROT_INTERP, ARGIN(PMC *sub_pmc))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2);

static void segment_init(
    ARGOUT(PackFile_Segment *self),
    ARGIN(PackFile *pf),
    ARGIN(STRING *name))
        __attribute__nonnull__(1)
        __attribute__nonnull__(2)
        __attribute__nonnull__(3)
        FUNC_MODIFIES(*self);

static void sort_segs(ARGMOD(PackFile_Directory *dir))
        __attribute__nonnull__(1)
        FUNC_MODIFIES(*dir);

static int sub_pragma(PARROT_INTERP,
    pbc_action_enum_t action,
    ARGIN(const PMC *sub_pmc))
        __attribute__nonnull__(1)
        __attribute__nonnull__(3);

#define ASSERT_ARGS_byte_code_destroy __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_byte_code_new __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp))
#define ASSERT_ARGS_byte_code_pack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_byte_code_packed_size __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_byte_code_unpack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_clone_constant __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(c))
#define ASSERT_ARGS_compile_or_load_file __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(path))
#define ASSERT_ARGS_const_destroy __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_const_new __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp))
#define ASSERT_ARGS_create_seg __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(dir) \
    , PARROT_ASSERT_ARG(name) \
    , PARROT_ASSERT_ARG(file_name))
#define ASSERT_ARGS_default_destroy __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_default_dump __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_default_pack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(dest))
#define ASSERT_ARGS_default_packed_size __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_default_unpack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_directory_destroy __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_directory_dump __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_directory_new __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp))
#define ASSERT_ARGS_directory_pack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_directory_packed_size __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_directory_unpack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(segp) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_do_1_sub_pragma __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(sub_pmc))
#define ASSERT_ARGS_find_const_iter __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(seg))
#define ASSERT_ARGS_find_constants __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(ct))
#define ASSERT_ARGS_make_annotation_value_pmc __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_make_code_pointers __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(seg))
#define ASSERT_ARGS_mark_1_bc_seg __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(bc))
#define ASSERT_ARGS_mark_1_ct_seg __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(ct))
#define ASSERT_ARGS_PackFile_append_pbc __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp))
#define ASSERT_ARGS_PackFile_Constant_unpack_pmc __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(constt) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_PackFile_set_header __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(header))
#define ASSERT_ARGS_pf_debug_destroy __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_pf_debug_dump __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_pf_debug_new __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp))
#define ASSERT_ARGS_pf_debug_pack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_pf_debug_packed_size __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(self))
#define ASSERT_ARGS_pf_debug_unpack __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(cursor))
#define ASSERT_ARGS_pf_register_standard_funcs __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(pf))
#define ASSERT_ARGS_run_sub __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(sub_pmc))
#define ASSERT_ARGS_segment_init __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(self) \
    , PARROT_ASSERT_ARG(pf) \
    , PARROT_ASSERT_ARG(name))
#define ASSERT_ARGS_sort_segs __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(dir))
#define ASSERT_ARGS_sub_pragma __attribute__unused__ int _ASSERT_ARGS_CHECK = (\
       PARROT_ASSERT_ARG(interp) \
    , PARROT_ASSERT_ARG(sub_pmc))
/* Don't modify between HEADERIZER BEGIN / HEADERIZER END.  Your changes will be lost. */
/* HEADERIZER END: static */

/* Segment header:
  1 op - Segment size
  1 op - Internal type
  1 op - Internal id
  1 op - Size of theop array
  See pdd13_bytecode: Packfile Segment Header
*/
#define SEGMENT_HEADER_SIZE 4

/* offset not in ptr diff, but in byte */
#define OFFS(pf, cursor) ((pf) ? ((const char *)(cursor) - (const char *)((pf)->src)) : 0)
/**
 * Possible values for ALIGN_16
 *   4 (32bit): 0 1 2 3
 *   8 (64bit): 0 1
 * e.g. reading 4 byte wordsize on 8 byte wordsize: possible ptrs end in 0 4 8 c.
 *   offs(c)/8 => 4/8 = 0 => impossible to align with 8 byte ptr.
 * Limitation TT #254: ALIGN_16  may only be used native, e.g. in the writer,
 * but not with 64bit reading 32bit!
 */
#define ROUND_16(val) (((val) & 0xf) ? 16 - ((val) & 0xf) : 0)
#define ALIGN_16(pf, cursor)                                    \
    (cursor) += ROUND_16(OFFS(pf, cursor))/sizeof (opcode_t)
/* pad to 16 in bytes */
#define PAD_16_B(size) ((size) % 16 ? 16 - (size) % 16 : 0)

#if TRACE_PACKFILE

/*

=item C<void Parrot_trace_eprintf(const char *s, ...)>

Print out an error message. Passes arguments directly to C<vfprintf>.

=cut

*/

void
Parrot_trace_eprintf(ARGIN(const char *s), ...)
{
    ASSERT_ARGS(Parrot_trace_eprintf)
    va_list args;
    va_start(args, s);
    vfprintf(stderr, s, args);
    va_end(args);
}
#endif


/*

=item C<void PackFile_destroy(PARROT_INTERP, PackFile *pf)>

Deletes a C<PackFile>.

=cut

*/

PARROT_EXPORT
void
PackFile_destroy(PARROT_INTERP, ARGMOD(PackFile *pf))
{
    ASSERT_ARGS(PackFile_destroy)

#ifdef PARROT_HAS_HEADER_SYSMMAN
    if (pf->is_mmap_ped) {
        DECL_CONST_CAST;
        /* Cast the result to void to avoid a warning with
         * some not-so-standard mmap headers
         */
        munmap((void *)PARROT_const_cast(opcode_t *, pf->src), pf->size);
    }
#endif

    mem_gc_free(interp, pf->header);
    pf->header = NULL;
    mem_gc_free(interp, pf->dirp);
    pf->dirp   = NULL;
    PackFile_Segment_destroy(interp, &pf->directory.base);
    return;
}


/*

=item C<static void make_code_pointers(PackFile_Segment *seg)>

Makes compact/shorthand pointers.

The first segments read are the default segments.

=cut

*/

static void
make_code_pointers(ARGMOD(PackFile_Segment *seg))
{
    ASSERT_ARGS(make_code_pointers)
    PackFile * const pf = seg->pf;

    switch (seg->type) {
      case PF_BYTEC_SEG:
        if (!pf->cur_cs)
            pf->cur_cs = (PackFile_ByteCode *)seg;
        break;
      case PF_CONST_SEG:
        if (!pf->cur_cs->const_table) {
            pf->cur_cs->const_table       = (PackFile_ConstTable *)seg;
            pf->cur_cs->const_table->code = pf->cur_cs;
        }
        break;
      case PF_UNKNOWN_SEG:
        break;
      case PF_DEBUG_SEG:
        pf->cur_cs->debugs       = (PackFile_Debug *)seg;
        pf->cur_cs->debugs->code = pf->cur_cs;
        break;
      default:
        break;
    }
}


/*

=item C<static int sub_pragma(PARROT_INTERP, pbc_action_enum_t action, const PMC
*sub_pmc)>

Checks B<sub_pmc>'s pragmas (e.g. flags like C<:load>, C<:main>, etc.)
returning 1 if the sub should be run for C<action>, a C<pbc_action_enum_t>.

=cut

*/

static int
sub_pragma(PARROT_INTERP, pbc_action_enum_t action, ARGIN(const PMC *sub_pmc))
{
    ASSERT_ARGS(sub_pragma)

    /* Note: the const casting is only needed because of the
     * internal details of the Sub_comp macros.
     * The assumption is that the TEST versions are in fact const,
     * so the casts are safe.
     * These casts are a quick fix to allow parrot build with c++,
     * a refactor of the macros will be a cleaner solution.  */
    DECL_CONST_CAST;
    Parrot_Sub_attributes *sub;
    int         todo    = 0;
    const int   pragmas = PObj_get_FLAGS(sub_pmc) &  SUB_FLAG_PF_MASK
                                                  & ~SUB_FLAG_IS_OUTER;
    PMC_get_sub(interp, PARROT_const_cast(PMC *, sub_pmc), sub);
    if (!pragmas && !Sub_comp_INIT_TEST(sub))
        return 0;

    switch (action) {
      case PBC_PBC:
      case PBC_MAIN:
        /* denote MAIN entry in first loaded PASM */
        if (interp->resume_flag & RESUME_INITIAL)
            todo = 1;

        /* :init functions need to be called at MAIN time, so return 1 */
        /* symreg.h:P_INIT */
        if (Sub_comp_INIT_TEST(sub))
            todo = 1;

        break;
      case PBC_LOADED:
        /* symreg.h:P_LOAD */
        if (pragmas & SUB_FLAG_PF_LOAD)
            todo = 1;
        break;
      default:
        break;
    }

    if (pragmas & (SUB_FLAG_PF_IMMEDIATE | SUB_FLAG_PF_POSTCOMP))
        todo = 1;

    return todo;
}


/*

=item C<static PMC* run_sub(PARROT_INTERP, PMC *sub_pmc)>

Runs the B<sub_pmc> due to its B<:load>, B<:immediate>, ... pragma

=cut

*/

PARROT_IGNORABLE_RESULT
PARROT_CAN_RETURN_NULL
static PMC*
run_sub(PARROT_INTERP, ARGIN(PMC *sub_pmc))
{
    ASSERT_ARGS(run_sub)
    Parrot_runcore_t *old_core = interp->run_core;
    PMC              *retval   = PMCNULL;

    Parrot_pcc_set_constants(interp, CURRENT_CONTEXT(interp),
            interp->code->const_table);

    Parrot_ext_call(interp, sub_pmc, "->P", &retval);
    interp->run_core = old_core;

    return retval;
}


/*

=item C<static PMC* do_1_sub_pragma(PARROT_INTERP, PMC *sub_pmc,
pbc_action_enum_t action)>

Runs autoloaded or immediate bytecode, marking the MAIN subroutine entry.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
static PMC*
do_1_sub_pragma(PARROT_INTERP, ARGMOD(PMC *sub_pmc), pbc_action_enum_t action)
{
    ASSERT_ARGS(do_1_sub_pragma)
    Parrot_Sub_attributes *sub;
    PMC_get_sub(interp, sub_pmc, sub);

    switch (action) {
      case PBC_IMMEDIATE:
        /* run IMMEDIATE sub */
        if (PObj_get_FLAGS(sub_pmc) & SUB_FLAG_PF_IMMEDIATE) {
            void *lo_var_ptr = interp->lo_var_ptr;
            PMC  *result;

            PObj_get_FLAGS(sub_pmc) &= ~SUB_FLAG_PF_IMMEDIATE;
            result     = run_sub(interp, sub_pmc);

            /* reset initial flag so MAIN detection works
             * and reset lo_var_ptr to prev */
            interp->resume_flag = RESUME_INITIAL;
            interp->lo_var_ptr  = lo_var_ptr;
            return result;
        }
        break;
      case PBC_POSTCOMP:
        /* run POSTCOMP sub */
        if (PObj_get_FLAGS(sub_pmc) &   SUB_FLAG_PF_POSTCOMP) {
            PObj_get_FLAGS(sub_pmc) &= ~SUB_FLAG_PF_POSTCOMP;
            run_sub(interp, sub_pmc);

            /* reset initial flag so MAIN detection works */
            interp->resume_flag = RESUME_INITIAL;
            return NULL;
        }
        break;

      case PBC_LOADED:
        if (PObj_get_FLAGS(sub_pmc) &   SUB_FLAG_PF_LOAD) {
            PObj_get_FLAGS(sub_pmc) &= ~SUB_FLAG_PF_LOAD;

            /* if loaded no need for init */
            Sub_comp_INIT_CLEAR(sub);
            run_sub(interp, sub_pmc);
        }
        break;
      default:
        if (PObj_get_FLAGS(sub_pmc) & SUB_FLAG_PF_MAIN) {
            if ((interp->resume_flag   &  RESUME_INITIAL)
             &&  interp->resume_offset == 0) {
                void           *ptr   = VTABLE_get_pointer(interp, sub_pmc);
                const ptrdiff_t code  = (ptrdiff_t) sub->seg->base.data;

                interp->resume_offset = ((ptrdiff_t)ptr - code)
                                      / sizeof (opcode_t *);

                PObj_get_FLAGS(sub_pmc)      &= ~SUB_FLAG_PF_MAIN;
                Parrot_pcc_set_sub(interp, CURRENT_CONTEXT(interp), sub_pmc);
            }
            else {
                Parrot_warn(interp, PARROT_WARNINGS_ALL_FLAG,
                                ":main sub not allowed\n");
            }
        }

        /* run :init tagged functions */
        if (action == PBC_MAIN && Sub_comp_INIT_TEST(sub)) {
            /* if loaded no need for init */
            Sub_comp_INIT_CLEAR(sub);

            /* if inited no need for load */
            PObj_get_FLAGS(sub_pmc) &= ~SUB_FLAG_PF_LOAD;

            run_sub(interp, sub_pmc);
            interp->resume_flag = RESUME_INITIAL;
        }
        break;
    }

    return NULL;
}


/*

=item C<static void mark_1_ct_seg(PARROT_INTERP, PackFile_ConstTable *ct)>

While the PMCs should be constant, their possible contents such as
properties aren't constructed const, so we have to mark them.

=cut

*/

static void
mark_1_ct_seg(PARROT_INTERP, ARGMOD(PackFile_ConstTable *ct))
{
    ASSERT_ARGS(mark_1_ct_seg)
    opcode_t i;

    for (i = 0; i < ct->str.const_count; i++)
        Parrot_gc_mark_STRING_alive(interp, ct->str.constants[i]);

    for (i = 0; i < ct->pmc.const_count; i++)
        Parrot_gc_mark_PMC_alive(interp, ct->pmc.constants[i]);
}


/*

=item C<static void mark_1_bc_seg(PARROT_INTERP, PackFile_ByteCode *bc)>

Mark gcables in bytecode header.

=cut

*/


static void
mark_1_bc_seg(PARROT_INTERP, ARGMOD(PackFile_ByteCode *bc))
{
    ASSERT_ARGS(mark_1_bc_seg)
    size_t i;
    for (i = 0; i < bc->n_libdeps; i++)
        Parrot_gc_mark_STRING_alive(interp, bc->libdeps[i]);
}

/*

=item C<static INTVAL find_const_iter(PARROT_INTERP, PackFile_Segment *seg, void
*user_data)>

Iterates over a PackFile_Directory, marking any constant segments.  Internal
use only.

=cut

*/

static INTVAL
find_const_iter(PARROT_INTERP, ARGIN(PackFile_Segment *seg),
                               ARGIN_NULLOK(void *user_data))
{
    ASSERT_ARGS(find_const_iter)

    Parrot_gc_mark_STRING_alive(interp, seg->name);

    switch (seg->type) {
      case PF_DIR_SEG:
        PackFile_map_segments(interp, (const PackFile_Directory *)seg,
                find_const_iter, user_data);
        break;

      case PF_CONST_SEG:
        mark_1_ct_seg(interp, (PackFile_ConstTable *)seg);
        break;

      case PF_BYTEC_SEG:
        mark_1_bc_seg(interp, (PackFile_ByteCode *)seg);
        break;

      default:
        break;
    }


    return 0;
}


/*

=item C<void mark_const_subs(PARROT_INTERP)>

Iterates over all directories and PackFile_Segments, finding and marking any
constant Subs.

=cut

*/

void
mark_const_subs(PARROT_INTERP)
{
    ASSERT_ARGS(mark_const_subs)

    PackFile * const self = interp->initial_pf;

    if (!self)
        return;
    else {
        /* locate top level dir */
        PackFile_Directory * const dir = &self->directory;

        /* iterate over all dir/segs */
        PackFile_map_segments(interp, dir, find_const_iter, NULL);
    }
}


/*

=item C<void do_sub_pragmas(PARROT_INTERP, PackFile_ByteCode *self,
pbc_action_enum_t action, PMC *eval_pmc)>

C<action> is one of C<PBC_PBC>, C<PBC_LOADED>, C<PBC_INIT>, or C<PBC_MAIN>.
These determine which subs get executed at this point. Some rules:

 :immediate subs always execute immediately
 :postcomp subs always execute immediately
 :main subs execute when we have the PBC_MAIN or PBC_PBC actions
 :init subs execute when :main does
 :load subs execute on PBC_LOAD

Also store the C<eval_pmc> in the sub structure, so that the eval PMC is kept
alive by living subs.

=cut

*/

PARROT_EXPORT
void
do_sub_pragmas(PARROT_INTERP, ARGIN(PackFile_ByteCode *self),
               pbc_action_enum_t action, ARGIN_NULLOK(PMC *eval_pmc))
{
    ASSERT_ARGS(do_sub_pragmas)
    PackFile_ConstTable * const ct = self->const_table;
    opcode_t i;

    TRACE_PRINTF(("PackFile: do_sub_pragmas (action=%d)\n", action));

    for (i = 0; i < ct->pmc.const_count; ++i) {
        STRING * const SUB = CONST_STRING(interp, "Sub");
        PMC *sub_pmc = ct->pmc.constants[i];

        if (VTABLE_isa(interp, sub_pmc, SUB)) {
            Parrot_Sub_attributes *sub;

            PMC_get_sub(interp, sub_pmc, sub);
            sub->eval_pmc = eval_pmc;

            if (((PObj_get_FLAGS(sub_pmc) & SUB_FLAG_PF_MASK)
            ||   (Sub_comp_get_FLAGS(sub) & SUB_COMP_FLAG_MASK))
            &&    sub_pragma(interp, action, sub_pmc)) {
                PMC * const result = do_1_sub_pragma(interp, sub_pmc,
                        action);

                /* replace Sub PMC with computation results */
                if (action == PBC_IMMEDIATE && !PMC_IS_NULL(result)) {
                    ct->pmc.constants[i] = result;
                }
            }
        }
    }
}


/*

=item C<opcode_t PackFile_unpack(PARROT_INTERP, PackFile *self, const opcode_t
*packed, size_t packed_size)>

Unpacks a C<PackFile> from a block of memory, ensuring that the magic number is
valid and that Parrot can read this bytecode version, Parrot, and performing
any required endian and word size transforms.

Returns size of unpacked opcodes if everything is okay, else zero (0).

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
opcode_t
PackFile_unpack(PARROT_INTERP, ARGMOD(PackFile *self),
    ARGIN(const opcode_t *packed), size_t packed_size)
{
    ASSERT_ARGS(PackFile_unpack)
    PackFile_Header * const header = self->header;
    const opcode_t         *cursor;
    int                     header_read_length;
    opcode_t                padding;
#if TRACE_PACKFILE
    PackFile        * const pf  = self;
#endif

    if (packed_size < PACKFILE_HEADER_BYTES) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: "
            "Buffer length %d is shorter than PACKFILE_HEADER_BYTES %d\n",
            packed_size, PACKFILE_HEADER_BYTES);
        return 0;
    }

    self->src  = packed;
    self->size = packed_size;

    /* Extract the header. */
    memcpy(header, packed, PACKFILE_HEADER_BYTES);

    /* Ensure the magic is correct. */
    if (memcmp(header->magic, "\376PBC\r\n\032\n", 8) != 0) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: "
            "This is not a valid Parrot bytecode file\n");
        return 0;
    }

    /* Ensure the bytecode version is one we can read. Currently, we only
     * support bytecode versions matching the current one.
     *
     * tools/dev/pbc_header.pl --upd t/native_pbc/(ASTERISK).pbc
     * stamps version and fingerprint in the native tests.
     * NOTE: (ASTERISK) is *, we don't want to fool the C preprocessor. */
    if (header->bc_major != PARROT_PBC_MAJOR
    ||  header->bc_minor != PARROT_PBC_MINOR) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: This Parrot cannot read "
            "bytecode files with version %d.%d.\n",
            header->bc_major, header->bc_minor);
        if (!(self->options & PFOPT_UTILS))
            return 0;
    }

    /* Check wordsize, byte order and floating point number type are valid. */
    if (header->wordsize != 4 && header->wordsize != 8) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: Invalid wordsize %d\n",
                    header->wordsize);
        return 0;
    }

    if (header->byteorder != 0 && header->byteorder != 1) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: Invalid byte ordering %d\n",
                    header->byteorder);
        return 0;
    }

    if (header->floattype > FLOATTYPE_MAX) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: Invalid floattype %d\n",
                    header->floattype);
        return 0;
    }

    /* Describe what was read for debugging. */
    TRACE_PRINTF(("PackFile_unpack: Wordsize %d.\n", header->wordsize));
    TRACE_PRINTF(("PackFile_unpack: Floattype %d (%s).\n",
                  header->floattype,
                  header->floattype == FLOATTYPE_8
                      ? FLOATTYPE_8_NAME
                      : header->floattype == FLOATTYPE_16
                          ? FLOATTYPE_16_NAME
                          : FLOATTYPE_12_NAME));
    TRACE_PRINTF(("PackFile_unpack: Byteorder %d (%sendian).\n",
                  header->byteorder, header->byteorder ? "big " : "little-"));

    /* Check the UUID type is valid and, if needed, read a UUID. */
    if (header->uuid_type == 0) {
        /* No UUID; fine, nothing more to do. */
    }
    else if (header->uuid_type == 1) {
        if (packed_size < (size_t) PACKFILE_HEADER_BYTES + header->uuid_size) {
            Parrot_io_eprintf(NULL, "PackFile_unpack: "
                    "Buffer length %d is shorter than PACKFILE_HEADER_BYTES + uuid_size %d\n",
                    packed_size, PACKFILE_HEADER_BYTES + header->uuid_size);
            return 0;
        }


        /* Read in the UUID. We'll put it in a NULL-terminated string, just in
         * case people use it that way. */
        header->uuid_data = mem_gc_allocate_n_typed(interp,
                header->uuid_size + 1, unsigned char);

        memcpy(header->uuid_data, packed + PACKFILE_HEADER_BYTES,
                header->uuid_size);

        /* NULL terminate */
        header->uuid_data[header->uuid_size] = '\0';
    }
    else
        /* Don't know this UUID type. */
        Parrot_io_eprintf(NULL, "PackFile_unpack: Invalid UUID type %d\n",
                    header->uuid_type);

    /* Set cursor to position after what we've read, allowing for padding to a
     * 16 byte boundary. */
    header_read_length  = PACKFILE_HEADER_BYTES + header->uuid_size;
    header_read_length += PAD_16_B(header_read_length);
    cursor              = packed + (header_read_length / sizeof (opcode_t));
    TRACE_PRINTF(("PackFile_unpack: pad=%d\n",
                  (char *)cursor - (char *)packed));

    /* Set what transforms we need to do when reading the rest of the file. */
    PackFile_assign_transforms(self);

    if (self->options & PFOPT_PMC_FREEZE_ONLY)
        return cursor - packed;

    /* Directory format. */
    header->dir_format = PF_fetch_opcode(self, &cursor);

    if (header->dir_format != PF_DIR_FORMAT) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: Dir format was %d not %d\n",
                    header->dir_format, PF_DIR_FORMAT);
        return 0;
    }

    /* Padding. */
    TRACE_PRINTF(("PackFile_unpack: 3 words padding.\n"));
    padding = PF_fetch_opcode(self, &cursor);
    padding = PF_fetch_opcode(self, &cursor);
    padding = PF_fetch_opcode(self, &cursor);
    UNUSED(padding);

    TRACE_PRINTF(("PackFile_unpack: Directory read, offset %d.\n",
                  (INTVAL)cursor - (INTVAL)packed));
    self->directory.base.file_offset = (INTVAL)cursor - (INTVAL)self->src;
    if (self->options & PFOPT_HEADERONLY)
        return cursor - packed;

    /* now unpack dir, which unpacks its contents ... */
    Parrot_block_GC_mark(interp);
    cursor = PackFile_Segment_unpack(interp,
                                     &self->directory.base, cursor);
    Parrot_unblock_GC_mark(interp);

#ifdef PARROT_HAS_HEADER_SYSMMAN
    if (self->is_mmap_ped
    && (self->need_endianize || self->need_wordsize)) {
        DECL_CONST_CAST;
        /* Cast the result to void to avoid a warning with
         * some not-so-standard mmap headers
         */
        munmap((void *)PARROT_const_cast(opcode_t *, self->src), self->size);
        self->is_mmap_ped = 0;
    }
#endif

    TRACE_PRINTF(("PackFile_unpack: Unpack done.\n"));

    return cursor - packed;
}


/*

=item C<INTVAL PackFile_map_segments(PARROT_INTERP, const PackFile_Directory
*dir, PackFile_map_segments_func_t callback, void *user_data)>

Calls the callback function C<callback> for each segment in the directory
C<dir> called. The pointer C<user_data> is included in each call.

If a callback returns non-zero, segment processing stops, returning this value.

=cut

*/

PARROT_EXPORT
INTVAL
PackFile_map_segments(PARROT_INTERP, ARGIN(const PackFile_Directory *dir),
                       PackFile_map_segments_func_t callback,
                       ARGIN_NULLOK(void *user_data))
{
    ASSERT_ARGS(PackFile_map_segments)
    size_t i;

    for (i = 0; i < dir->num_segments; ++i) {
        const INTVAL ret = callback(interp, dir->segments[i], user_data);
        if (ret)
            return ret;
    }

    return 0;
}


/*

=item C<void PackFile_add_segment(PARROT_INTERP, PackFile_Directory *dir,
PackFile_Segment *seg)>

Adds the Segment C<seg> to the directory C<dir>.  The PackFile becomes the
owner of the segment; it gets destroyed when the PackFile does.

=cut

*/

PARROT_EXPORT
void
PackFile_add_segment(PARROT_INTERP, ARGMOD(PackFile_Directory *dir),
        ARGMOD(PackFile_Segment *seg))
{
    ASSERT_ARGS(PackFile_add_segment)
    dir->segments = mem_gc_realloc_n_typed_zeroed(interp, dir->segments,
            dir->num_segments + 1, dir->num_segments, PackFile_Segment *);
    dir->segments[dir->num_segments] = seg;
    ++dir->num_segments;
    seg->dir = dir;

    return;
}


/*

=item C<PackFile_Segment * PackFile_find_segment(PARROT_INTERP,
PackFile_Directory *dir, const STRING *name, int sub_dir)>

Finds the segment with the name C<name> in the C<PackFile_Directory> if
C<sub_dir> is true, searches directories recursively.  The returned segment is
still owned by the C<PackFile>.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
PackFile_Segment *
PackFile_find_segment(PARROT_INTERP, ARGIN_NULLOK(PackFile_Directory *dir),
    ARGIN(const STRING *name), int sub_dir)
{
    ASSERT_ARGS(PackFile_find_segment)
    if (dir) {
        size_t i;

        for (i = 0; i < dir->num_segments; ++i) {
            PackFile_Segment *seg = dir->segments[i];

            if (seg) {
                if (STRING_equal(interp, seg->name, name))
                    return seg;

                if (sub_dir && seg->type == PF_DIR_SEG) {
                    seg = PackFile_find_segment(interp,
                            (PackFile_Directory *)seg, name, sub_dir);

                    if (seg)
                        return seg;
                }
            }
        }
    }

    return NULL;
}


/*

=item C<PackFile_Segment * PackFile_remove_segment_by_name(PARROT_INTERP,
PackFile_Directory *dir, STRING *name)>

Finds, removes, and returns the segment with name C<name> in the
C<PackFile_Directory>.  The caller is responsible for destroying the segment.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
PackFile_Segment *
PackFile_remove_segment_by_name(PARROT_INTERP, ARGMOD(PackFile_Directory *dir),
                                ARGIN(STRING *name))
{
    ASSERT_ARGS(PackFile_remove_segment_by_name)
    size_t i;

    for (i = 0; i < dir->num_segments; ++i) {
        PackFile_Segment * const seg = dir->segments[i];
        if (STRING_equal(interp, seg->name, name)) {
            dir->num_segments--;

            if (i != dir->num_segments) {
                /* We're not the last segment, so we need to move things */
                memmove(&dir->segments[i], &dir->segments[i+1],
                       (dir->num_segments - i) * sizeof (PackFile_Segment *));
            }

            return seg;
        }
    }

    return NULL;
}


/*

=back

=head2 PackFile Structure Functions

=over 4

=item C<static void PackFile_set_header(PackFile_Header *header)>

Fills a C<PackFile> header with system specific data.

=cut

*/

static void
PackFile_set_header(ARGOUT(PackFile_Header *header))
{
    ASSERT_ARGS(PackFile_set_header)
    memcpy(header->magic, "\376PBC\r\n\032\n", 8);
    header->wordsize    = sizeof (opcode_t);
    header->byteorder   = PARROT_BIGENDIAN;
    header->major       = PARROT_MAJOR_VERSION;
    header->minor       = PARROT_MINOR_VERSION;
    header->patch       = PARROT_PATCH_VERSION;
    header->bc_major    = PARROT_PBC_MAJOR;
    header->bc_minor    = PARROT_PBC_MINOR;
#if NUMVAL_SIZE == 8
    header->floattype = FLOATTYPE_8;
#else
#  if (NUMVAL_SIZE == 12) && !PARROT_BIGENDIAN
    header->floattype = FLOATTYPE_12;
#  else
#    if (NUMVAL_SIZE == 16)
    header->floattype = FLOATTYPE_16;
#    else
    exit_fatal(1, "PackFile_set_header: Unsupported floattype NUMVAL_SIZE=%d,"
               " PARROT_BIGENDIAN=%s\n", NUMVAL_SIZE,
               PARROT_BIGENDIAN ? "big-endian" : "little-endian");
#    endif
#  endif
#endif
}


/*

=item C<PackFile * PackFile_new(PARROT_INTERP, INTVAL is_mapped)>

Allocates a new empty C<PackFile> and sets up the directory.

Directory segment:

  +----------+----------+----------+----------+
  |    Segment Header                         |
  |    ..............                         |
  +----------+----------+----------+----------+

  +----------+----------+----------+----------+
  |    number of directory items              |
  +----------+----------+----------+----------+

followed by a sequence of items

  +----------+----------+----------+----------+
  |    Segment type                           |
  +----------+----------+----------+----------+
  |    "name"                                 |
  |    ...     '\0'       padding bytes       |
  +----------+----------+----------+----------+
  |    Offset in the file                     |
  +----------+----------+----------+----------+
  |    Size of the segment                    |
  +----------+----------+----------+----------+

"name" is a NUL-terminated c-string encoded in plain ASCII.

Segment types are defined in F<include/parrot/packfile.h>.

Offset and size are in C<opcode_t>.

A Segment Header has these entries:

 - op_count     total ops of segment incl. this count
 - itype        internal type of segment
 - id           internal id e.g code seg nr
 - size         size of following op array, 0 if none
 * data         possibly empty data, or e.g. byte code

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
PackFile *
PackFile_new(PARROT_INTERP, INTVAL is_mapped)
{
    ASSERT_ARGS(PackFile_new)
    PackFile * const pf = mem_gc_allocate_zeroed_typed(interp, PackFile);
    pf->header          = mem_gc_allocate_zeroed_typed(interp, PackFile_Header);
    pf->is_mmap_ped     = is_mapped;
    pf->options         = PFOPT_NONE;

    /* fill header with system specific data */
    PackFile_set_header(pf->header);

    /* Other fields empty for now */
    pf->cur_cs = NULL;
    pf_register_standard_funcs(interp, pf);

    /* create the master directory, all sub-dirs go there */
    pf->directory.base.pf = pf;
    pf->dirp              = (PackFile_Directory *)
        PackFile_Segment_new_seg(interp, &pf->directory,
            PF_DIR_SEG, DIRECTORY_SEGMENT_NAME, 0);
    pf->directory         = *pf->dirp;

    pf->fetch_op = (packfile_fetch_op_t)NULL;
    pf->fetch_iv = (packfile_fetch_iv_t)NULL;
    pf->fetch_nv = (packfile_fetch_nv_t)NULL;

    return pf;
}


/*

=item C<PackFile * PackFile_new_dummy(PARROT_INTERP, STRING *name)>

Creates a new (initial) dummy PackFile. This is necessary if the interpreter
doesn't load any bytecode but instead uses C<Parrot_compile_string>.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
PackFile *
PackFile_new_dummy(PARROT_INTERP, ARGIN(STRING *name))
{
    ASSERT_ARGS(PackFile_new_dummy)

    PackFile * const pf = PackFile_new(interp, 0);
    interp->initial_pf  = pf;
    interp->code        = pf->cur_cs
                        = PF_create_default_segs(interp, name, 1);

    return pf;
}


/*

=item C<void PackFile_funcs_register(PARROT_INTERP, PackFile *pf, UINTVAL type,
const PackFile_funcs funcs)>

Registers the C<pack>/C<unpack>/... functions for a packfile type.

=cut

*/

PARROT_EXPORT
void
PackFile_funcs_register(SHIM_INTERP, ARGOUT(PackFile *pf), UINTVAL type,
                        const PackFile_funcs funcs)
{
    ASSERT_ARGS(PackFile_funcs_register)
    pf->PackFuncs[type] = funcs;
}


/*

=item C<static const opcode_t * default_unpack(PARROT_INTERP, PackFile_Segment
*self, const opcode_t *cursor)>

Unpacks a PackFile given a cursor into PBC.  This is the default unpack.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
static const opcode_t *
default_unpack(PARROT_INTERP, ARGMOD(PackFile_Segment *self), ARGIN(const opcode_t *cursor))
{
    ASSERT_ARGS(default_unpack)
    DECL_CONST_CAST_OF(opcode_t);
#if TRACE_PACKFILE
    PackFile * const pf  = self->pf;
#endif

    self->op_count = PF_fetch_opcode(self->pf, &cursor);
    self->itype    = PF_fetch_opcode(self->pf, &cursor);
    self->id       = PF_fetch_opcode(self->pf, &cursor);
    self->size     = PF_fetch_opcode(self->pf, &cursor);
    TRACE_PRINTF_VAL(("default_unpack: op_count=%d, itype=%d, id=%d, size=%d.\n",
        self->op_count, self->itype, self->id, self->size));

    if (self->size == 0)
        return cursor;

    /* if the packfile is mmap()ed just point to it if we don't
     * need any fetch transforms */
    if (self->pf->is_mmap_ped
    && !self->pf->need_endianize
    && !self->pf->need_wordsize) {
        self->data  = PARROT_const_cast(opcode_t *, cursor);
        cursor     += self->size;
        return cursor;
    }

    /* else allocate mem */
    self->data = mem_gc_allocate_n_typed(interp, self->size, opcode_t);

    if (!self->data) {
        Parrot_io_eprintf(NULL, "PackFile_unpack: Unable to allocate data memory!\n");
        self->size = 0;
        return NULL;
    }

    if (!self->pf->need_endianize && !self->pf->need_wordsize) {
        mem_sys_memcopy(self->data, cursor, self->size * sizeof (opcode_t));
        cursor += self->size;
    }
    else {
        int i;
        TRACE_PRINTF(("default_unpack: pre-fetch %d ops into data\n",
                      self->size));
        for (i = 0; i < (int)self->size; ++i) {
            self->data[i] = PF_fetch_opcode(self->pf, &cursor);
            TRACE_PRINTF(("default_unpack: transformed op[#%d]/%d %u\n",
                          i, self->size, self->data[i]));
        }
    }

    return cursor;
}


/*

=item C<void default_dump_header(PARROT_INTERP, const PackFile_Segment *self)>

Dumps the header of a given PackFile_Segment.

=cut

*/

void
default_dump_header(PARROT_INTERP, ARGIN(const PackFile_Segment *self))
{
    ASSERT_ARGS(default_dump_header)
    Parrot_io_printf(interp, "%Ss => [ # offs 0x%x(%d)",
            self->name, (int)self->file_offset, (int)self->file_offset);
    Parrot_io_printf(interp, " = op_count %d, itype %d, id %d, size %d, ...",
            (int)self->op_count, (int)self->itype,
            (int)self->id, (int)self->size);
}


/*

=item C<static void default_dump(PARROT_INTERP, const PackFile_Segment *self)>

Dumps a PackFile_Segment.

=cut

*/

static void
default_dump(PARROT_INTERP, ARGIN(const PackFile_Segment *self))
{
    ASSERT_ARGS(default_dump)
    size_t i = self->data ? 0: self->file_offset + SEGMENT_HEADER_SIZE;

    default_dump_header(interp, self);

    if (i % 8)
        Parrot_io_printf(interp, "\n %04x:  ", (int) i);

    for (; i < (self->data ? self->size :
            self->file_offset + self->op_count); ++i) {

        if (i % 8 == 0)
            Parrot_io_printf(interp, "\n %04x:  ", (int) i);

        Parrot_io_printf(interp, "%08lx ", (unsigned long)
                self->data ? self->data[i] : self->pf->src[i]);
    }

    Parrot_io_printf(interp, "\n]\n");
}


/*

=item C<static void pf_register_standard_funcs(PARROT_INTERP, PackFile *pf)>

Registers a PackFile's functions; called from within C<PackFile_new()>.

=cut

*/

static void
pf_register_standard_funcs(PARROT_INTERP, ARGMOD(PackFile *pf))
{
    ASSERT_ARGS(pf_register_standard_funcs)

    static const PackFile_funcs dirf = {
        directory_new,
        directory_destroy,
        directory_packed_size,
        directory_pack,
        directory_unpack,
        directory_dump
    };

    static const PackFile_funcs defaultf = {
        PackFile_Segment_new,
        (PackFile_Segment_destroy_func_t)     NULLfunc,
        (PackFile_Segment_packed_size_func_t) NULLfunc,
        (PackFile_Segment_pack_func_t)        NULLfunc,
        (PackFile_Segment_unpack_func_t)      NULLfunc,
        default_dump
    };

    static const PackFile_funcs constf = {
        const_new,
        const_destroy,
        PackFile_ConstTable_pack_size,
        PackFile_ConstTable_pack,
        PackFile_ConstTable_unpack,
        default_dump
    };

    static const PackFile_funcs bytef = {
        byte_code_new,
        byte_code_destroy,
        byte_code_packed_size,
        byte_code_pack,
        byte_code_unpack,
        default_dump
    };

    static const PackFile_funcs debugf = {
        pf_debug_new,
        pf_debug_destroy,
        pf_debug_packed_size,
        pf_debug_pack,
        pf_debug_unpack,
        pf_debug_dump
    };

    static const PackFile_funcs annotationf = {
        PackFile_Annotations_new,
        PackFile_Annotations_destroy,
        PackFile_Annotations_packed_size,
        PackFile_Annotations_pack,
        PackFile_Annotations_unpack,
        PackFile_Annotations_dump
    };

    PackFile_funcs_register(interp, pf, PF_DIR_SEG,         dirf);
    PackFile_funcs_register(interp, pf, PF_UNKNOWN_SEG,     defaultf);
    PackFile_funcs_register(interp, pf, PF_CONST_SEG,       constf);
    PackFile_funcs_register(interp, pf, PF_BYTEC_SEG,       bytef);
    PackFile_funcs_register(interp, pf, PF_DEBUG_SEG,       debugf);
    PackFile_funcs_register(interp, pf, PF_ANNOTATIONS_SEG, annotationf);

    return;
}


/*

=item C<PackFile_Segment * PackFile_Segment_new_seg(PARROT_INTERP,
PackFile_Directory *dir, UINTVAL type, STRING *name, int add)>

Creates a new segment in the given PackFile_Directory of the given C<type> with
the given C<name>.  If C<add> is true, adds the segment to the directory.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
PackFile_Segment *
PackFile_Segment_new_seg(PARROT_INTERP, ARGMOD(PackFile_Directory *dir),
        UINTVAL type, ARGIN(STRING *name), int add)
{
    ASSERT_ARGS(PackFile_Segment_new_seg)
    PackFile * const                  pf  = dir->base.pf;
    const PackFile_Segment_new_func_t f   = pf->PackFuncs[type].new_seg;
    PackFile_Segment * const          seg = (f)(interp, pf, name, add);

    segment_init(seg, pf, name);
    seg->type = type;

    if (add)
        PackFile_add_segment(interp, dir, seg);

    return seg;
}


/*

=item C<static PackFile_Segment * create_seg(PARROT_INTERP, PackFile_Directory
*dir, pack_file_types t, STRING *name, STRING *file_name, int add)>

Creates a new PackFile_Segment for the given C<file_name>.  See
C<PackFile_Segment_new_seg()> for the other arguments.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment *
create_seg(PARROT_INTERP, ARGMOD(PackFile_Directory *dir), pack_file_types t,
           ARGIN(STRING *name), ARGIN(STRING *file_name), int add)
{
    ASSERT_ARGS(create_seg)
    PackFile_Segment *seg;
    STRING           *seg_name;

    seg_name = Parrot_sprintf_c(interp, "%Ss_%Ss", name, file_name);
    seg = PackFile_Segment_new_seg(interp, dir, t, seg_name, add);
    return seg;
}


/*

=item C<PackFile_ByteCode * PF_create_default_segs(PARROT_INTERP, STRING
*file_name, int add)>

Creates the bytecode and constant segments for C<file_name>. If C<add>
is true, the current packfile becomes the owner of these segments by adding the
segments to the directory.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
PackFile_ByteCode *
PF_create_default_segs(PARROT_INTERP, ARGIN(STRING *file_name), int add)
{
    ASSERT_ARGS(PF_create_default_segs)
    PackFile          * const pf     = interp->initial_pf;
    PackFile_ByteCode * const cur_cs =
        (PackFile_ByteCode *)create_seg(interp, &pf->directory,
            PF_BYTEC_SEG, BYTE_CODE_SEGMENT_NAME, file_name, add);

    cur_cs->const_table  =
        (PackFile_ConstTable *)create_seg(interp, &pf->directory,
            PF_CONST_SEG, CONSTANT_SEGMENT_NAME, file_name, add);

    cur_cs->const_table->code = cur_cs;

    return cur_cs;
}


/*

=item C<void PackFile_Segment_destroy(PARROT_INTERP, PackFile_Segment *self)>

Destroys the given PackFile_Segment.

=cut

*/

PARROT_EXPORT
void
PackFile_Segment_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
{
    ASSERT_ARGS(PackFile_Segment_destroy)
    const PackFile_Segment_destroy_func_t f =
        self->pf->PackFuncs[self->type].destroy;

    if (f)
        (f)(interp, self);

    /* destroy self after specific */
    default_destroy(interp, self);
}


/*

=item C<size_t PackFile_Segment_packed_size(PARROT_INTERP, PackFile_Segment
*self)>

Returns the size of the given segment, when packed, taking into account padding
and alignment.

=cut

*/

PARROT_EXPORT
size_t
PackFile_Segment_packed_size(PARROT_INTERP, ARGIN(PackFile_Segment *self))
{
    ASSERT_ARGS(PackFile_Segment_packed_size)
    size_t size                           = default_packed_size(self);
    const size_t align                    = 16 / sizeof (opcode_t);
    PackFile_Segment_packed_size_func_t f =
        self->pf->PackFuncs[self->type].packed_size;

    if (f)
        size += (f)(interp, self);

    /* pad/align it */
    if (align && size % align)
        size += (align - size % align);

    return size;
}


/*

=item C<opcode_t * PackFile_Segment_pack(PARROT_INTERP, PackFile_Segment *self,
opcode_t *cursor)>

Packs a PackFile_Segment, returning a cursor to the start of the results.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
opcode_t *
PackFile_Segment_pack(PARROT_INTERP, ARGIN(PackFile_Segment *self),
        ARGIN(opcode_t *cursor))
{
    ASSERT_ARGS(PackFile_Segment_pack)
    /*const size_t align             = 16 / sizeof (opcode_t);*/
    PackFile_Segment_pack_func_t f =
        self->pf->PackFuncs[self->type].pack;
    opcode_t * old_cursor;          /* Used for filling padding with 0 */
#if TRACE_PACKFILE
    PackFile * const pf  = self->pf;
#endif

    cursor = default_pack(self, cursor);

    if (f)
        cursor = (f)(interp, self, cursor);

    TRACE_PRINTF_ALIGN(("-ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                        OFFS(pf, cursor), pf->src, cursor));
    old_cursor = cursor;
    ALIGN_16(self->pf, cursor);
    /* fill padding with zeros */
    while (old_cursor != cursor)
        *old_cursor++ = 0;

    /*if (align && (cursor - self->pf->src) % align)
      cursor += align - (cursor - self->pf->src) % align;*/
    TRACE_PRINTF_ALIGN(("+ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                      OFFS(pf, cursor), pf->src, cursor));

    return cursor;
}


/*

=item C<const opcode_t * PackFile_Segment_unpack(PARROT_INTERP, PackFile_Segment
*self, const opcode_t *cursor)>

Unpacks a PackFile_Segment, returning a cursor to the results on success and
NULL otherwise.

All all these functions call the related C<default_*> function.

If a special is defined this gets called after.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
const opcode_t *
PackFile_Segment_unpack(PARROT_INTERP, ARGMOD(PackFile_Segment *self),
        ARGIN(const opcode_t *cursor))
{
    ASSERT_ARGS(PackFile_Segment_unpack)
    PackFile_Segment_unpack_func_t f = self->pf->PackFuncs[self->type].unpack;
    int offs;
#if TRACE_PACKFILE
    PackFile * const pf  = self->pf;
#endif

    cursor = default_unpack(interp, self, cursor);

    if (!cursor)
        return NULL;

    if (f) {
        TRACE_PRINTF(("PackFile_Segment_unpack: special\n"));

        cursor = (f)(interp, self, cursor);
        if (!cursor)
            return NULL;
    }

    offs = OFFS(self->pf, cursor);
    TRACE_PRINTF_ALIGN(("-S ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                        offs, self->pf->src, cursor));
    offs += PAD_16_B(offs);
    cursor = self->pf->src + offs/(sizeof (opcode_t));
    TRACE_PRINTF_ALIGN(("+S ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                        offs, self->pf->src, cursor));
    return cursor;
}


/*

=item C<void PackFile_Segment_dump(PARROT_INTERP, PackFile_Segment *self)>

Dumps the segment C<self>.

=cut

*/

PARROT_EXPORT
void
PackFile_Segment_dump(PARROT_INTERP, ARGIN(PackFile_Segment *self))
{
    ASSERT_ARGS(PackFile_Segment_dump)
    self->pf->PackFuncs[self->type].dump(interp, self);
}


/*

=back

=head2 Standard Directory Functions

=over 4

=item C<static PackFile_Segment * directory_new(PARROT_INTERP, PackFile *pf,
STRING *name, int add)>

Returns a new C<PackFile_Directory> cast as a C<PackFile_Segment>.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment *
directory_new(PARROT_INTERP, SHIM(PackFile *pf), SHIM(STRING *name), SHIM(int add))
{
    ASSERT_ARGS(directory_new)

    return (PackFile_Segment *)mem_gc_allocate_zeroed_typed(interp, PackFile_Directory);
}


/*

=item C<static void directory_dump(PARROT_INTERP, const PackFile_Segment *self)>

Dumps the directory C<self>.

=cut

*/

static void
directory_dump(PARROT_INTERP, ARGIN(const PackFile_Segment *self))
{
    ASSERT_ARGS(directory_dump)
    const PackFile_Directory * const dir = (const PackFile_Directory *) self;
    size_t i;

    default_dump_header(interp, self);

    Parrot_io_printf(interp, "\n\t# %d segments\n", dir->num_segments);

    for (i = 0; i < dir->num_segments; ++i) {
        const PackFile_Segment * const seg = dir->segments[i];

        Parrot_io_printf(interp,
                "\ttype %d\t%Ss\t", (int)seg->type, seg->name);

        Parrot_io_printf(interp,
                " offs 0x%x(0x%x)\top_count %d\n",
                (int)seg->file_offset,
                (int)seg->file_offset * sizeof (opcode_t),
                (int)seg->op_count);
    }

    Parrot_io_printf(interp, "]\n");

    for (i = 0; i < dir->num_segments; ++i)
        PackFile_Segment_dump(interp, dir->segments[i]);
}


/*

=item C<static const opcode_t * directory_unpack(PARROT_INTERP, PackFile_Segment
*segp, const opcode_t *cursor)>

Unpacks the directory from the provided cursor.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static const opcode_t *
directory_unpack(PARROT_INTERP, ARGMOD(PackFile_Segment *segp), ARGIN(const opcode_t *cursor))
{
    ASSERT_ARGS(directory_unpack)
    PackFile_Directory * const dir = (PackFile_Directory *)segp;
    PackFile           * const pf  = dir->base.pf;
    const opcode_t            *pos;
    size_t                     i;
    int                        offs;

    PARROT_ASSERT(pf);
    dir->num_segments = PF_fetch_opcode(pf, &cursor);
    TRACE_PRINTF(("directory_unpack: %ld num_segments\n", dir->num_segments));
    dir->segments = mem_gc_allocate_n_zeroed_typed(interp,
            dir->num_segments, PackFile_Segment *);

    for (i = 0; i < dir->num_segments; ++i) {
        PackFile_Segment *seg;
        char             *buf;
        STRING           *name;
        size_t            opcode;

        /* get type */
        UINTVAL type = PF_fetch_opcode(pf, &cursor);
        if (type >= PF_MAX_SEG)
            type = PF_UNKNOWN_SEG;

        TRACE_PRINTF_VAL(("Segment type %d.\n", type));

        /* get name */
        buf = PF_fetch_cstring(interp, pf, &cursor);
        TRACE_PRINTF_VAL(("Segment name \"%s\".\n", name));

        /* create it */
        name = Parrot_str_new(interp, buf, strlen(buf));
        seg  = PackFile_Segment_new_seg(interp, dir, type, name, 0);
        mem_gc_free(interp, buf);

        seg->file_offset = PF_fetch_opcode(pf, &cursor);
        TRACE_PRINTF_VAL(("Segment file_offset %ld.\n", seg->file_offset));

        seg->op_count    = PF_fetch_opcode(pf, &cursor);
        TRACE_PRINTF_VAL(("Segment op_count %ld.\n", seg->op_count));

        if (pf->need_wordsize) {
#if OPCODE_T_SIZE == 8
            if (pf->header->wordsize == 4)
                pos = pf->src + seg->file_offset / 2;
#else
            if (pf->header->wordsize == 8)
                pos = pf->src + seg->file_offset * 2;
#endif
            else {
                fprintf(stderr, "directory_unpack failed: invalid wordsize %d\n",
                        (int)pf->header->wordsize);
                return NULL;
            }
            TRACE_PRINTF_VAL(("Segment offset: new pos 0x%x "
                              "(src=0x%x cursor=0x%x).\n",
                              OFFS(pf, pos), pf->src, cursor));
        }
        else
            pos = pf->src + seg->file_offset;

        opcode = PF_fetch_opcode(pf, &pos);

        if (seg->op_count != opcode) {
            Parrot_io_eprintf(interp,
                     "%Ss: Size in directory %d doesn't match size %d "
                     "at offset 0x%x\n", seg->name, (int)seg->op_count,
                     (int)opcode, (int)seg->file_offset);
        }

        if (i) {
            PackFile_Segment *last = dir->segments[i - 1];
            if (last->file_offset + last->op_count != seg->file_offset)
                fprintf(stderr, "section: sections are not back to back\n");
        }

        make_code_pointers(seg);

        /* store the segment */
        dir->segments[i] = seg;
        seg->dir         = dir;
    }

    offs = OFFS(pf, cursor);
    TRACE_PRINTF_ALIGN(("-ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                      offs, pf->src, cursor));
    offs += PAD_16_B(offs);
    cursor = pf->src + offs/(sizeof (opcode_t));
    TRACE_PRINTF_ALIGN(("+ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                      offs, pf->src, cursor));

    /* and now unpack contents of dir */
    for (i = 0; cursor && i < dir->num_segments; ++i) {
        const opcode_t * const csave = cursor;

        /* check len again */
        size_t tmp   = PF_fetch_opcode(pf, &cursor);

        /* keep gcc -O silent */
        size_t delta = 0;

        cursor = csave;
        TRACE_PRINTF_VAL(("PackFile_Segment_unpack [%d] tmp len=%d.\n", i, tmp));
        pos    = PackFile_Segment_unpack(interp, dir->segments[i], cursor);

        if (!pos) {
            Parrot_io_eprintf(interp, "PackFile_unpack segment '%Ss' failed\n",
                    dir->segments[i]->name);
            return NULL;
        }
        else {
            TRACE_PRINTF_VAL(("PackFile_Segment_unpack ok. pos=0x%x\n", pos));
        }

        /* FIXME bug on 64bit reading 32bit lurking here! TT #254 */
        if (pf->need_wordsize) {
#if OPCODE_T_SIZE == 8
            if (pf->header->wordsize == 4)
                delta = (pos - cursor) * 2;
#else
            if (pf->header->wordsize == 8)
                delta = (pos - cursor) / 2;
#endif
        }
        else
            delta = pos - cursor;

        TRACE_PRINTF_VAL(("  delta=%d, pos=0x%x, cursor=0x%x\n",
                          delta, pos, cursor));

        if ((size_t)delta != tmp || dir->segments[i]->op_count != tmp)
            Parrot_io_eprintf(interp, "PackFile_unpack segment '%Ss' directory length %d "
                    "length in file %d needed %d for unpack\n",
                    dir->segments[i]->name,
                    (int)dir->segments[i]->op_count, (int)tmp,
                    (int)delta);
        cursor = pos;
    }

    return cursor;
}


/*

=item C<static void directory_destroy(PARROT_INTERP, PackFile_Segment *self)>

Destroys the directory.

=cut

*/

static void
directory_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
{
    ASSERT_ARGS(directory_destroy)
    PackFile_Directory * const dir = (PackFile_Directory *)self;
    size_t i;

    for (i = 0; i < dir->num_segments; ++i) {
        PackFile_Segment *segment = dir->segments[i];
        /* Prevent repeated destruction */
        dir->segments[i] = NULL;

        if (segment && segment != self)
            PackFile_Segment_destroy(interp, segment);
    }

    if (dir->segments) {
        mem_gc_free(interp, dir->segments);
        dir->segments     = NULL;
        dir->num_segments = 0;
    }
}


/*

=item C<static void sort_segs(PackFile_Directory *dir)>

Sorts the segments in C<dir>.

=cut

*/

static void
sort_segs(ARGMOD(PackFile_Directory *dir))
{
    ASSERT_ARGS(sort_segs)
    const size_t      num_segs = dir->num_segments;
    PackFile_Segment *seg      = dir->segments[0];

    if (seg->type != PF_BYTEC_SEG) {
        size_t i;

        for (i = 1; i < num_segs; ++i) {
            PackFile_Segment * const s2 = dir->segments[i];
            if (s2->type == PF_BYTEC_SEG) {
                dir->segments[0] = s2;
                dir->segments[i] = seg;
                break;
            }
        }
    }


    /* XXX
     * Temporary? hack to put ConstantTable in front of other segments.
     * This is useful for Annotations because we ensure that constants used
     * for keys already available during unpack.
     */
    seg = dir->segments[1];

    if (seg->type != PF_CONST_SEG) {
        size_t i;

        for (i = 3; i < num_segs; ++i) {
            PackFile_Segment * const s2 = dir->segments[i];
            if (s2->type == PF_CONST_SEG) {
                dir->segments[2] = s2;
                dir->segments[i] = seg;
                break;
            }
        }
    }
}


/*

=item C<static size_t directory_packed_size(PARROT_INTERP, PackFile_Segment
*self)>

Returns the size of the directory minus the value returned by
C<default_packed_size()>.

=cut

*/

PARROT_WARN_UNUSED_RESULT
static size_t
directory_packed_size(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
{
    ASSERT_ARGS(directory_packed_size)
    PackFile_Directory * const dir   = (PackFile_Directory *)self;
    const size_t               align = 16 / sizeof (opcode_t);
    size_t size, i;

    /* need bytecode, fixup, other segs ... */
    sort_segs(dir);

    /* number of segments + default, we need it for the offsets */
    size = 1 + default_packed_size(self);

    for (i = 0; i < dir->num_segments; ++i) {
        char * const name = Parrot_str_to_cstring(interp, dir->segments[i]->name);
        /* type, offset, size */
        size += 3;
        size += PF_size_cstring(name);
        Parrot_str_free_cstring(name);
    }

    /* pad/align it */
    if (align && size % align)
        size += (align - size % align);

    for (i = 0; i < dir->num_segments; ++i) {
        size_t seg_size;

        dir->segments[i]->file_offset = size + self->file_offset;
        seg_size                      =
            PackFile_Segment_packed_size(interp, dir->segments[i]);
        dir->segments[i]->op_count    = seg_size;
        size += seg_size;
    }

    self->op_count = size;

    /* subtract default, it is added in PackFile_Segment_packed_size */
    return size - default_packed_size(self);
}


/*

=item C<static opcode_t * directory_pack(PARROT_INTERP, PackFile_Segment *self,
opcode_t *cursor)>

Packs the directory C<self>, using the given cursor.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t *
directory_pack(PARROT_INTERP, ARGIN(PackFile_Segment *self), ARGOUT(opcode_t *cursor))
{
    ASSERT_ARGS(directory_pack)
    PackFile_Directory * const dir      = (PackFile_Directory *)self;
    const size_t               num_segs = dir->num_segments;
    /*const size_t               align    = 16/sizeof (opcode_t);*/
    size_t i;
    PackFile           * const pf       = self->pf;
    opcode_t                 * old_cursor;  /* Used for filling padding with 0 */

    *cursor++ = num_segs;

    for (i = 0; i < num_segs; i++) {
        const PackFile_Segment * const seg = dir->segments[i];
        char * const name = Parrot_str_to_cstring(interp, seg->name);

        *cursor++ = seg->type;
        cursor = PF_store_cstring(cursor, name);
        *cursor++ = seg->file_offset;
        *cursor++ = seg->op_count;
        Parrot_str_free_cstring(name);
    }

    TRACE_PRINTF_ALIGN(("-ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                      OFFS(pf, cursor), pf->src, cursor));
    old_cursor = cursor;
    ALIGN_16(pf, cursor);
    /* fill padding with zeros */
    while (old_cursor != cursor)
        *old_cursor++ = 0;
    TRACE_PRINTF_ALIGN(("+ALIGN_16: offset=0x%x src=0x%x cursor=0x%x\n",
                      OFFS(pf, cursor), pf->src, cursor));
    /*if (align && (cursor - self->pf->src) % align)
      cursor += align - (cursor - self->pf->src) % align;*/

    /* now pack all segments into new format */
    for (i = 0; i < dir->num_segments; ++i) {
        PackFile_Segment * const seg = dir->segments[i];
        cursor = PackFile_Segment_pack(interp, seg, cursor);
    }

    return cursor;
}


/*

=back

=head2 C<PackFile_Segment> Functions

=over 4

=item C<static void segment_init(PackFile_Segment *self, PackFile *pf, STRING
*name)>

Initializes the segment C<self> with the provided PackFile and the given name.
Note that this duplicates the given name.

=cut

*/

static void
segment_init(ARGOUT(PackFile_Segment *self), ARGIN(PackFile *pf),
        ARGIN(STRING *name))
{
    ASSERT_ARGS(segment_init)
    self->pf          = pf;
    self->type        = PF_UNKNOWN_SEG;
    self->file_offset = 0;
    self->op_count    = 0;
    self->itype       = 0;
    self->size        = 0;
    self->data        = NULL;
    self->id          = 0;
    self->name        = name;
}


/*

=item C<PackFile_Segment * PackFile_Segment_new(PARROT_INTERP, PackFile *pf,
STRING *name, int add)>

Creates a new default section.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
PackFile_Segment *
PackFile_Segment_new(PARROT_INTERP, SHIM(PackFile *pf), SHIM(STRING *name), SHIM(int add))
{
    ASSERT_ARGS(PackFile_Segment_new)
    PackFile_Segment * const seg = mem_gc_allocate_zeroed_typed(interp, PackFile_Segment);

    return seg;
}


/*

=back

=head2 Default Function Implementations

The default functions are called before the segment specific functions
and can read a block of C<opcode_t> data.

=over 4

=item C<static void default_destroy(PARROT_INTERP, PackFile_Segment *self)>

The default destroy function.  Destroys a PackFile_Segment.

=cut

*/

static void
default_destroy(PARROT_INTERP, ARGFREE_NOTNULL(PackFile_Segment *self))
{
    ASSERT_ARGS(default_destroy)
    if (!self->pf->is_mmap_ped && self->data) {
        mem_gc_free(interp, self->data);
        self->data = NULL;
    }

    mem_gc_free(interp, self);
}


/*

=item C<static size_t default_packed_size(const PackFile_Segment *self)>

Returns the default size of the segment C<self>.

=cut

*/

static size_t
default_packed_size(ARGIN(const PackFile_Segment *self))
{
    ASSERT_ARGS(default_packed_size)
    return SEGMENT_HEADER_SIZE + self->size;
}


/*

=item C<static opcode_t * default_pack(const PackFile_Segment *self, opcode_t
*dest)>

Performs the default pack.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t *
default_pack(ARGIN(const PackFile_Segment *self), ARGOUT(opcode_t *dest))
{
    ASSERT_ARGS(default_pack)
    *dest++ = self->op_count;
    *dest++ = self->itype;
    *dest++ = self->id;
    *dest++ = self->size;

    if (self->size)
        STRUCT_COPY_N(dest, self->data, self->size);

    return dest + self->size;
}


/*

=back

=head2 ByteCode

=over 4

=item C<static void byte_code_destroy(PARROT_INTERP, PackFile_Segment *self)>

Destroys the C<PackFile_ByteCode> segment C<self>.

=cut

*/

static void
byte_code_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
{
    ASSERT_ARGS(byte_code_destroy)
    PackFile_ByteCode * const byte_code = (PackFile_ByteCode *)self;

    if (byte_code->op_func_table)
        mem_gc_free(interp, byte_code->op_func_table);
    if (byte_code->op_info_table)
        mem_gc_free(interp, byte_code->op_info_table);
    if (byte_code->op_mapping.libs) {
        opcode_t n_libs = byte_code->op_mapping.n_libs;
        opcode_t i;

        for (i = 0; i < n_libs; i++) {
            mem_gc_free(interp, byte_code->op_mapping.libs[i].table_ops);
            mem_gc_free(interp, byte_code->op_mapping.libs[i].lib_ops);
        }

        mem_gc_free(interp, byte_code->op_mapping.libs);
    }

    if (byte_code->libdeps)
        mem_gc_free(interp, byte_code->libdeps);

    if (byte_code->annotations)
        PackFile_Annotations_destroy(interp, (PackFile_Segment *)byte_code->annotations);

    byte_code->annotations     = NULL;
    byte_code->const_table     = NULL;
    byte_code->debugs          = NULL;
    byte_code->op_func_table   = NULL;
    byte_code->op_info_table   = NULL;
    byte_code->op_mapping.libs = NULL;
    byte_code->libdeps         = NULL;
}


/*

=item C<static PackFile_Segment * byte_code_new(PARROT_INTERP, PackFile *pf,
STRING *name, int add)>

Creates a new C<PackFile_ByteCode> segment.  Ignores C<pf>, C<name>, and C<add>.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment *
byte_code_new(PARROT_INTERP, SHIM(PackFile *pf), SHIM(STRING *name), SHIM(int add))
{
    ASSERT_ARGS(byte_code_new)
    PackFile_ByteCode * const byte_code = mem_gc_allocate_zeroed_typed(interp, PackFile_ByteCode);

    return (PackFile_Segment *) byte_code;
}

/*

=item C<static size_t byte_code_packed_size(PARROT_INTERP, PackFile_Segment
*self)>

Computes the size in multiples of C<opcode_t> required to store the passed
C<PackFile_ByteCode>.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_PURE_FUNCTION
static size_t
byte_code_packed_size(SHIM_INTERP, ARGIN(PackFile_Segment *self))
{
    ASSERT_ARGS(byte_code_packed_size)
    PackFile_ByteCode * const byte_code = (PackFile_ByteCode *)self;
    size_t size;
    int i;
    unsigned int u;

    size = 3; /* op_count + n_libs + n_libdeps*/

    for (u = 0; u < byte_code->n_libdeps; u++)
        size += PF_size_string(byte_code->libdeps[u]);

    for (i = 0; i < byte_code->op_mapping.n_libs; i++) {
        PackFile_ByteCode_OpMappingEntry * const entry = &byte_code->op_mapping.libs[i];

        /* dynoplib data */
        size += PF_size_cstring(entry->lib->name);
        size += 3; /* major + minor + patch */

        /* op entries */
        size += 1;                /* n_ops */
        size += entry->n_ops * 2; /* lib_ops and table_ops */
    }

    return size;
}

/*

=item C<static opcode_t * byte_code_pack(PARROT_INTERP, PackFile_Segment *self,
opcode_t *cursor)>

Stores the passed C<PackFile_ByteCode> segment in bytecode.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t *
byte_code_pack(SHIM_INTERP, ARGMOD(PackFile_Segment *self), ARGOUT(opcode_t *cursor))
{
    ASSERT_ARGS(byte_code_pack)
    PackFile_ByteCode * const byte_code = (PackFile_ByteCode *)self;
    int i;
    unsigned int u;

    *cursor++ = byte_code->n_libdeps;
    *cursor++ = byte_code->op_count;
    *cursor++ = byte_code->op_mapping.n_libs;

    for (u = 0; u < byte_code->n_libdeps; u++)
        cursor = PF_store_string(cursor, byte_code->libdeps[u]);

    for (i = 0; i < byte_code->op_mapping.n_libs; i++) {
        int j;

        PackFile_ByteCode_OpMappingEntry * const entry = &byte_code->op_mapping.libs[i];

        /* dynoplib data */
        cursor    = PF_store_cstring(cursor, entry->lib->name);
        *cursor++ = entry->lib->major_version;
        *cursor++ = entry->lib->minor_version;
        *cursor++ = entry->lib->patch_version;

        /* op entries */
        *cursor++ = entry->n_ops;
        for (j = 0; j < entry->n_ops; j++) {
            *cursor++ = entry->table_ops[j];
            *cursor++ = entry->lib_ops[j];
        }
    }

    return cursor;
}

/*

=item C<static const opcode_t * byte_code_unpack(PARROT_INTERP, PackFile_Segment
*self, const opcode_t *cursor)>

Unpacks a bytecode segment into the passed C<PackFile_ByteCode>.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static const opcode_t *
byte_code_unpack(PARROT_INTERP, ARGMOD(PackFile_Segment *self), ARGIN(const opcode_t *cursor))
{
    ASSERT_ARGS(byte_code_unpack)
    PackFile_ByteCode * const byte_code = (PackFile_ByteCode *)self;
    int i;
    unsigned int u;
    size_t total_ops = 0;

    byte_code->n_libdeps         = PF_fetch_opcode(self->pf, &cursor);
    byte_code->libdeps           = mem_gc_allocate_n_zeroed_typed(interp,
                                        byte_code->n_libdeps, STRING *);

    byte_code->op_count          = PF_fetch_opcode(self->pf, &cursor);
    byte_code->op_func_table     = mem_gc_allocate_n_zeroed_typed(interp,
                                        byte_code->op_count, op_func_t);
    byte_code->op_info_table     = mem_gc_allocate_n_zeroed_typed(interp,
                                        byte_code->op_count, op_info_t *);


    byte_code->op_mapping.n_libs = PF_fetch_opcode(self->pf, &cursor);
    byte_code->op_mapping.libs   = mem_gc_allocate_n_zeroed_typed(interp,
                                    byte_code->op_mapping.n_libs,
                                    PackFile_ByteCode_OpMappingEntry);

    for (u = 0; u < byte_code->n_libdeps; u++) {
        STRING *libname = PF_fetch_string(interp, self->pf, &cursor);
        PMC    *lib_pmc = Parrot_dyn_load_lib(interp, libname, NULL);
        byte_code->libdeps[u] = libname;
    }

    for (i = 0; i < byte_code->op_mapping.n_libs; i++) {
        PackFile_ByteCode_OpMappingEntry * const entry = &byte_code->op_mapping.libs[i];

        /* dynoplib data */
        {
            char * const    lib_name = PF_fetch_cstring(interp, self->pf, &cursor);
            const opcode_t  major    = PF_fetch_opcode(self->pf, &cursor);
            const opcode_t  minor    = PF_fetch_opcode(self->pf, &cursor);
            const opcode_t  patch    = PF_fetch_opcode(self->pf, &cursor);

            /* XXX
             * broken encapsulation => should make this data easier to access somehow
             */
            if (STREQ(lib_name, PARROT_CORE_OPLIB_NAME)) {
                entry->lib = PARROT_CORE_OPLIB_INIT(interp, 1);
            }
            else {
                PMC *lib_pmc = Parrot_dyn_load_lib(interp,
                                                Parrot_str_new(interp, lib_name, 0),
                                                NULL);
                typedef op_lib_t *(*oplib_init_t)(PARROT_INTERP, long init);
                void *oplib_init;
                oplib_init_t oplib_init_f;
                if (!VTABLE_get_bool(interp, lib_pmc))
                    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
                        "Could not load oplib `%s'", lib_name);
                GETATTR_ParrotLibrary_oplib_init(interp, lib_pmc, oplib_init);
                oplib_init_f = (oplib_init_t)D2FPTR(oplib_init);
                entry->lib = oplib_init_f(interp, 1);
            }


            mem_gc_free(interp, lib_name);

            if (entry->lib->major_version != major
            ||  entry->lib->minor_version != minor
            ||  entry->lib->patch_version != patch)
                Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
                    "Incompatible versions of `%s' oplib. Found %d.%d.%d but loaded %d.%d.%d",
                    entry->lib->name, major, minor, patch, entry->lib->major_version,
                    entry->lib->minor_version, entry->lib->patch_version);
        }

        /* op entries */
        {
            int       j;
            total_ops += entry->n_ops = PF_fetch_opcode(self->pf, &cursor);

            entry->table_ops = mem_gc_allocate_n_zeroed_typed(interp,
                                    entry->n_ops, opcode_t);
            entry->lib_ops   = mem_gc_allocate_n_zeroed_typed(interp,
                                    entry->n_ops, opcode_t);

            for (j = 0; j < entry->n_ops; j++) {
                opcode_t idx = PF_fetch_opcode(self->pf, &cursor);
                opcode_t op  = PF_fetch_opcode(self->pf, &cursor);

                if (0 > op || op >= entry->lib->op_count)
                    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_MALFORMED_PACKFILE,
                        "opcode index out of bounds on library `%s'. Found %d, expected 0 to %d.",
                        entry->lib->name, op, entry->lib->op_count - 1);

                if (0 > idx || (size_t)idx >= byte_code->op_count)
                    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_MALFORMED_PACKFILE,
                        "op table index out of bounds for entry from library `%s'."
                        " Found %d, expected 0 to %d",
                        entry->lib->name, idx, byte_code->op_count - 1);

                if (byte_code->op_func_table[idx])
                    Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_MALFORMED_PACKFILE,
                        "duplicate entries in optable");

                entry->table_ops[j]           = idx;
                entry->lib_ops[j]             = op;
                byte_code->op_func_table[idx] = entry->lib->op_func_table[op];
                byte_code->op_info_table[idx] = &entry->lib->op_info_table[op];
            }
        }
    }

    if (total_ops != byte_code->op_count)
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_MALFORMED_PACKFILE,
            "wrong number of ops decoded for optable. Decoded %d, but expected %d",
            total_ops, byte_code->op_count);

    return cursor;
}

/*

=back

=head2 Debug Info

=over 4

=item C<static void pf_debug_destroy(PARROT_INTERP, PackFile_Segment *self)>

Destroys the C<PackFile_Debug> segment C<self>.

=cut

*/

static void
pf_debug_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
{
    ASSERT_ARGS(pf_debug_destroy)
    PackFile_Debug * const debug = (PackFile_Debug *) self;

    /* Free mappings pointer array. */
    mem_gc_free(interp, debug->mappings);
    debug->mappings     = NULL;
    debug->num_mappings = 0;
}


/*

=item C<static PackFile_Segment * pf_debug_new(PARROT_INTERP, PackFile *pf,
STRING *name, int add)>

Creates and returns a new C<PackFile_Debug> segment.  Ignores C<pf>, C<name>,
and C<add> ignored.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment *
pf_debug_new(PARROT_INTERP, SHIM(PackFile *pf), SHIM(STRING *name), SHIM(int add))
{
    ASSERT_ARGS(pf_debug_new)
    PackFile_Debug * const debug = mem_gc_allocate_zeroed_typed(interp, PackFile_Debug);

    /* don't create initial mappings here; they'll get overwritten later */

    return (PackFile_Segment *)debug;
}


/*

=item C<static size_t pf_debug_packed_size(PARROT_INTERP, PackFile_Segment
*self)>

Returns the size of the C<PackFile_Debug> segment's filename in C<opcode_t>
units.

=cut

*/

static size_t
pf_debug_packed_size(SHIM_INTERP, ARGIN(PackFile_Segment *self))
{
    ASSERT_ARGS(pf_debug_packed_size)
    PackFile_Debug * const debug = (PackFile_Debug *)self;

    return (debug->num_mappings*2) + 1;
}


/*

=item C<static opcode_t * pf_debug_pack(PARROT_INTERP, PackFile_Segment *self,
opcode_t *cursor)>

Packs the debug segment, using the given cursor.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static opcode_t *
pf_debug_pack(PARROT_INTERP, ARGMOD(PackFile_Segment *self), ARGOUT(opcode_t *cursor))
{
    ASSERT_ARGS(pf_debug_pack)
    PackFile_Debug * const debug = (PackFile_Debug *)self;
    const              int n     = debug->num_mappings;
    int i;

    if (n > 0 && debug->mappings == NULL)
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_MALFORMED_PACKFILE,
                                    "No mappings but non zero num mappings(%I)", n);

    /* Store number of mappings. */
    *cursor++ = n;

    /* Now store each mapping. */
    for (i = 0; i < n; ++i) {
        /* Bytecode offset and filename. */
        *cursor++ = debug->mappings[i].offset;
        *cursor++ = debug->mappings[i].filename;
    }

    return cursor;
}


/*

=item C<static const opcode_t * pf_debug_unpack(PARROT_INTERP, PackFile_Segment
*self, const opcode_t *cursor)>

Unpacks a debug segment into a PackFile_Debug structure, given the cursor.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static const opcode_t *
pf_debug_unpack(PARROT_INTERP, ARGOUT(PackFile_Segment *self), ARGIN(const opcode_t *cursor))
{
    ASSERT_ARGS(pf_debug_unpack)
    PackFile_Debug * const debug = (PackFile_Debug *)self;
    PackFile_ByteCode     *code;
    int                    i;

    /* For some reason, we store the source file name in the segment
       name. So we can't find the bytecode seg without knowing the filename.
       But with the new scheme we can have many file names. For now, just
       base this on the name of the debug segment. */
    STRING *code_name = NULL;
    size_t str_len;

    /* Number of mappings. */
    debug->num_mappings = PF_fetch_opcode(self->pf, &cursor);

    /* Allocate space for mappings vector. */
    debug->mappings = mem_gc_allocate_n_zeroed_typed(interp,
            debug->num_mappings, PackFile_DebugFilenameMapping);

    /* Read in each mapping. */
    for (i = 0; i < debug->num_mappings; ++i) {
        /* Get offset and filename type. */
        debug->mappings[i].offset   = PF_fetch_opcode(self->pf, &cursor);
        debug->mappings[i].filename = PF_fetch_opcode(self->pf, &cursor);
    }

    /* find seg e.g. CODE_DB => CODE and attach it */
    str_len     = Parrot_str_length(interp, debug->base.name);
    code_name   = STRING_substr(interp, debug->base.name, 0, str_len - 3);
    code        = (PackFile_ByteCode *)PackFile_find_segment(interp, self->dir, code_name, 0);

    if (!code || code->base.type != PF_BYTEC_SEG) {
        Parrot_ex_throw_from_c_args(interp, NULL, 1,
            "Code '%Ss' not found for debug segment '%Ss'\n",
            code_name, self->name);
    }

    code->debugs = debug;
    debug->code  = code;

    return cursor;
}


/*

=item C<static void pf_debug_dump(PARROT_INTERP, const PackFile_Segment *self)>

Dumps a debug segment to a human readable form.

=cut

*/

static void
pf_debug_dump(PARROT_INTERP, ARGIN(const PackFile_Segment *self))
{
    ASSERT_ARGS(pf_debug_dump)
    const PackFile_Debug * const debug = (const PackFile_Debug *)self;

    opcode_t i;

    default_dump_header(interp, self);

    Parrot_io_printf(interp, "\n  mappings => [\n");
    for (i = 0; i < debug->num_mappings; ++i) {
        Parrot_io_printf(interp, "    #%d\n    [\n", i);
        Parrot_io_printf(interp, "        OFFSET => %d,\n",
                   debug->mappings[i].offset);
        Parrot_io_printf(interp, "        FILENAME => %Ss\n",
                debug->code->const_table->str.constants[debug->mappings[i].filename]);
        Parrot_io_printf(interp, "    ],\n");
    }

    Parrot_io_printf(interp, "  ]\n");
}


/*

=item C<PackFile_Debug * Parrot_new_debug_seg(PARROT_INTERP, PackFile_ByteCode
*cs, size_t size)>

Creates and appends (or resizes) a new debug seg for a code segment.  Uses the
given size as its size.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
PackFile_Debug *
Parrot_new_debug_seg(PARROT_INTERP, ARGMOD(PackFile_ByteCode *cs), size_t size)
{
    ASSERT_ARGS(Parrot_new_debug_seg)
    PackFile_Debug *debug;

    /* it exists already, resize it */
    if (cs->debugs) {
        debug = cs->debugs;
        debug->base.data = mem_gc_realloc_n_typed(interp, debug->base.data, size, opcode_t);
    }
    /* create one */
    else {
        STRING * name;
        const int add     = (interp->code && interp->code->base.dir);
        PackFile_Directory * const dir = add
                ? interp->code->base.dir
                : cs->base.dir
                    ? cs->base.dir
                    : &interp->initial_pf->directory;

        name = Parrot_sprintf_c(interp, "%Ss_DB", cs->base.name);
        debug = (PackFile_Debug *)PackFile_Segment_new_seg(interp, dir,
                                    PF_DEBUG_SEG, name, add);

        debug->base.data = mem_gc_allocate_n_zeroed_typed(interp, size, opcode_t);
        debug->code      = cs;
        cs->debugs       = debug;
    }

    debug->base.size = size;

    return debug;
}


/*

=item C<void Parrot_debug_add_mapping(PARROT_INTERP, PackFile_Debug *debug,
opcode_t offset, const char *filename)>

Adds a bytecode offset to a filename mapping for a PackFile_Debug.

=cut

*/

PARROT_EXPORT
void
Parrot_debug_add_mapping(PARROT_INTERP, ARGMOD(PackFile_Debug *debug),
                         opcode_t offset, ARGIN(const char *filename))
{
    ASSERT_ARGS(Parrot_debug_add_mapping)
    PackFile_ConstTable * const    ct         = debug->code->const_table;
    int                            insert_pos = 0;
    opcode_t                       prev_filename_n;
    STRING                        *filename_pstr;

    /* If the previous mapping has the same filename, don't record it. */
    if (debug->num_mappings) {
        prev_filename_n = debug->mappings[debug->num_mappings-1].filename;
        filename_pstr = Parrot_str_new(interp, filename, 0);
        if (ct->str.constants[prev_filename_n] &&
                STRING_equal(interp, filename_pstr,
                    ct->str.constants[prev_filename_n])) {
            return;
        }
    }

    /* Allocate space for the extra entry. */
    debug->mappings = mem_gc_realloc_n_typed(interp,
            debug->mappings, debug->num_mappings + 1,
            PackFile_DebugFilenameMapping);

    /* Can it just go on the end? */
    if (debug->num_mappings == 0
    ||  offset              >= debug->mappings[debug->num_mappings - 1].offset)
        insert_pos = debug->num_mappings;
    else {
        /* Find the right place and shift stuff that's after it. */
        int i;

        for (i = 0; i < debug->num_mappings; ++i) {
            if (debug->mappings[i].offset > offset) {
                insert_pos = i;
                memmove(debug->mappings + i + 1, debug->mappings + i,
                    debug->num_mappings - i);
                break;
            }
        }
    }

    /* Need to put filename in constants table. */
    {
        /* Set up new entry and insert it. */
        PackFile_DebugFilenameMapping *mapping = debug->mappings + insert_pos;
        STRING *namestr = Parrot_str_new_init(interp, filename, strlen(filename),
                Parrot_default_encoding_ptr, 0);
        size_t count = ct->str.const_count;
        size_t i;

        mapping->offset = offset;

        /* Check if there is already a constant with this filename */
        for (i= 0; i < count; ++i) {
            if (STRING_equal(interp, namestr, ct->str.constants[i]))
                break;
        }
        if (i < count) {
            /* There is one, use it */
            count = i;
       }
       else {
            /* Not found, create a new one */
            ct->str.const_count++;
            ct->str.constants = mem_gc_realloc_n_typed_zeroed(interp, ct->str.constants,
                    ct->str.const_count, ct->str.const_count - 1, STRING *);
            ct->str.constants[ct->str.const_count - 1] =
                Parrot_str_new_init(interp, filename, strlen(filename),
                    Parrot_default_encoding_ptr,
                    PObj_constant_FLAG);
        }

        /* Set the mapped value */
        mapping->filename = count;
        debug->num_mappings         = debug->num_mappings + 1;
    }
}


/*

=item C<STRING * Parrot_debug_pc_to_filename(PARROT_INTERP, const PackFile_Debug
*debug, opcode_t pc)>

Returns the filename of the source for the given position in the bytecode.

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
STRING *
Parrot_debug_pc_to_filename(PARROT_INTERP, ARGIN(const PackFile_Debug *debug),
    opcode_t pc)
{
    ASSERT_ARGS(Parrot_debug_pc_to_filename)
    /* Look through mappings until we find one that maps the passed
       bytecode offset. */

    int i;
    for (i = 0; i < debug->num_mappings; ++i) {
        /* If this is the last mapping or the current position is
           between this mapping and the next one, return a filename. */
       if (i + 1                          == debug->num_mappings
       || (debug->mappings[i].offset     <= pc
       &&  debug->mappings[i + 1].offset >  pc))
            return debug->code->const_table->str.constants[debug->mappings[i].filename];
    }

    /* Otherwise, no mappings == no filename. */
    return CONST_STRING(interp, "(unknown file)");
}


/*

=item C<void Parrot_switch_to_cs_by_nr(PARROT_INTERP, opcode_t seg)>

Switches the current bytecode segment to the segment keyed by number C<seg>.

=cut

*/

PARROT_EXPORT
void
Parrot_switch_to_cs_by_nr(PARROT_INTERP, opcode_t seg)
{
    ASSERT_ARGS(Parrot_switch_to_cs_by_nr)
    const PackFile_Directory * const dir      = interp->code->base.dir;
    const size_t                     num_segs = dir->num_segments;

    size_t   i;
    opcode_t n;

    /* TODO make an index of code segments for faster look up */
    for (i = n = 0; i < num_segs; ++i) {
        if (dir->segments[i]->type == PF_BYTEC_SEG) {
            if (n == seg) {
                Parrot_switch_to_cs(interp, (PackFile_ByteCode *)
                        dir->segments[i], 1);
                return;
            }
            ++n;
        }
    }

    Parrot_ex_throw_from_c_args(interp, NULL, 1,
        "Segment number %d not found\n", (int)seg);
}


/*

=item C<PackFile_ByteCode * Parrot_switch_to_cs(PARROT_INTERP, PackFile_ByteCode
*new_cs, int really)>

Switches to a bytecode segment C<new_cs>, returning the old segment.

=cut

*/

PARROT_EXPORT
PARROT_IGNORABLE_RESULT
PARROT_CANNOT_RETURN_NULL
PackFile_ByteCode *
Parrot_switch_to_cs(PARROT_INTERP, ARGIN(PackFile_ByteCode *new_cs), int really)
{
    ASSERT_ARGS(Parrot_switch_to_cs)
    PackFile_ByteCode * const cur_cs = interp->code;

    if (!new_cs)
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_NO_PREV_CS,
            "No code segment to switch to\n");

    /* compiling source code uses this function too,
     * which gives misleading trace messages */
    if (really && Interp_trace_TEST(interp, PARROT_TRACE_SUB_CALL_FLAG)) {
        Interp * const tracer = interp->pdb && interp->pdb->debugger
                              ? interp->pdb->debugger
                              : interp;
        Parrot_io_eprintf(tracer, "*** switching to %s\n", new_cs->base.name);
    }

    interp->code               = new_cs;

    Parrot_pcc_set_constants(interp, CURRENT_CONTEXT(interp), really
                               ? find_constants(interp, new_cs->const_table)
                               : new_cs->const_table);

    if (really)
        prepare_for_run(interp);

    return cur_cs;
}


/*

=item C<static void clone_constant(PARROT_INTERP, PMC **c)>

Clones a constant (at least, if it's a Sub PMC), returning the clone.

=cut

*/

static void
clone_constant(PARROT_INTERP, ARGIN(PMC **c))
{
    ASSERT_ARGS(clone_constant)
    STRING * const _sub = CONST_STRING(interp, "Sub");

    if (VTABLE_isa(interp, *c, _sub)) {
        PMC                   *old_sub_pmc, *new_sub_pmc;
        Parrot_Sub_attributes *old_sub,     *new_sub;

        old_sub_pmc   = *c;
        new_sub_pmc   = Parrot_thaw_constants(interp, Parrot_freeze(interp, old_sub_pmc));

        PMC_get_sub(interp, new_sub_pmc, new_sub);
        PMC_get_sub(interp, old_sub_pmc, old_sub);
        new_sub->seg = old_sub->seg;

        /* Vtable overrides and methods were already cloned, so don't reclone them. */
        if (new_sub->vtable_index == -1
        && !(old_sub->comp_flags   &  SUB_COMP_FLAG_METHOD))
            Parrot_ns_store_sub(interp, new_sub_pmc);

        *c = new_sub_pmc;
    }
}


/*

=item C<static PackFile_ConstTable * find_constants(PARROT_INTERP,
PackFile_ConstTable *ct)>

Finds the constant table associated with a thread. For now, we need to copy
constant tables because some entries aren't really constant; e.g. subroutines
need to refer to namespace pointers.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PackFile_ConstTable *
find_constants(PARROT_INTERP, ARGIN(PackFile_ConstTable *ct))
{
    ASSERT_ARGS(find_constants)
    if (!n_interpreters
    ||  !interp->thread_data
    ||  interp->thread_data->tid == 0)
        return ct;
    else {
        Hash                 *tables;
        PackFile_ConstTable  *new_ct;

        PARROT_ASSERT(interp->thread_data);

        if (!interp->thread_data->const_tables) {
            interp->thread_data->const_tables = parrot_new_pointer_hash(interp);
        }

        tables = interp->thread_data->const_tables;
        new_ct = (PackFile_ConstTable *)parrot_hash_get(interp, tables, ct);

        if (!new_ct) {
            /* need to construct it */

            int i;

            new_ct = mem_gc_allocate_zeroed_typed(interp, PackFile_ConstTable);

            new_ct->num.const_count = ct->num.const_count;
            new_ct->num.constants = mem_gc_allocate_n_zeroed_typed(interp,
                                        ct->num.const_count, FLOATVAL);
            memcpy(new_ct->num.constants, ct->num.constants,
                    ct->num.const_count * sizeof (FLOATVAL));

            new_ct->str.const_count = ct->str.const_count;
            new_ct->str.constants = mem_gc_allocate_n_zeroed_typed(interp,
                                        ct->str.const_count, STRING *);
            memcpy(new_ct->str.constants, ct->str.constants,
                    ct->str.const_count * sizeof (STRING *));

            new_ct->pmc.const_count = ct->pmc.const_count;
            new_ct->pmc.constants = mem_gc_allocate_n_zeroed_typed(interp,
                                        ct->pmc.const_count, PMC *);
            memcpy(new_ct->pmc.constants, ct->pmc.constants,
                    ct->pmc.const_count * sizeof (PMC *));
            for (i = 0; i < new_ct->pmc.const_count; ++i)
                clone_constant(interp, &new_ct->pmc.constants[i]);

            parrot_hash_put(interp, tables, ct, new_ct);
        }

        return new_ct;
    }
}


/*

=item C<void Parrot_destroy_constants(PARROT_INTERP)>

Destroys the constants for an interpreter.

=cut

*/

PARROT_EXPORT
void
Parrot_destroy_constants(PARROT_INTERP)
{
    ASSERT_ARGS(Parrot_destroy_constants)
    Hash *hash;

    return;

    if (!interp->thread_data)
        return;

    hash = interp->thread_data->const_tables;

    if (!hash)
        return;

    parrot_hash_iterate(hash,
        PackFile_ConstTable * const ct        = (PackFile_ConstTable *)_bucket->value;
        PackFile_ConstTable_clear(interp, ct);
        mem_gc_free(interp, ct););
    parrot_hash_destroy(interp, hash);
}

/*

=back

=head2 PackFile ConstTable Structure Functions

=over 4

=item C<void PackFile_ConstTable_clear(PARROT_INTERP, PackFile_ConstTable
*self)>

Clear the C<PackFile_ConstTable> C<self>.

=cut

*/

PARROT_EXPORT
void
PackFile_ConstTable_clear(PARROT_INTERP, ARGMOD(PackFile_ConstTable *self))
{
    ASSERT_ARGS(PackFile_ConstTable_clear)

    if (self->num.constants) {
        mem_gc_free(interp, self->num.constants);
        self->num.constants = NULL;
    }

    if (self->str.constants) {
        mem_gc_free(interp, self->str.constants);
        self->str.constants = NULL;
    }

    if (self->pmc.constants) {
        mem_gc_free(interp, self->pmc.constants);
        self->pmc.constants = NULL;
    }

    if (self->string_hash) {
        parrot_hash_destroy(interp, self->string_hash);
        self->string_hash = NULL;
    }

    return;
}


/*

=item C<const opcode_t * PackFile_ConstTable_unpack(PARROT_INTERP,
PackFile_Segment *seg, const opcode_t *cursor)>

Unpacks a PackFile ConstTable from a block of memory. The format is:

  opcode_t const_count
  *  constants

Returns cursor if everything is OK, else zero (0).

=cut

*/

PARROT_EXPORT
PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
const opcode_t *
PackFile_ConstTable_unpack(PARROT_INTERP, ARGIN(PackFile_Segment *seg),
        ARGIN(const opcode_t *cursor))
{
    ASSERT_ARGS(PackFile_ConstTable_unpack)
    PackFile_ConstTable * const self = (PackFile_ConstTable *)seg;
    PackFile            * const pf   = seg->pf;
    opcode_t                    i;

    PackFile_ConstTable_clear(interp, self);

    self->num.const_count = PF_fetch_opcode(pf, &cursor);
    self->str.const_count = PF_fetch_opcode(pf, &cursor);
    self->pmc.const_count = PF_fetch_opcode(pf, &cursor);

    if (self->num.const_count) {
        self->num.constants = mem_gc_allocate_n_zeroed_typed(interp,
                                    self->num.const_count, FLOATVAL);
        if (!self->num.constants)
            goto err;
    }

    if (self->str.const_count) {
        self->str.constants = mem_gc_allocate_n_zeroed_typed(interp,
                                    self->str.const_count, STRING *);
        if (!self->str.constants)
            goto err;
    }

    if (self->pmc.const_count) {
        self->pmc.constants = mem_gc_allocate_n_zeroed_typed(interp,
                                    self->pmc.const_count, PMC *);
        if (!self->pmc.constants)
            goto err;
    }

    for (i = 0; i < self->num.const_count; i++)
        self->num.constants[i] = PF_fetch_number(pf, &cursor);

    for (i = 0; i < self->str.const_count; i++)
        self->str.constants[i] = PF_fetch_string(interp, pf, &cursor);

    for (i = 0; i < self->pmc.const_count; i++)
        self->pmc.constants[i] = PackFile_Constant_unpack_pmc(interp, self, &cursor);

    return cursor;

  err:
    Parrot_io_eprintf(interp,
        "PackFile_ConstTable_unpack: Could not allocate memory for array!\n");
    PackFile_ConstTable_clear(interp, self);
    return NULL;
}


/*

=item C<static PackFile_Segment * const_new(PARROT_INTERP, PackFile *pf, STRING
*name, int add)>

Returns a new C<PackFile_ConstTable> segment.

=cut

*/

PARROT_MALLOC
PARROT_CANNOT_RETURN_NULL
static PackFile_Segment *
const_new(PARROT_INTERP, SHIM(PackFile *pf), SHIM(STRING *name), SHIM(int add))
{
    ASSERT_ARGS(const_new)
    PackFile_ConstTable * const const_table =
            mem_gc_allocate_zeroed_typed(interp, PackFile_ConstTable);

    return (PackFile_Segment *)const_table;
}


/*

=item C<static void const_destroy(PARROT_INTERP, PackFile_Segment *self)>

Destroys the C<PackFile_ConstTable> C<self>.

=cut

*/

static void
const_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *self))
{
    ASSERT_ARGS(const_destroy)
    PackFile_ConstTable * const ct = (PackFile_ConstTable *)self;
    PackFile_ConstTable_clear(interp, ct);
}


/*

=item C<static PMC * PackFile_Constant_unpack_pmc(PARROT_INTERP,
PackFile_ConstTable *constt, const opcode_t **cursor)>

Unpacks a constant PMC.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
static PMC *
PackFile_Constant_unpack_pmc(PARROT_INTERP, ARGIN(PackFile_ConstTable *constt),
        ARGIN(const opcode_t **cursor))
{
    ASSERT_ARGS(PackFile_Constant_unpack_pmc)
    PackFile * const pf         = constt->base.pf;
    STRING          *_sub       = CONST_STRING(interp, "Sub");
    PMC             *pmc;
    /* thawing the PMC needs the real packfile in place */
    PackFile_ByteCode * const cs_save = interp->code;
    interp->code                      = pf->cur_cs;
    pmc                               = Parrot_thaw_pbc(interp, constt, cursor);

    /* finally place the sub into some namespace stash
     * XXX place this code in Sub.thaw ?  */
    if (VTABLE_isa(interp, pmc, _sub))
        Parrot_ns_store_sub(interp, pmc);

    /* restore code */
    interp->code = cs_save;

    return pmc;
}


/*

=item C<PackFile_Segment * PackFile_Annotations_new(PARROT_INTERP, struct
PackFile *pf, STRING *name, int add)>

Creates a new annotations segment structure. Ignores the parameters C<name> and
C<add>.

=cut

*/
PARROT_EXPORT
PARROT_CANNOT_RETURN_NULL
PackFile_Segment *
PackFile_Annotations_new(PARROT_INTERP, SHIM(struct PackFile *pf),
        SHIM(STRING *name), SHIM(int add))
{
    ASSERT_ARGS(PackFile_Annotations_new)

    /* Allocate annotations structure; create it all zeroed, and we will
     * allocate memory for each of the arrays on demand. */
    PackFile_Annotations * const seg = mem_gc_allocate_zeroed_typed(interp,
            PackFile_Annotations);
    return (PackFile_Segment *) seg;
}


/*

=item C<void PackFile_Annotations_destroy(PARROT_INTERP, PackFile_Segment *seg)>

Frees all memory associated with an annotations segment.

=cut

*/

void
PackFile_Annotations_destroy(PARROT_INTERP, ARGMOD(PackFile_Segment *seg))
{
    ASSERT_ARGS(PackFile_Annotations_destroy)
    PackFile_Annotations * const self = (PackFile_Annotations *)seg;

    /* Free any keys. */
    if (self->keys)
        mem_gc_free(interp, self->keys);

    /* Free any groups. */
    if (self->groups)
        mem_gc_free(interp, self->groups);

    /* Free any entries. */
    if (self->entries)
        mem_gc_free(interp, self->entries);

    self->keys    = NULL;
    self->groups  = NULL;
    self->entries = NULL;
}


/*

=item C<size_t PackFile_Annotations_packed_size(PARROT_INTERP, PackFile_Segment
*seg)>

Computes the number of opcode_ts needed to store the passed annotations
segment.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_PURE_FUNCTION
size_t
PackFile_Annotations_packed_size(SHIM_INTERP, ARGIN(PackFile_Segment *seg))
{
    ASSERT_ARGS(PackFile_Annotations_packed_size)
    const PackFile_Annotations * const self = (PackFile_Annotations *)seg;
    return 3                      /* Counts. */
         + self->num_keys    * 2  /* Keys. */
         + self->num_groups  * 2  /* Groups. */
         + self->num_entries * 3; /* Entries. */
}


/*

=item C<opcode_t * PackFile_Annotations_pack(PARROT_INTERP, PackFile_Segment
*seg, opcode_t *cursor)>

Packs this segment into bytecode.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CANNOT_RETURN_NULL
opcode_t *
PackFile_Annotations_pack(SHIM_INTERP, ARGIN(PackFile_Segment *seg),
        ARGMOD(opcode_t *cursor))
{
    ASSERT_ARGS(PackFile_Annotations_pack)
    const PackFile_Annotations * const self = (PackFile_Annotations *)seg;
    INTVAL i;

    /* Write key count and any keys. */
    *cursor++ = self->num_keys;

    for (i = 0; i < self->num_keys; ++i) {
        const PackFile_Annotations_Key * const key = self->keys + i;
        *cursor++ = key->name;
        *cursor++ = key->type;
    }

    /* Write group count and any groups. */
    *cursor++ = self->num_groups;

    for (i = 0; i < self->num_groups; ++i) {
        const PackFile_Annotations_Group * const group = self->groups + i;
        *cursor++ = group->bytecode_offset;
        *cursor++ = group->entries_offset;
    }

    /* Write entry count and any entries. */
    *cursor++ = self->num_entries;

    for (i = 0; i < self->num_entries; ++i) {
        const PackFile_Annotations_Entry * const entry = self->entries + i;
        *cursor++ = entry->bytecode_offset;
        *cursor++ = entry->key;
        *cursor++ = entry->value;
    }

    return cursor;
}


/*

=item C<const opcode_t * PackFile_Annotations_unpack(PARROT_INTERP,
PackFile_Segment *seg, const opcode_t *cursor)>

Unpacks this segment from the bytecode.

=cut

*/

PARROT_CANNOT_RETURN_NULL
const opcode_t *
PackFile_Annotations_unpack(PARROT_INTERP, ARGMOD(PackFile_Segment *seg),
        ARGIN(const opcode_t *cursor))
{
    ASSERT_ARGS(PackFile_Annotations_unpack)
    PackFile_Annotations * const self = (PackFile_Annotations *)seg;
    PackFile_ByteCode    *code;
    STRING               *code_name;
#if TRACE_PACKFILE
    PackFile * const pf  = seg->pf;
#endif
    INTVAL               i, str_len;

    /* Unpack keys. */
    self->num_keys = PF_fetch_opcode(seg->pf, &cursor);

    TRACE_PRINTF(("PackFile_Annotations_unpack: Unpacking %ld keys\n",
                  self->num_keys));

    self->keys     = mem_gc_allocate_n_zeroed_typed(interp,
            self->num_keys, PackFile_Annotations_Key);

    for (i = 0; i < self->num_keys; ++i) {
        PackFile_Annotations_Key * const key = self->keys + i;
        key->name = PF_fetch_opcode(seg->pf, &cursor);
        key->type = PF_fetch_opcode(seg->pf, &cursor);
        TRACE_PRINTF_VAL(("PackFile_Annotations_unpack: key[%d]/%d name=%s type=%d\n",
              i, self->num_keys, key->name, key->type));
    }

    /* Unpack groups. */
    self->num_groups = PF_fetch_opcode(seg->pf, &cursor);
    self->groups     = mem_gc_allocate_n_zeroed_typed(interp,
            self->num_groups, PackFile_Annotations_Group);

    for (i = 0; i < self->num_groups; ++i) {
        PackFile_Annotations_Group * const group = self->groups + i;
        group->bytecode_offset = PF_fetch_opcode(seg->pf, &cursor);
        group->entries_offset  = PF_fetch_opcode(seg->pf, &cursor);
        TRACE_PRINTF_VAL((
           "PackFile_Annotations_unpack: group[%d]/%d bytecode_offset=%d entries_offset=%d\n",
           i, self->num_groups, group->bytecode_offset,
           group->entries_offset));
    }

    /* Unpack entries. */
    self->num_entries = PF_fetch_opcode(seg->pf, &cursor);
    self->entries     = mem_gc_allocate_n_zeroed_typed(interp,
            self->num_entries, PackFile_Annotations_Entry);

    for (i = 0; i < self->num_entries; ++i) {
        PackFile_Annotations_Entry * const entry = self->entries + i;
        entry->bytecode_offset = PF_fetch_opcode(seg->pf, &cursor);
        entry->key             = PF_fetch_opcode(seg->pf, &cursor);
        entry->value           = PF_fetch_opcode(seg->pf, &cursor);
    }

    /* Need to associate this segment with the applicable code segment. */
    str_len     = Parrot_str_length(interp, self->base.name);
    code_name   = STRING_substr(interp, self->base.name, 0, str_len - 4);
    code        = (PackFile_ByteCode *)PackFile_find_segment(interp,
                                self->base.dir, code_name, 0);

    if (!code || code->base.type != PF_BYTEC_SEG) {
        Parrot_ex_throw_from_c_args(interp, NULL, 1,
            "Code '%s' not found for annotations segment '%s'\n",
            code_name, self->base.name);
    }

    self->code        = code;
    code->annotations = self;

    return cursor;
}


/*

=item C<void PackFile_Annotations_dump(PARROT_INTERP, const PackFile_Segment
*seg)>

Produces a dump of the annotations segment.

=cut

*/

void
PackFile_Annotations_dump(PARROT_INTERP, ARGIN(const PackFile_Segment *seg))
{
    ASSERT_ARGS(PackFile_Annotations_dump)
    const PackFile_Annotations * const self = (const PackFile_Annotations *)seg;
    INTVAL                      i;

    default_dump_header(interp, (const PackFile_Segment *)self);

    /* Dump keys. */
    Parrot_io_printf(interp, "\n  keys => [\n");
    for (i = 0; i < self->num_keys; ++i) {
        const PackFile_Annotations_Key * const key = self->keys + i;
        Parrot_io_printf(interp, "    #%d\n    [\n", i);
        Parrot_io_printf(interp, "        NAME => %Ss\n",
                self->code->const_table->str.constants[key->name]);
        Parrot_io_printf(interp, "        TYPE => %s\n",
                key->type == PF_ANNOTATION_KEY_TYPE_INT ? "integer" :
                key->type == PF_ANNOTATION_KEY_TYPE_STR ? "string" :
                key->type == PF_ANNOTATION_KEY_TYPE_NUM ? "number" :
                "PMC");
        Parrot_io_printf(interp, "    ],\n");
    }

    Parrot_io_printf(interp, "  ],\n");

    /* Dump groups. */
    Parrot_io_printf(interp, "\n  groups => [\n");
    for (i = 0; i < self->num_groups; ++i) {
        const PackFile_Annotations_Group * const group = self->groups + i;
        Parrot_io_printf(interp, "    #%d\n    [\n", i);
        Parrot_io_printf(interp, "        BYTECODE_OFFSET => %d\n",
                group->bytecode_offset);
        Parrot_io_printf(interp, "        ENTRIES_OFFSET => %d\n",
                group->entries_offset);
        Parrot_io_printf(interp, "    ],\n");
    }

    Parrot_io_printf(interp, "  ],\n");

    /* Dump entries. */
    Parrot_io_printf(interp, "\n  entries => [\n");

    for (i = 0; i < self->num_entries; ++i) {
        const PackFile_Annotations_Entry * const entry = self->entries + i;
        Parrot_io_printf(interp, "    #%d\n    [\n", i);
        Parrot_io_printf(interp, "        BYTECODE_OFFSET => %d\n",
                entry->bytecode_offset);
        Parrot_io_printf(interp, "        KEY => %d\n",
                entry->key);
        Parrot_io_printf(interp, "        VALUE => %d\n",
                entry->value);
        Parrot_io_printf(interp, "    ],\n");
    }

    Parrot_io_printf(interp, "  ],\n");
    Parrot_io_printf(interp, "],\n");
}


/*

=item C<void PackFile_Annotations_add_group(PARROT_INTERP, PackFile_Annotations
*self, opcode_t offset)>

Starts a new bytecode annotation group. Takes the offset in the bytecode where
the new annotations group starts.

=cut

*/
PARROT_EXPORT
void
PackFile_Annotations_add_group(PARROT_INTERP, ARGMOD(PackFile_Annotations *self),
        opcode_t offset)
{
    ASSERT_ARGS(PackFile_Annotations_add_group)
    PackFile_Annotations_Group *group;

    /* Allocate extra space for the group in the groups array. */
    if (self->groups)
        self->groups = mem_gc_realloc_n_typed_zeroed(interp, self->groups,
            1 + self->num_groups, self->num_groups, PackFile_Annotations_Group);
    else
        self->groups = mem_gc_allocate_n_typed(interp,
                1 + self->num_groups, PackFile_Annotations_Group);

    /* Store details. */
    group = self->groups + self->num_groups;
    group->bytecode_offset = offset;
    group->entries_offset  = self->num_entries;

    /* Increment group count. */
    ++self->num_groups;
}


/*

=item C<void PackFile_Annotations_add_entry(PARROT_INTERP, PackFile_Annotations
*self, opcode_t offset, opcode_t key, opcode_t type, opcode_t value)>

Adds a new bytecode annotation entry. Takes the annotations segment to add the
entry to, the current bytecode offset (assumed to be the greatest one so far in
the currently active group), the annotation key (as an index into the constants
table), the annotation value type (one of PF_ANNOTATION_KEY_TYPE_INT,
PF_ANNOTATION_KEY_TYPE_STR or PF_ANNOTATION_KEY_TYPE_NUM) and the value. The
value will be an integer literal in the case of type being
PF_ANNOTATION_KEY_TYPE_INT, or an index into the constants table otherwise.

=cut

*/
PARROT_EXPORT
void
PackFile_Annotations_add_entry(PARROT_INTERP, ARGMOD(PackFile_Annotations *self),
        opcode_t offset, opcode_t key, opcode_t type, opcode_t value)
{
    ASSERT_ARGS(PackFile_Annotations_add_entry)
    /* See if we already have this key. */
    STRING  * const key_name = self->code->const_table->str.constants[key];
    opcode_t key_id   = -1;
    INTVAL   i;

    for (i = 0; i < self->num_keys; ++i) {
        STRING * const test_key = self->code->const_table->str.constants[self->keys[i].name];
        if (STRING_equal(interp, test_key, key_name)) {
            key_id = i;
            break;
        }
    }

    if (key_id == -1) {
        /* We do have it. Add key entry. */
        if (self->keys)
            self->keys = mem_gc_realloc_n_typed_zeroed(interp, self->keys,
                    1 + self->num_keys, self->num_keys, PackFile_Annotations_Key);
        else
            self->keys = mem_gc_allocate_n_typed(interp,
                    1 + self->num_keys, PackFile_Annotations_Key);

        key_id             = self->num_keys;
        ++self->num_keys;

        /* Populate it. */
        self->keys[key_id].name = key;
        self->keys[key_id].type = type;
    }
    else {
        /* Ensure key types are compatible. */
        if (self->keys[key_id].type != type)
            Parrot_ex_throw_from_c_args(interp, NULL,
                EXCEPTION_INVALID_OPERATION,
                "Annotations with different types of value used for key '%S'\n",
                key_name);
    }

    /* Add annotations entry. */
    if (self->entries)
        self->entries = mem_gc_realloc_n_typed(interp, self->entries,
                1 + self->num_entries, PackFile_Annotations_Entry);
    else
        self->entries = mem_gc_allocate_n_typed(interp,
                1 + self->num_entries, PackFile_Annotations_Entry);

    self->entries[self->num_entries].bytecode_offset = offset;
    self->entries[self->num_entries].key             = key_id;
    self->entries[self->num_entries].value           = value;

    ++self->num_entries;
}


/*

=item C<static PMC * make_annotation_value_pmc(PARROT_INTERP,
PackFile_Annotations *self, INTVAL type, opcode_t value)>

Makes a PMC of the right type holding the value.  Helper for
C<PackFile_Annotations_lookup()>.

=cut

*/

PARROT_CANNOT_RETURN_NULL
static PMC *
make_annotation_value_pmc(PARROT_INTERP, ARGIN(PackFile_Annotations *self),
        INTVAL type, opcode_t value)
{
    ASSERT_ARGS(make_annotation_value_pmc)
    PMC *result;

    switch (type) {
      case PF_ANNOTATION_KEY_TYPE_INT:
        result = Parrot_pmc_new_init_int(interp, enum_class_Integer, value);
        break;
      case PF_ANNOTATION_KEY_TYPE_NUM:
        result = Parrot_pmc_new(interp, enum_class_Float);
        VTABLE_set_number_native(interp, result,
                    self->code->const_table->num.constants[value]);
        break;
      default:
        result = Parrot_pmc_new(interp, enum_class_String);
        VTABLE_set_string_native(interp, result,
                    self->code->const_table->str.constants[value]);
    }

    return result;
}


/*

=item C<PMC * PackFile_Annotations_lookup(PARROT_INTERP, PackFile_Annotations
*self, opcode_t offset, STRING *key)>

Looks up the annotation(s) in force at the given bytecode offset. If just one
particular annotation is required, it can be passed as key, and the value will
be returned (or a NULL PMC if no annotation of that name is in force).
Otherwise, a Hash will be returned of the all annotations. If there are none in
force, an empty hash will be returned.

=cut

*/

PARROT_CANNOT_RETURN_NULL
PMC *
PackFile_Annotations_lookup(PARROT_INTERP, ARGIN(PackFile_Annotations *self),
        opcode_t offset, ARGIN_NULLOK(STRING *key))
{
    ASSERT_ARGS(PackFile_Annotations_lookup)
    PMC   *result;
    INTVAL start_entry = 0;
    INTVAL i;

    /* If we have a key, look up its ID; if we don't find one. */
    opcode_t key_id = -1;

    if (!STRING_IS_NULL(key)) {
        for (i = 0; i < self->num_keys; ++i) {
            STRING * const test_key = self->code->const_table->str.constants[self->keys[i].name];
            if (STRING_equal(interp, test_key, key)) {
                key_id = i;
                break;
            }
        }

        if (key_id == -1)
            return PMCNULL;
    }

    /* Use groups to find search start point. */
    for (i = 0; i < self->num_groups; ++i)
        if (offset < self->groups[i].bytecode_offset)
            break;
        else
            start_entry = self->groups[i].entries_offset;

    if (key_id == -1) {
        /* Look through entries, storing what we find by key and tracking those
         * that we have values for. */
        opcode_t *latest_values = mem_gc_allocate_n_zeroed_typed(interp,
                self->num_keys, opcode_t);
        opcode_t *have_values   = mem_gc_allocate_n_zeroed_typed(interp,
                self->num_keys, opcode_t);

        for (i = start_entry; i < self->num_entries; ++i) {
            if (self->entries[i].bytecode_offset >= offset)
                break;

            latest_values[self->entries[i].key] = self->entries[i].value;
            have_values[self->entries[i].key]   = 1;
        }

        /* Create hash of values we have. */
        result = Parrot_pmc_new(interp, enum_class_Hash);

        for (i = 0; i < self->num_keys; ++i) {
            if (have_values[i]) {
                STRING * const key_name =
                    self->code->const_table->str.constants[self->keys[i].name];
                VTABLE_set_pmc_keyed_str(interp, result, key_name,
                        make_annotation_value_pmc(interp, self, self->keys[i].type,
                                latest_values[i]));
            }
        }

        mem_gc_free(interp, latest_values);
        mem_gc_free(interp, have_values);
    }
    else {
        /* Look for latest applicable value of the key. */
        opcode_t latest_value = 0;
        opcode_t found_value  = 0;

        for (i = start_entry; i < self->num_entries; ++i) {
            if (self->entries[i].bytecode_offset >= offset)
                break;

            if (self->entries[i].key == key_id) {
                latest_value = self->entries[i].value;
                found_value  = 1;
            }
        }

        /* Did we find anything? */
        if (!found_value)
            result = PMCNULL;
        else
            result = make_annotation_value_pmc(interp, self,
                    self->keys[key_id].type, latest_value);
    }

    return result;
}

/*

=item C<static void compile_or_load_file(PARROT_INTERP, STRING *path,
enum_runtime_ft file_type)>

Either load a bytecode file and append it to the current packfile directory, or
compile a PIR or PASM file from source.

=cut

*/

static void
compile_or_load_file(PARROT_INTERP, ARGIN(STRING *path),
        enum_runtime_ft file_type)
{
    ASSERT_ARGS(compile_or_load_file)
    char * const filename = Parrot_str_to_cstring(interp, path);

    UINTVAL regs_used[]     = { 2, 2, 2, 2 }; /* Arbitrary values */
    const int parrot_hll_id = 0;
    PMC * context = Parrot_push_context(interp, regs_used);
    Parrot_pcc_set_HLL(interp, context, parrot_hll_id);
    Parrot_pcc_set_namespace(interp, context,
            Parrot_hll_get_HLL_namespace(interp, parrot_hll_id));

    if (file_type == PARROT_RUNTIME_FT_PBC) {
        PackFile * const pf = PackFile_append_pbc(interp, filename);
        Parrot_str_free_cstring(filename);

        if (!pf)
            Parrot_ex_throw_from_c_args(interp, NULL, 1,
                "Unable to append PBC to the current directory");

        mem_gc_free(interp, pf->header);
        pf->header = NULL;
        mem_gc_free(interp, pf->dirp);
        pf->dirp   = NULL;
        /* no need to free pf here, as directory_destroy will get it */
    }
    else {
        STRING *err;
        PackFile_ByteCode * const cs =
            (PackFile_ByteCode *)Parrot_compile_file(interp,
                filename, &err);
        Parrot_str_free_cstring(filename);

        if (cs)
            do_sub_pragmas(interp, cs, PBC_LOADED, NULL);
        else
            Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
                "compiler returned NULL ByteCode '%Ss' - %Ss", path, err);
    }

    Parrot_pop_context(interp);
}

/*

=item C<void Parrot_load_language(PARROT_INTERP, STRING *lang_name)>

Load the compiler libraries for a given high-level language into the
interpreter.

=cut

*/

PARROT_EXPORT
void
Parrot_load_language(PARROT_INTERP, ARGIN_NULLOK(STRING *lang_name))
{
    ASSERT_ARGS(Parrot_load_language)
    STRING *wo_ext, *file_str, *path, *pbc;
    STRING *found_path, *found_ext;
    INTVAL name_length;
    enum_runtime_ft file_type;
    PMC *is_loaded_hash;

    if (STRING_IS_NULL(lang_name))
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
            "\"load_language\" no language name");

    /* Full path to language library is "abc/abc.pbc". */
    pbc = CONST_STRING(interp, "pbc");
    wo_ext   = Parrot_str_concat(interp, lang_name, CONST_STRING(interp, "/"));
    wo_ext   = Parrot_str_concat(interp, wo_ext, lang_name);
    file_str = Parrot_str_concat(interp, wo_ext, CONST_STRING(interp, "."));
    file_str = Parrot_str_concat(interp, file_str, pbc);

    /* Check if the language is already loaded */
    is_loaded_hash = VTABLE_get_pmc_keyed_int(interp,
        interp->iglobals, IGLOBALS_PBC_LIBS);
    if (VTABLE_exists_keyed_str(interp, is_loaded_hash, wo_ext))
        return;

    file_type = PARROT_RUNTIME_FT_LANG;

    path = Parrot_locate_runtime_file_str(interp, file_str, file_type);
    if (!path)
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
            "\"load_language\" couldn't find a compiler module for the language '%Ss'", lang_name);

    /* remember wo_ext => full_path mapping */
    VTABLE_set_string_keyed_str(interp, is_loaded_hash,
            wo_ext, path);

    /* Add the include and dynext paths to the global search */

    /* Get the base path of the located module */
    parrot_split_path_ext(interp, path, &found_path, &found_ext);
    name_length = Parrot_str_length(interp, lang_name);
    found_path = STRING_substr(interp, found_path, 0,
            Parrot_str_length(interp, found_path)-name_length);

    Parrot_lib_add_path(interp, Parrot_str_concat(interp, found_path, CONST_STRING(interp, "include/")),
            PARROT_LIB_PATH_INCLUDE);
    Parrot_lib_add_path(interp, Parrot_str_concat(interp, found_path, CONST_STRING(interp, "dynext/")),
            PARROT_LIB_PATH_DYNEXT);
    Parrot_lib_add_path(interp, Parrot_str_concat(interp, found_path, CONST_STRING(interp, "library/")),
            PARROT_LIB_PATH_LIBRARY);


    /* Check if the file found was actually a bytecode file (.pbc extension) or
     * a source file (.pir or .pasm extension. */

    if (STRING_equal(interp, found_ext, pbc))
        file_type = PARROT_RUNTIME_FT_PBC;
    else
        file_type = PARROT_RUNTIME_FT_SOURCE;

    compile_or_load_file(interp, path, file_type);
}

/*

=item C<static PackFile * PackFile_append_pbc(PARROT_INTERP, const char
*filename)>

Reads and appends a PBC it to the current directory.  Fixes up sub addresses in
newly loaded bytecode and runs C<:load> subs.

=cut

*/

PARROT_WARN_UNUSED_RESULT
PARROT_CAN_RETURN_NULL
static PackFile *
PackFile_append_pbc(PARROT_INTERP, ARGIN_NULLOK(const char *filename))
{
    ASSERT_ARGS(PackFile_append_pbc)
    PackFile * const pf = Parrot_pbc_read(interp, filename, 0);

    if (pf) {
        /* An embedder can try to load_bytecode without having an initial_pf */
        if (!interp->initial_pf) {
            interp->initial_pf = PackFile_new_dummy(interp, CONST_STRING(interp, "dummy"));
            /* PackFile_new_dummy must never fail */
            PARROT_ASSERT(interp->initial_pf);
        }
        PackFile_add_segment(interp, &interp->initial_pf->directory,
                &pf->directory.base);

        do_sub_pragmas(interp, pf->cur_cs, PBC_LOADED, NULL);
    }

    return pf;
}


/*

=item C<void Parrot_load_bytecode(PARROT_INTERP, Parrot_String file_str)>

Load a bytecode, PIR, or PASM file into the interpreter.

=cut

*/

/* intermediate hook during changes */
PARROT_EXPORT
void
Parrot_load_bytecode(PARROT_INTERP, ARGIN_NULLOK(Parrot_String file_str))
{
    ASSERT_ARGS(Parrot_load_bytecode)
    STRING         *wo_ext, *ext, *pbc, *path;
    STRING         *found_path, *found_ext;
    PMC            *is_loaded_hash;
    enum_runtime_ft file_type;

    if (STRING_IS_NULL(file_str))
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
            "\"load_bytecode\" no file name");

    parrot_split_path_ext(interp, file_str, &wo_ext, &ext);

    /* check if wo_ext is loaded */
    is_loaded_hash = VTABLE_get_pmc_keyed_int(interp,
        interp->iglobals, IGLOBALS_PBC_LIBS);

    if (VTABLE_exists_keyed_str(interp, is_loaded_hash, wo_ext))
        return;

    pbc = CONST_STRING(interp, "pbc");

    if (STRING_equal(interp, ext, pbc))
        file_type = PARROT_RUNTIME_FT_PBC;
    else
        file_type = PARROT_RUNTIME_FT_SOURCE;

    path = Parrot_locate_runtime_file_str(interp, file_str, file_type);
    if (!path)
        Parrot_ex_throw_from_c_args(interp, NULL, EXCEPTION_LIBRARY_ERROR,
            "\"load_bytecode\" couldn't find file '%Ss'", file_str);

    /* remember wo_ext => full_path mapping */
    VTABLE_set_string_keyed_str(interp, is_loaded_hash, wo_ext, path);

    parrot_split_path_ext(interp, path, &found_path, &found_ext);

    /* Check if the file found was actually a bytecode file (.pbc
     * extension) or a source file (.pir or .pasm extension). */

    if (STRING_equal(interp, found_ext, pbc))
        file_type = PARROT_RUNTIME_FT_PBC;
    else
        file_type = PARROT_RUNTIME_FT_SOURCE;

    compile_or_load_file(interp, path, file_type);
}


/*

=item C<void PackFile_fixup_subs(PARROT_INTERP, pbc_action_enum_t what, PMC
*eval)>

Calls C<:load>, C<:init>, C<:main>, C<:immediate> and/or C<:postcomp>
subroutines in the current packfile, depending on the value of C<action>.
See C<do_sub_pragmas> for more details.

=cut

*/

PARROT_EXPORT
void
PackFile_fixup_subs(PARROT_INTERP, pbc_action_enum_t what, ARGIN_NULLOK(PMC *eval))
{
    ASSERT_ARGS(PackFile_fixup_subs)
    PARROT_CALLIN_START(interp);
    do_sub_pragmas(interp, interp->code, what, eval);
    PARROT_CALLIN_END(interp);
}


/*

=back

=head1 HISTORY

Parrot_readbc and Parrot_loadbc renamed. Trace macros, long double and
64-bit conversion work by Reini Urban 2009.

Rework by Melvin; new bytecode format, make bytecode portable. (Do
endian conversion and wordsize transforms on the fly.)

leo applied and modified Juergen Boemmels packfile patch giving an
extensible packfile format with directory reworked again, with common
chunks (C<default_*>).

2003.11.21 leo: moved low level item fetch routines to new
F<pf/pf_items.c>

=cut

*/


/*
 * Local variables:
 *   c-file-style: "parrot"
 * End:
 * vim: expandtab shiftwidth=4 cinoptions='\:2=2' :
 */

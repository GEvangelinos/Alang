#ifndef ABC_HEADER_HPP
#define ABC_HEADER_HPP
#include "abc_spec.hpp"
#include "core/numeric_types.hpp"

namespace alpha::abc
{
/* ============================================================================
 * AVM (Abstract Virtual Machine) Binary File Format
 * Specification Version: 1.0
 * Byte Order: Little Endian (LE)
 * ============================================================================
 *
 * FILE LAYOUT:
 * +-----------------------+ 0x00
 * | ABC_Header            | (64 Bytes)
 * +-----------------------+ [off_strings]
 * | String Pool           | (Length-prefixed strings, null-terminated)
 * +-----------------------+ [off_userfuncs]
 * | User Function Table   | (Address, LocalSize, ID-Index)
 * +-----------------------+ [off_libfuncs]
 * | Lib Function Table    | (Indices into String Pool)
 * +-----------------------+ [off_code]
 * | Code Segment          | (Instructions: Opcode + 3 Operands)
 * +-----------------------+
 *
 * ----------------------------------------------------------------------------
 * HEADER DEFINITION (64 Bytes, Zero Implicit Padding)
 * ----------------------------------------------------------------------------
 * [Offset] | [Size] | [Name]          | [Description]
 * ---------|--------|-----------------|---------------------------------------
 * 0       | 4      | magic           | 0x14470105 (340200501)
 * 4       | 4      | header_size     | Total bytes in header (current: 64)
 * 8       | 2      | v_major         | Breaking change version
 * 10      | 2      | v_minor         | Compatible change version
 * 12      | 4      | alignment_pad   | Explicit 0-fill for 8-byte alignment
 * 16      | 8      | timestamp       | Unix Epoch (seconds since 1970)
 * 24      | 8      | file_size       | Full file size in bytes
 * 32      | 8      | Sections        | Section table.
 * ----------------------------------------------------------------------------
 * * DATA SEGMENTS:
 * * 1. String Pool:
 * [u32 total_count]
 * [ {u32 len, char data[len]} ... ] // Null-terminated internally
 * * 2. User Function Table:
 * [u32 total_count]
 * [ {u32 address, u32 localsize, u32 string_index} ... ]
 * * 3. Lib Function Table:
 * [u32 total_count]
 * [ {u32 string_index} ... ]
 * * 4. Code Segment:
 * [u32 total_count]
 * [ {u8 opcode, Operand arg1, Operand arg2, Operand arg3} ... ]
 * Where Operand = {u8 type, u32 value}
 * ============================================================================
 */

struct BufferSpan
{
    u32 offset; // Where the region starts inside the binary.
    u32 size;

    [[nodiscard]] auto begin() const noexcept { return offset; }      // Inclusive
    [[nodiscard]] auto end() const noexcept { return offset + size; } // Exclusive
};

struct Header
{
    struct Sections
    {

        struct Catalog
        {
            BufferSpan lut; // Points to the [Span0, Span1, Span2...] array
        };

        Catalog strings;
        Catalog progfuncs;
        BufferSpan instructions;
    };

    abc::spec::MagicT magic;
    u32 header_size; /* The "Gatekeeper" for extensibility */

    u16 v_major;
    u16 v_minor;
    char padding[4];

    u64 timestamp;
    u64 file_size;

    Sections sections;
};

// If the compiler adds hidden padding, this sum will NOT equal sizeof(struct ABC_Header)
static_assert(
    sizeof(Header) ==
    sizeof(uint32_t) * 2 + // magic, header_size
    sizeof(uint16_t) * 2 + // v_major, v_minor
    sizeof(uint32_t) +     // alignment_pad
    sizeof(uint64_t) * 2 + // timestamp, file_size
    sizeof(Header::Sections),
    "Error: Compiler added implicit padding to ABC_Header. Fix alignment manually."
);
} // namespace alpha::abc
#endif //ABC_HEADER_HPP

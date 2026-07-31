#include "postcodes.h"

#define POST_CODE_WORD_COUNT 4

static uint16_t codeWords[POST_CODE_WORD_COUNT] = {0};
static uint8_t currFlavor = 0;
static uint8_t currIndex = 0; // 0 == no pending accumulation
// Set when a blank (index 0) Segments-write of the same flavor as the
// pending accumulation is observed; consumed by the next real segment to
// prove it's a genuine continuation rather than an unrelated/abandoned one.
static uint8_t sawContinuationMarker = 0;

static void resetCodeWords(void) {
    for (int i = 0; i < POST_CODE_WORD_COUNT; i++) {
        codeWords[i] = 0;
    }
    currIndex = 0;
    sawContinuationMarker = 0;
}

static uint64_t assembleCode(void) {
    uint64_t code = 0;
    for (int i = 0; i < POST_CODE_WORD_COUNT; i++) {
        code |= (uint64_t)codeWords[i] << (16 * i);
    }
    return code;
}

const char *POST_GetSegmentName(uint8_t flavor) {
    switch (flavor & SEGMENT_FLAVOR_MASK) {
        case CODE_FLAVOR_CPU:
            return "CPU";
        case CODE_FLAVOR_SP:
            return "SP ";
        case CODE_FLAVOR_SMC:
            return "SMC";
        case CODE_FLAVOR_OS:
            return "OS ";
        default:
            return "???";
    }
}

uint8_t POST_ProcessSegment(PostCode *out) {
    uint8_t segByte = Slave_RegRead(REG_Segments);
    uint8_t flavor = segByte & SEGMENT_FLAVOR_MASK;
    uint8_t rawIndex = segByte & SEGMENT_INDEX_MASK;

    if (rawIndex == 0) {
        // Blank write: no digit data, but the console sends it between
        // every part of a genuine multi-part code tagged with that code's flavor.
        if (currIndex != 0 && flavor == currFlavor) {
            sawContinuationMarker = 1;
        }
        return 0;
    }

    uint8_t code_idx;
    switch (rawIndex) {
        case 1: code_idx = 0; break;
        case 2: code_idx = 1; break;
        case 4: code_idx = 2; break;
        case 8: code_idx = 3; break;
        default: return 0;
    }

    uint8_t isContinuation = currIndex != 0 && sawContinuationMarker && flavor == currFlavor;
    if (currIndex != 0 && !isContinuation) {
        // Pending accumulation was never continued (no matching blank
        // marker seen); discard it instead of merging stale digits into
        // this segment.
        resetCodeWords();
    }

    codeWords[code_idx] = (Slave_RegRead(REG_Digit0) & 0x0F)
        | ((Slave_RegRead(REG_Digit1) & 0x0F) << 4)
        | ((Slave_RegRead(REG_Digit2) & 0x0F) << 8)
        | ((Slave_RegRead(REG_Digit3) & 0x0F) << 12);
    currFlavor = flavor;
    currIndex = rawIndex;
    sawContinuationMarker = 0;

    // Codes come in MSB-first (index order: 8, 4, 2, 1)
    // When index is 1, full code was transmitted from console
    if (rawIndex != 1) {
        return 0;
    }

    out->code = assembleCode();
    out->flavor = currFlavor;
    resetCodeWords();
    return 1;
}

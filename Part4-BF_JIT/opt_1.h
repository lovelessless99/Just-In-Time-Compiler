#ifndef __OPT_1_H
#define __OPT_1_H

char epilogue [] = {
    0x48, 0x81, 0xC4, 0x38, 0x75, 0x00, 0x00, // addq $30008, %rsp
    // restore callee saved registers
    0x41, 0x5E, // popq %r14
    0x41, 0x5D, // popq %r13
    0x41, 0x5C, // popq %r12
    0x5d, // pop rbp
    0xC3 // ret
  };
  
int opcode_table[][20] = {
        ['>'] = { 0x49, 0xFF, 0xC4 }, // inc %r12
        ['<'] = { 0x49, 0xFF, 0xCC }, // dec %r12
        ['+'] = { 0x41, 0xFE, 0x04, 0x24 }, // incb (%r12)
        ['-'] = { 0x41, 0xFE, 0x0C, 0x24 }, // decv (%r12)
        ['.'] = { 
                        0x41, 0x0F, 0xB6, 0x3C, 0x24, // movzbl (%r12), %edi
                        0x41, 0xFF, 0xD5              // callq *%r13
                },

        [','] = {
                        0x41, 0xFF, 0xD6, // callq *%r14
                        0x41, 0x88, 0x04, 0x24 // movb %al, (%r12)
                },

        ['['] = {
                        0x41, 0x80, 0x3C, 0x24, 0x00, // cmpb $0, (%r12)
                        // Needs to be patched up
                        0x0F, 0x84, 0x00, 0x00, 0x00, 0x00 // je <32b relative offset, 2's compliment, LE>
                },

        [']'] = { 
                        0x41, 0x80, 0x3C, 0x24, 0x00, // cmpb $0, (%r12)
                        // Needs to be patched up
                        0x0F, 0x85, 0x00, 0x00, 0x00, 0x00 // jne <33b relative offset, 2's compliment, LE>
                }
};













#endif



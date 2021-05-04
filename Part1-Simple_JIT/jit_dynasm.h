/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.3.0, DynASM x64 version 1.3.0
** DO NOT EDIT! The original file is in "jit_dynasm.dasc".
*/

#if DASM_VERSION != 10300
#error "Version mismatch between DynASM and included encoding engine"
#endif

# 1 "jit_dynasm.dasc"
// DynASM directives.
//|.arch x64
//|.actionlist actions
static const unsigned char actions[4] = {
  184,237,195,255
};

# 4 "jit_dynasm.dasc"

// This define affects "|" DynASM lines.  "Dst" must
// resolve to a dasm_State** that points to a dasm_State*.
#define Dst &state

int main(int argc, char *argv[]) {
        if (argc < 2) {
                fprintf(stderr, "Usage: jit1 <integer>\n");
                return 1;
        }

        int num = atoi(argv[1]);
        dasm_State *state;
        initjit(&state, actions);

        // Generate the code.  Each line appends to a buffer in
        // "state", but the code in this buffer is not fully linked
        // yet because labels can be referenced before they are
        // defined.
        //
        // The run-time value of C variable "num" is substituted
        // into the immediate value of the instruction.
        //|  mov eax, num
        //|  ret
        dasm_put(Dst, 0, num);
# 28 "jit_dynasm.dasc"

        // Link the code and write it to executable memory.
        int (*fptr)() = jitcode(&state);

        // Call the JIT-ted function.
        int ret = fptr();
        assert(num == ret);

        // Free the machine code.
        free_jitcode(fptr);

        return ret;
}

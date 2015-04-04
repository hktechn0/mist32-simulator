import sys

from opcodes import mist32_opcodes

class OpsGen(object):
    template_header = """
static inline void insn_dispatch(const Instruction insn)
{
  switch(insn.base.opcode) {
"""

    template_case = """
  case {0:d}:
    i_{1}(insn);
    break;
"""

    template_footer = """
  default:
    i_invalid(insn);
    break;
  }
}
"""

    # ops => dict { opcode: "op_name", ... }
    def gen(self, ops, outfile = sys.stdout):
        g = (self.template_case.format(op, name) for op, name in ops.iteritems())
        outfile.write(self.template_header)
        outfile.writelines(g)
        outfile.write(self.template_footer)

if __name__ == "__main__":
    if len(sys.argv) > 1:
        filename = sys.argv[1]

        with open(filename, "w") as f:
            g = OpsGen()
            g.gen(mist32_opcodes, f)
    else:
        print("no output file specified.")


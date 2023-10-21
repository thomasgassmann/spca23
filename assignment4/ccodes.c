#include "ccodes.h"

struct ccodes getccodes(reg_flags_t eflags)
{
  struct ccodes ccodes;
  /********************************************************************
   * STUDENT TODO: EDIT CODE BELOW
   *
   * Your task is to extract the condition codes from the EFLAGS
   * register (stored in parameter eflags) and fill the ccodes struct
   * appropriatly.
   *
   * You can find out about the bits that correspond to the condition
   * flags from the Intel Architecture Software Developer's Manual
   * (Volume 1, Section 3.4.3); the link to the document is indicated
   * on the course webpage.
   *********************************************************************/
  ccodes.cf = eflags & 1;
  ccodes.zf = !!(eflags & 0x0040);
  ccodes.sf = !!(eflags & 0x0080);
  ccodes.of = !!(eflags & 0x0800);
  /********************************************************************
   * END EDITING
   ********************************************************************/

  return ccodes;
}
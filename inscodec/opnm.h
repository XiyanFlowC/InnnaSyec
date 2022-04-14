enum opnm{
     ADD,     ADDI,    ADDIU,     ADDU,      AND,     ANDI,      BEQ,     BEQL,
    BGEZ,   BGEZAL,  BGEZALL,    BGEZL,     BGTZ,    BGTZL,     BLEZ,    BLEZL,
    BLTZ,   BLTZAL,  BLTZALL,    BLTZL,      BNE,     BNEL,    BREAK,     DADD,
   DADDI,   DADDIU,    DADDU,      DIV,     DIVU,     DSLL,   DSLL32,    DSLLV,
    DSRA,   DSRA32,    DSRAV,     DSRL,   DSRL32,    DSRLV,     DSUB,    DSUBU,
       J,      JAL,     JALR,       JR,       LB,      LBU,       LD,      LDL,
     LDR,       LH,      LHU,      LUI,       LW,      LWL,      LWR,      LWU,
    MFHI,     MFLO,     MOVN,     MOVZ,     MTHI,     MTLO,     MULT,    MULTU,
     NOR,       OR,      ORI,     PREF,       SB,       SD,      SDL,      SDR,
      SH,      SLL,     SLLV,      SLT,     SLTI,    SLTIU,     SLTU,      SRA,
    SRAV,      SRL,     SRLV,      SUB,     SUBU,       SW,      SWL,      SWR,
    SYNC,  SYSCALL,      TEQ,     TEQI,      TGE,     TGEI,    TGEIU,     TGEU,
     TLT,     TLTI,    TLTIU,     TLTU,      TNE,     TNEI,      XOR,     XORI,
    DIV1,    DIVU1,       LQ,     MADD,    MADD1,    MADDU,   MADDU1,    MFHI1,
   MFLO1,     MFSA,    MTHI1,    MTLO1,     MTSA,    MTSAB,    MTSAH,    MULT1,
  MULTU1,    PABSH,    PABSW,    PADDB,    PADDH,   PADDSB,   PADDSH,   PADDSW,
  PADDUB,   PADDUH,   PADDUW,    PADDW,   PADSBH,     PAND,    PCEQB,    PCEQH,
   PCEQW,    PCGTB,    PCGTH,    PCGTW,    PCPYH,   PCPYLD,   PCPYUD,   PDIVBW,
  PDIVUW,    PDIVW,    PEXCH,    PEXCW,    PEXEH,    PEXEW,    PEXT5,   PEXTLB,
  PEXTLH,   PEXTLW,   PEXTUB,   PEXTUH,   PEXTUW,   PHMADH,   PHMSBH,   PINTEH,
   PINTH,    PLZCW,   PMADDH,  PMADDUW,   PMADDW,    PMAXH,    PMAXW,    PMFHI,
PMFHL_LW, PMFHL_UW, PMFHL_SLW, PMFHL_LH, PMFHL_SH,    PMFLO,    PMINH,    PMINW,
  PMSUBH,   PMSUBW,    PMTHI, PMTHL_LW,    PMTLO,   PMULTH,  PMULTUW,   PMULTW,
    PNOR,      POR,    PPAC5,    PPACB,    PPACH,    PPACW,    PREVH,   PROT3W,
   PSLLH,   PSLLVW,    PSLLW,    PSRAH,   PSRAVW,    PSRAW,    PSRLH,   PSRLVW,
   PSRLW,    PSUBB,    PSUBH,   PSUBSB,   PSUBSH,   PSUBSW,   PSUBUB,   PSUBUH,
  PSUBUW,    PSUBW,     PXOR,    QFSRV,       SQ,     BC0F,    BC0FL,     BC0T,
   BC0TL,    CACHE,       DI,       EI,     ERET,    MFBPC,     MFC0,    MFDAB,
  MFDABM,    MFDVB,   MFDVBM,    MFIAB,   MFIABM,     MFPC,     MFPS,    MTBPC,
    MTC0,    MTDAB,   MTDABM,    MTDVB,   MTDVBM,    MTIAB,   MTIABM,     MTPC,
    MTPS,     TLBP,     TLBR,    TLBWI,    TLBWR,    ABS_S,    ABS_D,    ADD_S,
   ADD_D,     BC1F,     BC1T,    C_F_S,    C_F_D,   C_UN_S,   C_UN_D,   C_EQ_S,
  C_EQ_D,  C_UEQ_S,  C_UEQ_D,  C_OLT_S,  C_OLT_D,  C_ULT_S,  C_ULT_D,  C_OLE_S,
 C_OLE_D,  C_ULE_S,  C_ULE_D,   C_SF_S,   C_SF_D, C_NGLE_S, C_NGLE_D,  C_SEQ_S,
 C_SEQ_D,  C_NGL_S,  C_NGL_D,   C_LT_S,   C_LT_D,  C_NGE_S,  C_NGE_D,   C_LE_S,
  C_LE_D,  C_NGT_S,  C_NGT_D, CEIL_L_S, CEIL_L_D, CEIL_W_S, CEIL_W_D,     CFC1,
    CTC1,  CVT_D_S,  CVT_D_W,  CVT_D_L,  CVT_L_D,  CVT_L_S,  CVT_S_D,  CVT_S_W,
 CVT_S_L,  CVT_W_S,  CVT_W_D,    DIV_S,    DIV_D,    DMFC1,    DMTC1, FLOOR_L_S,
FLOOR_L_D, FLOOR_W_S, FLOOR_W_D,     LDC1,     LWC1,     MFC1,    MOV_S,    MOV_D,
    MTC1,    MUL_S,    MUL_D,    NEG_S,    NEG_D, ROUND_L_S, ROUND_L_D, ROUND_W_S,
ROUND_W_D,     SDC1,   SQRT_S,   SQRT_D,    SUB_S,    SUB_D,     SWC1, TRUNC_L_S,
TRUNC_L_D, TRUNC_W_S, TRUNC_W_D, NOP,

RESERVED = -256, SPECIAL, REGIMM, MMI, COP0, COP1, MMI0, MMI1, MMI2, MMI3,
PMFHL, MF0, MT0, BC0, C0, MF0DBG, MF0PREF, MT0DBG, MT0PREF, BC1, S, D, W, L,
};
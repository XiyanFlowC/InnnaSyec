case ADD:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case ADDI:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case ADDIU:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case ADDU:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case AND:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rd = instruction.rd;
break;
case ANDI:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case BEQ:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BEQL:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BGEZ:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BGEZAL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BGEZALL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BGEZL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BGTZ:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BGTZL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BLEZ:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BLEZL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BLTZ:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BLTZAL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BLTZALL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BLTZL:
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BNE:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BNEL:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BREAK:
ans.dtype.data = instruction.imm;
break;
case DADD:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case DADDI:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case DADDIU:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case DADDU:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case DIV:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case DIVU:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case DSLL:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case DSLL32:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case DSLLV:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case DSRA:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case DSRA32:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case DSRAV:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case DSRL:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case DSRL32:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case DSRLV:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case DSUB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case DSUBU:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case J:
ans.jtype.addr = instruction.imm >> 2;
break;
case JAL:
ans.jtype.addr = instruction.imm >> 2;
break;
case JALR:
ans.rtype.rs = instruction.rs;
ans.rtype.rd = instruction.rd;
break;
case JR:
ans.rtype.rs = instruction.rs;
break;
case LB:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LBU:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LD:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LDL:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LDR:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LH:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LHU:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LUI:
ans.rtype.rt = instruction.rt;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LW:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LWL:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LWR:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LWU:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case MFHI:
ans.rtype.rd = instruction.rd;
break;
case MFLO:
ans.rtype.rd = instruction.rd;
break;
case MOVN:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MOVZ:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MTHI:
ans.rtype.rs = instruction.rs;
break;
case MTLO:
ans.rtype.rs = instruction.rs;
break;
case MULT:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MULTU:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case NOR:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case OR:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case ORI:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case PREF:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SB:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SD:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SDL:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SDR:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SH:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SLL:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case SLLV:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case SLT:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case SLTI:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SLTIU:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SLTU:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case SRA:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case SRAV:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case SRL:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case SRLV:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case SUB:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case SUBU:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
break;
case SW:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SWL:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SWR:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SYNC:
ans.rtype.sa = instruction.sa;
break;
case SYSCALL:
ans.dtype.data = instruction.imm;
break;
case TEQ:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case TEQI:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case TGE:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case TGEI:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case TGEIU:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case TGEU:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case TLT:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case TLTI:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case TLTIU:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case TLTU:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case TNE:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case TNEI:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case XOR:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case XORI:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case DIV1:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case DIVU1:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case LQ:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case MADD:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MADD1:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MADDU:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MADDU1:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MFHI1:
ans.rtype.rd = instruction.rd;
break;
case MFLO1:
ans.rtype.rd = instruction.rd;
break;
case MFSA:
ans.rtype.rd = instruction.rd;
break;
case MTHI1:
ans.rtype.rs = instruction.rs;
break;
case MTLO1:
ans.rtype.rs = instruction.rs;
break;
case MTSA:
ans.rtype.rs = instruction.rs;
break;
case MTSAB:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case MTSAH:
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case MULT1:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case MULTU1:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PABSH:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PABSW:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PADDB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDSB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDSH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDSW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDUB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDUH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDUW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADDW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PADSBH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PAND:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCEQB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCEQH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCEQW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCGTB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCGTH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCGTW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCPYH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCPYLD:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PCPYUD:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PDIVBW:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PDIVUW:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PDIVW:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PEXCH:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PEXCW:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PEXEH:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PEXEW:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PEXT5:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PEXTLB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PEXTLH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PEXTLW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PEXTUB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PEXTUH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PEXTUW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PHMADH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PHMSBH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PINTEH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PINTH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PLZCW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
break;
case PMADDH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMADDUW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMADDW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMAXH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMAXW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMFHI:
ans.rtype.rd = instruction.rd;
break;
case PMFHL_LW:
ans.rtype.rd = instruction.rd;
break;
case PMFHL_UW:
ans.rtype.rd = instruction.rd;
break;
case PMFHL_SLW:
ans.rtype.rd = instruction.rd;
break;
case PMFHL_LH:
ans.rtype.rd = instruction.rd;
break;
case PMFHL_SH:
ans.rtype.rd = instruction.rd;
break;
case PMFLO:
ans.rtype.rd = instruction.rd;
break;
case PMINH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMINW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMSUBH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMSUBW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMTHI:
ans.rtype.rs = instruction.rs;
break;
case PMTHL_LW:
ans.rtype.rs = instruction.rs;
break;
case PMTLO:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMULTH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMULTUW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PMULTW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PNOR:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case POR:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PPAC5:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PPACB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PPACH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PPACW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PREVH:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PROT3W:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case PSLLH:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case PSLLVW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSLLW:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case PSRAH:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case PSRAVW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSRAW:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case PSRLH:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case PSRLVW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSRLW:
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
ans.rtype.sa = instruction.sa;
break;
case PSUBB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBSB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBSH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBSW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBUB:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBUH:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBUW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PSUBW:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case PXOR:
ans.rtype.rd = instruction.rd;
ans.rtype.rs = instruction.rs;
ans.rtype.rt = instruction.rt;
break;
case QFSRV:
ans.rtype.rd = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case SQ:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case BC0F:
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BC0FL:
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BC0T:
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BC0TL:
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case CACHE:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case DI:
break;
case EI:
break;
case ERET:
break;
case MFBPC:
ans.rtype.rt = instruction.rt;
break;
case MFC0:
ans.rtype.rt = instruction.rt;
break;
case MFDAB:
ans.rtype.rt = instruction.rt;
break;
case MFDABM:
ans.rtype.rt = instruction.rt;
break;
case MFDVB:
ans.rtype.rt = instruction.rt;
break;
case MFDVBM:
ans.rtype.rt = instruction.rt;
break;
case MFIAB:
ans.rtype.rt = instruction.rt;
break;
case MFIABM:
ans.rtype.rt = instruction.rt;
break;
case MFPC:
ans.rtype.rt = instruction.rt;
break;
case MFPS:
ans.rtype.rt = instruction.rt;
break;
case MTBPC:
ans.rtype.rt = instruction.rt;
break;
case MTC0:
ans.rtype.rt = instruction.rt;
break;
case MTDAB:
ans.rtype.rt = instruction.rt;
break;
case MTDABM:
ans.rtype.rt = instruction.rt;
break;
case MTDVB:
ans.rtype.rt = instruction.rt;
break;
case MTDVBM:
ans.rtype.rt = instruction.rt;
break;
case MTIAB:
ans.rtype.rt = instruction.rt;
break;
case MTIABM:
ans.rtype.rt = instruction.rt;
break;
case MTPC:
ans.rtype.rt = instruction.rt;
break;
case MTPS:
ans.rtype.rt = instruction.rt;
break;
case TLBP:
break;
case TLBR:
break;
case TLBWI:
break;
case TLBWR:
break;
case ABS_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case ABS_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case ADD_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case ADD_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case BC1F:
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case BC1T:
ans.itype.imm = (instruction.imm >> 2) & 0xFFFF;break;
case C_F_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_F_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_UN_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_UN_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_EQ_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_EQ_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_UEQ_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_UEQ_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_OLT_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_OLT_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_ULT_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_ULT_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_OLE_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_OLE_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_ULE_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_ULE_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_SF_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_SF_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGLE_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGLE_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_SEQ_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_SEQ_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGL_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGL_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_LT_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_LT_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGE_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGE_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_LE_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_LE_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGT_S:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case C_NGT_D:
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case CEIL_L_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CEIL_L_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CEIL_W_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CEIL_W_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CFC1:
ans.rtype.rt = instruction.rt;
break;
case CTC1:
ans.rtype.rt = instruction.rt;
break;
case CVT_D_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_D_W:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_D_L:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_L_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_L_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_S_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_S_W:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_S_L:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_W_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case CVT_W_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case DIV_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case DIV_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case DMFC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rd = instruction.rd;
break;
case DMTC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rd = instruction.rd;
break;
case FLOOR_L_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case FLOOR_L_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case FLOOR_W_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case FLOOR_W_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case LDC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case LWC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case MFC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rd = instruction.rd;
break;
case MOV_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case MOV_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case MTC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rd = instruction.rd;
break;
case MUL_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case MUL_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case NEG_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case NEG_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case ROUND_L_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case ROUND_L_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case ROUND_W_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case ROUND_W_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case SDC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case SQRT_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case SQRT_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case SUB_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case SUB_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
ans.rtype.rt = instruction.rt;
break;
case SWC1:
ans.rtype.rt = instruction.rt;
ans.rtype.rs = instruction.rs;
ans.itype.imm = instruction.imm & 0xFFFF;break;
case TRUNC_L_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case TRUNC_L_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case TRUNC_W_S:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case TRUNC_W_D:
ans.rtype.rd = instruction.sa;
ans.rtype.rs = instruction.rd;
break;
case NOP:
return 0;
break;

#include "codec.h"
#include "xyutils.h"

// get next word, return the terminating char*
// the buffer will recieve the word this function read.
// 
const char *get_next_word(const char *str, char *buffer) {
    while((*str == ' ' || *str == '\t') && *str != '\0') {
        ++str;
    }

    while((*str >= 'a' && *str <= 'z') ||
        (*str >= 'A' && *str <= 'B') ||
        (*str >= '0' && *str <= '9') ||
        (*str == '.'))
    {
        *buffer++ = *str++;
    }

    *buffer = '\0';
    return str;
}

// target num:
// 
int _get_target_(const char *str) {
    
    if(str[0] == 'i') {
        if(str[1] == 'm') return 1;
    }
    if(str[0] == 's') {
        if(str[1] == 'a') return 2;
    }
    if(str[0] == 'r') {
        if(str[1] == 't') return 3;
        if(str[1] == 'd') return 4;
        if(str[1] == 's') return 5;
    }
    if(str[0] == 'f') {
        if(str[1] == 't') return 3;
        if(str[1] == 's') return 4;
        if(str[1] == 'd') return 2;
    }
}

int disas(struct instr_t instr, unsigned int offset, char *buffer) {
    char mnemonic[128];
    char para_buf[512];
    struct instr_def def = GetInstructionDefinitionByIndex(instr.opcode);
    strcpy(mnemonic, def.name);
    strlwr(mnemonic);
    
    char *orip = def.para;
    char *para = para_buf;

    if(*orip != '_')
    while (*orip != '\0') {
        int type;
        switch (*orip) {
            case '!':
            type = 1;
            break;
            case '$':
            type = 2;
            break;
            case '^':
            type = 3;
            break;
            case '@':
            type = 4;
            break;
            case '#':
            type = 5;
            break;
            case '*':
            case '%':
            case '&':
            type = 6;
            break;
            default:
            *para++ = *orip++;
            goto wh_end;
        }

        unsigned long long value;
        int target = _get_target_(++orip);
        switch(target) {
            case 1:
            value = instr.imm;
            break;
            case 2:
            value = instr.sa;
            break;
            case 3:
            value = instr.rt;
            break;
            case 4:
            value = instr.rd;
            break;
            case 5:
            value = instr.rs;
        }
        if(def.type == IT_imma && target == 1) {
            value += offset + 4;
        }

        switch(type) {
            case 5:
            para += output_int(para, value, 0);
            break;
            case 1:
            case 6:
            para += output_int(para, value, 1);
            break;
            case 2:
            strcpy(para, gpr_names[value]);
            para += strlen(gpr_names[value]);
            break;
            case 3:
            strcpy(para, cop0r_names[value]);
            para += strlen(cop0r_names[value]);
            break;
            case 4:
            strcpy(para, cop1r_names[value]);
            para += strlen(cop1r_names[value]);
            break;
        }

        orip += 2;

        wh_end:;
    }

    *para = '\0';
    sprintf(buffer, "%s %s",mnemonic, para_buf);
    return 4;
}

/**
 * @brief Parse the def and fill the instruction.
 * 
 * @param para The parameters string.
 * @param def The definition of the parameter.
 * @param result The assembled instruction.
 * @return int 0 if success, -18; -11 if para is not given but expected, -16 if para is
 * given but not expected, -12; -13; -14 if integer syntax is incorrect, -15 if an
 * unexpected signed number is found, -19 if para is more than expected.
 */
int parse_param(const char *para, const char *def, struct instr_t *result) {
    char buffer[1024];

    if(para == NULL) {
        if(def[0] == '_') return 0;
        else return -18;
    }

    if(*para == '\0' && def[0] != '_') {
        return -11;
    }

    if(*para != '\0' && def[0] == '_') {
        return -16;
    }

    if(def[0] == '_') return 0;

    const char *orip = def;
    while (*orip != '\0') {
        if(*para == '\0') return -20;

        long long value;
        int tmp;
        switch (*orip) {
            case '!':
            tmp = parse_int(&value, para);
            if(tmp == -1) return -12; // format error
            para += tmp;
            break;
            case '$':
            para = get_next_word(para, buffer);
            value = index_of(gpr_names, 32, buffer);
            break;
            case '^':
            para = get_next_word(para, buffer);
            value = index_of(cop0r_names, 32, buffer);
            break;
            case '@':
            para = get_next_word(para, buffer);
            value = index_of(cop1r_names, 32, buffer);
            break;
            case '#':
            case '&':
            tmp = parse_int(&value, para);
            if(tmp == -1) return -13; // format error
            para += tmp;
            break;
            case '*':
            case '%':
            tmp = parse_int(&value, para);
            if(tmp == -1) return -14; // format error
            para += tmp;
            if(value < 0) return -15;
            break;
            default:
            if(*para != *orip) return -30;
            if(*orip == ' ') {
                para = str_first_not(para, ' ');
                ++orip;
                goto wh_end; // jump to while's end
            }

            ++orip, ++para;
            goto wh_end;
        }

        switch(_get_target_(++orip)) {
            case 1:
            result->imm = value;
            break;
            case 2:
            result->sa = value;
            break;
            case 3:
            result->rt = value;
            break;
            case 4:
            result->rd = value;
            break;
            case 5:
            result->rs = value;
        }

        orip += 2;

        wh_end:;
    }

    if(NULL != str_first_not(para, '\r')) return -19; // the line still something remain

    return 0;
}

/**
 * @brief Assemble a asm command.
 * 
 * @param str The asm language
 * @param offset The offset of the instruction (vma)
 * @param result The result instruction
 * @return int 0 if success, negative specified the error:
 * -10: no mnemonic found; 
 * -17: invalid mnemonic; 
 * others: see parse_para()
 */
int as(const char *str, unsigned int offset, struct instr_t *result) {
    char buffer[128]; // may overflow, need to improve in the future.
    const char *next = get_next_word(str, buffer);
    if(buffer[0] == '\0') return -10;
    
    int option = -1;
    strupr(buffer);
    for (int i = 0; i < OPTION_COUNT; ++i)
	{
		if (strcmp(instructions[i].name,  buffer) == 0)
			option = i;
	}
    if(option == -1) return -17;

    result->opcode = option;
    result->imm = 0;
    result->rs = 0;
    result->rt = 0;
    result->rd = 0;

    struct instr_def def;
    def = GetInstructionDefinitionByIndex(option);
    const char *para = str_first_not(next, ' ');

    int ret = parse_param(para, def.para, result);
    if(ret != 0) return ret;

    if(def.type == IT_imma) {
        result->imm -= offset + 4;
    }
    
    return 0;
}

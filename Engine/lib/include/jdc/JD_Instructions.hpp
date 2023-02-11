#pragma once
#include "./JD_Base.hpp"
#include "./JD_Class.hpp"
#include "./JD_ClassEx.hpp"
#include <xel/Byte.hpp>

namespace jdc
{

    constexpr const uint8_t OP_aaload = 0x32;
    constexpr const uint8_t OP_aastore = 0x53;
    constexpr const uint8_t OP_aconst_null = 0x01;

    constexpr const uint8_t OP_aload   = 0x19;
    constexpr const uint8_t OP_aload_0 = 0x2a;
    constexpr const uint8_t OP_aload_1 = 0x2b;
    constexpr const uint8_t OP_aload_2 = 0x2c;
    constexpr const uint8_t OP_aload_3 = 0x2d;

    constexpr const uint8_t OP_anewarray = 0xbd;
    constexpr const uint8_t OP_areturn = 0xb0;

    constexpr const uint8_t OP_arraylength = 0xbe;

    constexpr const uint8_t OP_astore   = 0x3a;
    constexpr const uint8_t OP_astore_0 = 0x4b;
    constexpr const uint8_t OP_astore_1 = 0x4c;
    constexpr const uint8_t OP_astore_2 = 0x4d;
    constexpr const uint8_t OP_astore_3 = 0x4e;

    constexpr const uint8_t OP_athrow = 0xbf;
    constexpr const uint8_t OP_baload = 0x33;
    constexpr const uint8_t OP_bastore = 0x54;
    constexpr const uint8_t OP_bipush = 0x10;
    constexpr const uint8_t OP_caload = 0x34;
    constexpr const uint8_t OP_castore = 0x55;
    constexpr const uint8_t OP_checkcast = 0xc0;

    constexpr const uint8_t OP_d2f = 0x90;
    constexpr const uint8_t OP_d2i = 0x8e;
    constexpr const uint8_t OP_d2l = 0x8f;
    constexpr const uint8_t OP_dadd = 0x63;
    constexpr const uint8_t OP_daload = 0x31;
    constexpr const uint8_t OP_dastore = 0x52;
    constexpr const uint8_t OP_dcmpg = 0x98; // if at least one of value1 or value2 is NaN, return 1
    constexpr const uint8_t OP_dcmpl = 0x97; // if at least one of value1 or value2 is NaN, return -1
    constexpr const uint8_t OP_dconst_0 = 0x0e;
    constexpr const uint8_t OP_dconst_1 = 0x0f;
    constexpr const uint8_t OP_ddiv = 0x6f;
    constexpr const uint8_t OP_dload   = 0x18;
    constexpr const uint8_t OP_dload_0 = 0x26;
    constexpr const uint8_t OP_dload_1 = 0x27;
    constexpr const uint8_t OP_dload_2 = 0x28;
    constexpr const uint8_t OP_dload_3 = 0x29;
    constexpr const uint8_t OP_dmul = 0x6b;
    constexpr const uint8_t OP_dneg = 0x77;
    constexpr const uint8_t OP_drem = 0x73;
    constexpr const uint8_t OP_dreturn = 0xaf;
    constexpr const uint8_t OP_dstore   = 0x39;
    constexpr const uint8_t OP_dstore_0 = 0x47;
    constexpr const uint8_t OP_dstore_1 = 0x48;
    constexpr const uint8_t OP_dstore_2 = 0x49;
    constexpr const uint8_t OP_dstore_3 = 0x4a;
    constexpr const uint8_t OP_dsub = 0x67;
    constexpr const uint8_t OP_dup = 0x59;
    constexpr const uint8_t OP_dup_x1 = 0x5a;
    constexpr const uint8_t OP_dup_x2 = 0x5b;
    constexpr const uint8_t OP_dup2 = 0x5c;
    constexpr const uint8_t OP_dup2_x1 = 0x5d;
    constexpr const uint8_t OP_dup2_x2 = 0x5e;

    constexpr const uint8_t OP_f2d = 0x8d;
    constexpr const uint8_t OP_f2i = 0x8b;
    constexpr const uint8_t OP_f2l = 0x8c;
    constexpr const uint8_t OP_fadd = 0x62;
    constexpr const uint8_t OP_faload = 0x30;
    constexpr const uint8_t OP_fastore = 0x51;
    constexpr const uint8_t OP_fcmpg = 0x96; //  at least one of value1 or value2 is NaN, return 1
    constexpr const uint8_t OP_fcmpl = 0x95; //  at least one of value1 or value2 is NaN, return -1
    constexpr const uint8_t OP_fconst_0 = 0x0b;
    constexpr const uint8_t OP_fconst_1 = 0x0c;
    constexpr const uint8_t OP_fconst_2 = 0x0d;
    constexpr const uint8_t OP_fdiv = 0x6e;
    constexpr const uint8_t OP_fload   = 0x17;
    constexpr const uint8_t OP_fload_0 = 0x22;
    constexpr const uint8_t OP_fload_1 = 0x23;
    constexpr const uint8_t OP_fload_2 = 0x24;
    constexpr const uint8_t OP_fload_3 = 0x25;
    constexpr const uint8_t OP_fmul = 0x6a;
    constexpr const uint8_t OP_fneg = 0x76;
    constexpr const uint8_t OP_frem = 0x72;
    constexpr const uint8_t OP_freturn = 0xae;
    constexpr const uint8_t OP_fstore   = 0x38;
    constexpr const uint8_t OP_fstore_0 = 0x43;
    constexpr const uint8_t OP_fstore_1 = 0x44;
    constexpr const uint8_t OP_fstore_2 = 0x45;
    constexpr const uint8_t OP_fstore_3 = 0x46;
    constexpr const uint8_t OP_fsub = 0x66;

    constexpr const uint8_t OP_getfield = 0xb4;
    constexpr const uint8_t OP_getstatic = 0xb2;
    constexpr const uint8_t OP_goto = 0xa7;
    constexpr const uint8_t OP_goto_w = 0xc8;
    constexpr const uint8_t OP_i2b = 0x91;
    constexpr const uint8_t OP_i2c = 0x92;
    constexpr const uint8_t OP_i2d = 0x87;
    constexpr const uint8_t OP_i2f = 0x86;
    constexpr const uint8_t OP_i2l = 0x85;
    constexpr const uint8_t OP_i2s = 0x93;
    constexpr const uint8_t OP_iadd = 0x60;
    constexpr const uint8_t OP_iaload = 0x2e;
    constexpr const uint8_t OP_iand = 0x7e;
    constexpr const uint8_t OP_iastore = 0x4f;
    constexpr const uint8_t OP_iconst_m1 = 0x02; // minus 1 (-1);
    constexpr const uint8_t OP_iconst_0  = 0x03;
    constexpr const uint8_t OP_iconst_1  = 0x04;
    constexpr const uint8_t OP_iconst_2  = 0x05;
    constexpr const uint8_t OP_iconst_3  = 0x06;
    constexpr const uint8_t OP_iconst_4  = 0x07;
    constexpr const uint8_t OP_iconst_5  = 0x08;
    constexpr const uint8_t OP_idiv = 0x6c;

    constexpr const uint8_t OP_if_acmpeq = 0xa5; // equal
    constexpr const uint8_t OP_if_acmpne = 0xa6; // not equal
    constexpr const uint8_t OP_if_icmpeq = 0x9f;
    constexpr const uint8_t OP_if_icmpne = 0xa0;
    constexpr const uint8_t OP_if_icmplt = 0xa1;
    constexpr const uint8_t OP_if_icmpge = 0xa2;
    constexpr const uint8_t OP_if_icmpgt = 0xa3;
    constexpr const uint8_t OP_if_icmple = 0xa4;
    constexpr const uint8_t OP_ifeq = 0x99;
    constexpr const uint8_t OP_ifne = 0x9a;
    constexpr const uint8_t OP_iflt = 0x9b;
    constexpr const uint8_t OP_ifge = 0x9c;
    constexpr const uint8_t OP_ifgt = 0x9d;
    constexpr const uint8_t OP_ifle = 0x9e;
    constexpr const uint8_t OP_ifnonnull = 0xc7;
    constexpr const uint8_t OP_ifnull = 0xc6;

    constexpr const uint8_t OP_iinc = 0x84;
    constexpr const uint8_t OP_iload = 0x15;
    constexpr const uint8_t OP_iload_0 = 0x1a;
    constexpr const uint8_t OP_iload_1 = 0x1b;
    constexpr const uint8_t OP_iload_2 = 0x1c;
    constexpr const uint8_t OP_iload_3 = 0x1d;
    constexpr const uint8_t OP_imul = 0x68;
    constexpr const uint8_t OP_ineg = 0x74;
    constexpr const uint8_t OP_instanceof = 0xc1;
    constexpr const uint8_t OP_invokedynamic = 0xba;
    constexpr const uint8_t OP_invokeinterface = 0xb9;
    constexpr const uint8_t OP_invokespecial = 0xb7;
    constexpr const uint8_t OP_invokestatic = 0xb8;
    constexpr const uint8_t OP_invokevirtual = 0xb6;
    constexpr const uint8_t OP_ior = 0x80;
    constexpr const uint8_t OP_irem = 0x70;
    constexpr const uint8_t OP_ireturn = 0xac;
    constexpr const uint8_t OP_ishl = 0x78;
    constexpr const uint8_t OP_ishr = 0x7a;
    constexpr const uint8_t OP_istore = 0x36;
    constexpr const uint8_t OP_istore_0 = 0x3b;
    constexpr const uint8_t OP_istore_1 = 0x3c;
    constexpr const uint8_t OP_istore_2 = 0x3d;
    constexpr const uint8_t OP_istore_3 = 0x3e;
    constexpr const uint8_t OP_isub = 0x64;
    constexpr const uint8_t OP_iushr = 0x7c;
    constexpr const uint8_t OP_ixor = 0x82;

    constexpr const uint8_t OP_jsr = 0xa8;
    constexpr const uint8_t OP_jsr_w = 0xc9;

    constexpr const uint8_t OP_l2d = 0x8a;
    constexpr const uint8_t OP_l2f = 0x89;
    constexpr const uint8_t OP_l2i = 0x88;
    constexpr const uint8_t OP_ladd = 0x61;
    constexpr const uint8_t OP_laload = 0x2f;
    constexpr const uint8_t OP_land = 0x7f;
    constexpr const uint8_t OP_lastore = 0x50;
    constexpr const uint8_t OP_lcmp = 0x94;
    constexpr const uint8_t OP_lconst_0 = 0x09;
    constexpr const uint8_t OP_lconst_1 = 0xa;
    constexpr const uint8_t OP_ldc = 0x12;
    constexpr const uint8_t OP_ldc_w = 0x13;
    constexpr const uint8_t OP_ldc2_w = 0x14;
    constexpr const uint8_t OP_ldiv = 0x6d;
    constexpr const uint8_t OP_lload = 0x16;
    constexpr const uint8_t OP_lload_0 = 0x1e;
    constexpr const uint8_t OP_lload_1 = 0x1f;
    constexpr const uint8_t OP_lload_2 = 0x20;
    constexpr const uint8_t OP_lload_3 = 0x21;
    constexpr const uint8_t OP_lmul = 0x69;
    constexpr const uint8_t OP_lneg = 0x75;

    constexpr const uint8_t OP_lookupswitch = 0xab;
    constexpr const uint8_t OP_lor = 0x81;
    constexpr const uint8_t OP_lrem = 0x71;
    constexpr const uint8_t OP_lreturn = 0xad;
    constexpr const uint8_t OP_lshl = 0x79;
    constexpr const uint8_t OP_lshr = 0x7b;
    constexpr const uint8_t OP_lstore = 0x37;
    constexpr const uint8_t OP_lstore_0 = 0x3f;
    constexpr const uint8_t OP_lstore_1 = 0x40;
    constexpr const uint8_t OP_lstore_2 = 0x41;
    constexpr const uint8_t OP_lstore_3 = 0x42;
    constexpr const uint8_t OP_lsub = 0x65;
    constexpr const uint8_t OP_lushr = 0x7d;
    constexpr const uint8_t OP_lxor = 0x83;

    constexpr const uint8_t OP_monitorenter = 0xc2;
    constexpr const uint8_t OP_monitorexit = 0xc3;
    constexpr const uint8_t OP_multianewarray = 0xc5;
    constexpr const uint8_t OP_new = 0xbb;
    constexpr const uint8_t OP_newarray = 0xbc;

    constexpr const uint8_t OP_nop = 0x0;
    constexpr const uint8_t OP_pop = 0x57;
    constexpr const uint8_t OP_pop2 = 0x58;
    constexpr const uint8_t OP_putfield = 0xb5;
    constexpr const uint8_t OP_putstatic = 0xb3;
    constexpr const uint8_t OP_ret = 0xa9;
    constexpr const uint8_t OP_return = 0xb1;

    constexpr const uint8_t OP_saload = 0x35;
    constexpr const uint8_t OP_sastore = 0x56;
    constexpr const uint8_t OP_sipush = 0x11;

    constexpr const uint8_t OP_swap = 0x5f;
    constexpr const uint8_t OP_tableswitch = 0xaa;
    constexpr const uint8_t OP_wide = 0xc4;

    X_GAME_API std::string BuildCode(const xMethodEx & Method);

}

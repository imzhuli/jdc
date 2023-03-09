#pragma once
#include "./_.hpp"

namespace jdc
{

    using xOpCode = uint8_t;

    constexpr const xOpCode OP_nop                 = 0x00;
    constexpr const xOpCode OP_aconst_null         = 0x01;
    constexpr const xOpCode OP_iconst_m1           = 0x02;
    constexpr const xOpCode OP_iconst_0            = 0x03;
    constexpr const xOpCode OP_iconst_1            = 0x04;
    constexpr const xOpCode OP_iconst_2            = 0x05;
    constexpr const xOpCode OP_iconst_3            = 0x06;
    constexpr const xOpCode OP_iconst_4            = 0x07;
    constexpr const xOpCode OP_iconst_5            = 0x08;
    constexpr const xOpCode OP_lconst_0            = 0x09;
    constexpr const xOpCode OP_lconst_1            = 0x0a;
    constexpr const xOpCode OP_fconst_0            = 0x0b;
    constexpr const xOpCode OP_fconst_1            = 0x0c;
    constexpr const xOpCode OP_fconst_2            = 0x0d;
    constexpr const xOpCode OP_dconst_0            = 0x0e;
    constexpr const xOpCode OP_dconst_1            = 0x0f;
    constexpr const xOpCode OP_bipush              = 0x10;
    constexpr const xOpCode OP_sipush              = 0x11;
    constexpr const xOpCode OP_ldc                 = 0x12;
    constexpr const xOpCode OP_ldc_w               = 0x13;
    constexpr const xOpCode OP_ldc2_w              = 0x14;
    constexpr const xOpCode OP_iload               = 0x15;
    constexpr const xOpCode OP_lload               = 0x16;
    constexpr const xOpCode OP_fload               = 0x17;
    constexpr const xOpCode OP_dload               = 0x18;
    constexpr const xOpCode OP_aload               = 0x19;
    constexpr const xOpCode OP_iload_0             = 0x1a;
    constexpr const xOpCode OP_iload_1             = 0x1b;
    constexpr const xOpCode OP_iload_2             = 0x1c;
    constexpr const xOpCode OP_iload_3             = 0x1d;
    constexpr const xOpCode OP_lload_0             = 0x1e;
    constexpr const xOpCode OP_lload_1             = 0x1f;
    constexpr const xOpCode OP_lload_2             = 0x20;
    constexpr const xOpCode OP_lload_3             = 0x21;
    constexpr const xOpCode OP_fload_0             = 0x22;
    constexpr const xOpCode OP_fload_1             = 0x23;
    constexpr const xOpCode OP_fload_2             = 0x24;
    constexpr const xOpCode OP_fload_3             = 0x25;
    constexpr const xOpCode OP_dload_0             = 0x26;
    constexpr const xOpCode OP_dload_1             = 0x27;
    constexpr const xOpCode OP_dload_2             = 0x28;
    constexpr const xOpCode OP_dload_3             = 0x29;
    constexpr const xOpCode OP_aload_0             = 0x2a;
    constexpr const xOpCode OP_aload_1             = 0x2b;
    constexpr const xOpCode OP_aload_2             = 0x2c;
    constexpr const xOpCode OP_aload_3             = 0x2d;
    constexpr const xOpCode OP_iaload              = 0x2e;
    constexpr const xOpCode OP_laload              = 0x2f;
    constexpr const xOpCode OP_faload              = 0x30;
    constexpr const xOpCode OP_daload              = 0x31;
    constexpr const xOpCode OP_aaload              = 0x32;
    constexpr const xOpCode OP_baload              = 0x33;
    constexpr const xOpCode OP_caload              = 0x34;
    constexpr const xOpCode OP_saload              = 0x35;
    constexpr const xOpCode OP_istore              = 0x36;
    constexpr const xOpCode OP_lstore              = 0x37;
    constexpr const xOpCode OP_fstore              = 0x38;
    constexpr const xOpCode OP_dstore              = 0x39;
    constexpr const xOpCode OP_astore              = 0x3a;
    constexpr const xOpCode OP_istore_0            = 0x3b;
    constexpr const xOpCode OP_istore_1            = 0x3c;
    constexpr const xOpCode OP_istore_2            = 0x3d;
    constexpr const xOpCode OP_istore_3            = 0x3e;
    constexpr const xOpCode OP_lstore_0            = 0x3f;
    constexpr const xOpCode OP_lstore_1            = 0x40;
    constexpr const xOpCode OP_lstore_2            = 0x41;
    constexpr const xOpCode OP_lstore_3            = 0x42;
    constexpr const xOpCode OP_fstore_0            = 0x43;
    constexpr const xOpCode OP_fstore_1            = 0x44;
    constexpr const xOpCode OP_fstore_2            = 0x45;
    constexpr const xOpCode OP_fstore_3            = 0x46;
    constexpr const xOpCode OP_dstore_0            = 0x47;
    constexpr const xOpCode OP_dstore_1            = 0x48;
    constexpr const xOpCode OP_dstore_2            = 0x49;
    constexpr const xOpCode OP_dstore_3            = 0x4a;
    constexpr const xOpCode OP_astore_0            = 0x4b;
    constexpr const xOpCode OP_astore_1            = 0x4c;
    constexpr const xOpCode OP_astore_2            = 0x4d;
    constexpr const xOpCode OP_astore_3            = 0x4e;
    constexpr const xOpCode OP_iastore             = 0x4f;
    constexpr const xOpCode OP_lastore             = 0x50;
    constexpr const xOpCode OP_fastore             = 0x51;
    constexpr const xOpCode OP_dastore             = 0x52;
    constexpr const xOpCode OP_aastore             = 0x53;
    constexpr const xOpCode OP_bastore             = 0x54;
    constexpr const xOpCode OP_castore             = 0x55;
    constexpr const xOpCode OP_sastore             = 0x56;
    constexpr const xOpCode OP_pop                 = 0x57;
    constexpr const xOpCode OP_pop2                = 0x58;
    constexpr const xOpCode OP_dup                 = 0x59;
    constexpr const xOpCode OP_dup_x1              = 0x5a;
    constexpr const xOpCode OP_dup_x2              = 0x5b;
    constexpr const xOpCode OP_dup2                = 0x5c;
    constexpr const xOpCode OP_dup2_x1             = 0x5d;
    constexpr const xOpCode OP_dup2_x2             = 0x5e;
    constexpr const xOpCode OP_swap                = 0x5f;
    constexpr const xOpCode OP_iadd                = 0x60;
    constexpr const xOpCode OP_ladd                = 0x61;
    constexpr const xOpCode OP_fadd                = 0x62;
    constexpr const xOpCode OP_dadd                = 0x63;
    constexpr const xOpCode OP_isub                = 0x64;
    constexpr const xOpCode OP_lsub                = 0x65;
    constexpr const xOpCode OP_fsub                = 0x66;
    constexpr const xOpCode OP_dsub                = 0x67;
    constexpr const xOpCode OP_imul                = 0x68;
    constexpr const xOpCode OP_lmul                = 0x69;
    constexpr const xOpCode OP_fmul                = 0x6a;
    constexpr const xOpCode OP_dmul                = 0x6b;
    constexpr const xOpCode OP_idiv                = 0x6c;
    constexpr const xOpCode OP_ldiv                = 0x6d;
    constexpr const xOpCode OP_fdiv                = 0x6e;
    constexpr const xOpCode OP_ddiv                = 0x6f;
    constexpr const xOpCode OP_irem                = 0x70;
    constexpr const xOpCode OP_lrem                = 0x71;
    constexpr const xOpCode OP_frem                = 0x72;
    constexpr const xOpCode OP_drem                = 0x73;
    constexpr const xOpCode OP_ineg                = 0x74;
    constexpr const xOpCode OP_lneg                = 0x75;
    constexpr const xOpCode OP_fneg                = 0x76;
    constexpr const xOpCode OP_dneg                = 0x77;
    constexpr const xOpCode OP_ishl                = 0x78;
    constexpr const xOpCode OP_lshl                = 0x79;
    constexpr const xOpCode OP_ishr                = 0x7a;
    constexpr const xOpCode OP_lshr                = 0x7b;
    constexpr const xOpCode OP_iushr               = 0x7c;
    constexpr const xOpCode OP_lushr               = 0x7d;
    constexpr const xOpCode OP_iand                = 0x7e;
    constexpr const xOpCode OP_land                = 0x7f;
    constexpr const xOpCode OP_ior                 = 0x80;
    constexpr const xOpCode OP_lor                 = 0x81;
    constexpr const xOpCode OP_ixor                = 0x82;
    constexpr const xOpCode OP_lxor                = 0x83;
    constexpr const xOpCode OP_iinc                = 0x84;
    constexpr const xOpCode OP_i2l                 = 0x85;
    constexpr const xOpCode OP_i2f                 = 0x86;
    constexpr const xOpCode OP_i2d                 = 0x87;
    constexpr const xOpCode OP_l2i                 = 0x88;
    constexpr const xOpCode OP_l2f                 = 0x89;
    constexpr const xOpCode OP_l2d                 = 0x8a;
    constexpr const xOpCode OP_f2i                 = 0x8b;
    constexpr const xOpCode OP_f2l                 = 0x8c;
    constexpr const xOpCode OP_f2d                 = 0x8d;
    constexpr const xOpCode OP_d2i                 = 0x8e;
    constexpr const xOpCode OP_d2l                 = 0x8f;
    constexpr const xOpCode OP_d2f                 = 0x90;
    constexpr const xOpCode OP_i2b                 = 0x91;
    constexpr const xOpCode OP_i2c                 = 0x92;
    constexpr const xOpCode OP_i2s                 = 0x93;
    constexpr const xOpCode OP_lcmp                = 0x94;
    constexpr const xOpCode OP_fcmpl               = 0x95;
    constexpr const xOpCode OP_fcmpg               = 0x96;
    constexpr const xOpCode OP_dcmpl               = 0x97;
    constexpr const xOpCode OP_dcmpg               = 0x98;
    constexpr const xOpCode OP_ifeq                = 0x99;
    constexpr const xOpCode OP_ifne                = 0x9a;
    constexpr const xOpCode OP_iflt                = 0x9b;
    constexpr const xOpCode OP_ifge                = 0x9c;
    constexpr const xOpCode OP_ifgt                = 0x9d;
    constexpr const xOpCode OP_ifle                = 0x9e;
    constexpr const xOpCode OP_if_icmpeq           = 0x9f;
    constexpr const xOpCode OP_if_icmpne           = 0xa0;
    constexpr const xOpCode OP_if_icmplt           = 0xa1;
    constexpr const xOpCode OP_if_icmpge           = 0xa2;
    constexpr const xOpCode OP_if_icmpgt           = 0xa3;
    constexpr const xOpCode OP_if_icmple           = 0xa4;
    constexpr const xOpCode OP_if_acmpeq           = 0xa5;
    constexpr const xOpCode OP_if_acmpne           = 0xa6;
    constexpr const xOpCode OP_goto                = 0xa7;
    constexpr const xOpCode OP_jsr                 = 0xa8;
    constexpr const xOpCode OP_ret                 = 0xa9;
    constexpr const xOpCode OP_tableswitch         = 0xaa;
    constexpr const xOpCode OP_lookupswitch        = 0xab;
    constexpr const xOpCode OP_ireturn             = 0xac;
    constexpr const xOpCode OP_lreturn             = 0xad;
    constexpr const xOpCode OP_freturn             = 0xae;
    constexpr const xOpCode OP_dreturn             = 0xaf;
    constexpr const xOpCode OP_areturn             = 0xb0;
    constexpr const xOpCode OP_return              = 0xb1;
    constexpr const xOpCode OP_getstatic           = 0xb2;
    constexpr const xOpCode OP_putstatic           = 0xb3;
    constexpr const xOpCode OP_getfield            = 0xb4;
    constexpr const xOpCode OP_putfield            = 0xb5;
    constexpr const xOpCode OP_invokevirtual       = 0xb6;
    constexpr const xOpCode OP_invokespecial       = 0xb7;
    constexpr const xOpCode OP_invokestatic        = 0xb8;
    constexpr const xOpCode OP_invokeinterface     = 0xb9;
    constexpr const xOpCode OP_invokedynamic       = 0xba;
    constexpr const xOpCode OP_new                 = 0xbb;
    constexpr const xOpCode OP_newarray            = 0xbc;
    constexpr const xOpCode OP_anewarray           = 0xbd;
    constexpr const xOpCode OP_arraylength         = 0xbe;
    constexpr const xOpCode OP_athrow              = 0xbf;
    constexpr const xOpCode OP_checkcast           = 0xc0;
    constexpr const xOpCode OP_instanceof          = 0xc1;
    constexpr const xOpCode OP_monitorenter        = 0xc2;
    constexpr const xOpCode OP_monitorexit         = 0xc3;
    constexpr const xOpCode OP_wide                = 0xc4;
    constexpr const xOpCode OP_multianewarray      = 0xc5;
    constexpr const xOpCode OP_ifnull              = 0xc6;
    constexpr const xOpCode OP_ifnonnull           = 0xc7;
    constexpr const xOpCode OP_goto_w              = 0xc8;
    constexpr const xOpCode OP_jsr_w               = 0xc9;
    constexpr const xOpCode OP_breakpoint          = 0xca;

    constexpr const xOpCode OP_RESERVED_impdep_1   = 0xfe; // implementation defined 1
    constexpr const xOpCode OP_RESERVED_impdep_2   = 0xff; // implementation defined 2

    X_GAME_API const char * GetOpName(xOpCode OpCode);
    X_INLINE   size_t GetOpCount() { return 0x00cb; }

}

#include "interrupt_handler.h"
#include "serial.h"
#include <stddef.h>
#include <stdint.h>

static void serial_write_str(const char *s)
{
    for (size_t i = 0; s[i]; i++)
        serial_write_char(s[i]);
}

// Print a hex byte (0x00-0xFF) to serial
void print_hex(uint8_t n) {
    const char hex[] = "0123456789ABCDEF";
    serial_write_char('0');
    serial_write_char('x');
    serial_write_char(hex[(n >> 4) & 0xF]);
    serial_write_char(hex[n & 0xF]);
}

// Generic interrupt/exception handler
void catch_all_handler(void *frame) {
    serial_write_str("Interrupt: unknown\n");
}

// Per-vector handlers (stubs)
#define MAKE_HANDLER(n) \
void catch_handler_##n(void *frame) { \
    serial_write_str("Interrupt: "); print_hex(n); serial_write_str("\n"); \
}

// Generate 256 handlers
MAKE_HANDLER(0)  MAKE_HANDLER(1)  MAKE_HANDLER(2)  MAKE_HANDLER(3)
MAKE_HANDLER(4)  MAKE_HANDLER(5)  MAKE_HANDLER(6)  MAKE_HANDLER(7)
MAKE_HANDLER(8)  MAKE_HANDLER(9)  MAKE_HANDLER(10) MAKE_HANDLER(11)
MAKE_HANDLER(12) MAKE_HANDLER(13) MAKE_HANDLER(14) MAKE_HANDLER(15)
MAKE_HANDLER(16) MAKE_HANDLER(17) MAKE_HANDLER(18) MAKE_HANDLER(19)
MAKE_HANDLER(20) MAKE_HANDLER(21) MAKE_HANDLER(22) MAKE_HANDLER(23)
MAKE_HANDLER(24) MAKE_HANDLER(25) MAKE_HANDLER(26) MAKE_HANDLER(27)
MAKE_HANDLER(28) MAKE_HANDLER(29) MAKE_HANDLER(30) MAKE_HANDLER(31)
MAKE_HANDLER(32) MAKE_HANDLER(33) MAKE_HANDLER(34) MAKE_HANDLER(35)
MAKE_HANDLER(36) MAKE_HANDLER(37) MAKE_HANDLER(38) MAKE_HANDLER(39)
MAKE_HANDLER(40) MAKE_HANDLER(41) MAKE_HANDLER(42) MAKE_HANDLER(43)
MAKE_HANDLER(44) MAKE_HANDLER(45) MAKE_HANDLER(46) MAKE_HANDLER(47)
MAKE_HANDLER(48) MAKE_HANDLER(49) MAKE_HANDLER(50) MAKE_HANDLER(51)
MAKE_HANDLER(52) MAKE_HANDLER(53) MAKE_HANDLER(54) MAKE_HANDLER(55)
MAKE_HANDLER(56) MAKE_HANDLER(57) MAKE_HANDLER(58) MAKE_HANDLER(59)
MAKE_HANDLER(60) MAKE_HANDLER(61) MAKE_HANDLER(62) MAKE_HANDLER(63)
MAKE_HANDLER(64) MAKE_HANDLER(65) MAKE_HANDLER(66) MAKE_HANDLER(67)
MAKE_HANDLER(68) MAKE_HANDLER(69) MAKE_HANDLER(70) MAKE_HANDLER(71)
MAKE_HANDLER(72) MAKE_HANDLER(73) MAKE_HANDLER(74) MAKE_HANDLER(75)
MAKE_HANDLER(76) MAKE_HANDLER(77) MAKE_HANDLER(78) MAKE_HANDLER(79)
MAKE_HANDLER(80) MAKE_HANDLER(81) MAKE_HANDLER(82) MAKE_HANDLER(83)
MAKE_HANDLER(84) MAKE_HANDLER(85) MAKE_HANDLER(86) MAKE_HANDLER(87)
MAKE_HANDLER(88) MAKE_HANDLER(89) MAKE_HANDLER(90) MAKE_HANDLER(91)
MAKE_HANDLER(92) MAKE_HANDLER(93) MAKE_HANDLER(94) MAKE_HANDLER(95)
MAKE_HANDLER(96) MAKE_HANDLER(97) MAKE_HANDLER(98) MAKE_HANDLER(99)
MAKE_HANDLER(100) MAKE_HANDLER(101) MAKE_HANDLER(102) MAKE_HANDLER(103)
MAKE_HANDLER(104) MAKE_HANDLER(105) MAKE_HANDLER(106) MAKE_HANDLER(107)
MAKE_HANDLER(108) MAKE_HANDLER(109) MAKE_HANDLER(110) MAKE_HANDLER(111)
MAKE_HANDLER(112) MAKE_HANDLER(113) MAKE_HANDLER(114) MAKE_HANDLER(115)
MAKE_HANDLER(116) MAKE_HANDLER(117) MAKE_HANDLER(118) MAKE_HANDLER(119)
MAKE_HANDLER(120) MAKE_HANDLER(121) MAKE_HANDLER(122) MAKE_HANDLER(123)
MAKE_HANDLER(124) MAKE_HANDLER(125) MAKE_HANDLER(126) MAKE_HANDLER(127)
MAKE_HANDLER(128) MAKE_HANDLER(129) MAKE_HANDLER(130) MAKE_HANDLER(131)
MAKE_HANDLER(132) MAKE_HANDLER(133) MAKE_HANDLER(134) MAKE_HANDLER(135)
MAKE_HANDLER(136) MAKE_HANDLER(137) MAKE_HANDLER(138) MAKE_HANDLER(139)
MAKE_HANDLER(140) MAKE_HANDLER(141) MAKE_HANDLER(142) MAKE_HANDLER(143)
MAKE_HANDLER(144) MAKE_HANDLER(145) MAKE_HANDLER(146) MAKE_HANDLER(147)
MAKE_HANDLER(148) MAKE_HANDLER(149) MAKE_HANDLER(150) MAKE_HANDLER(151)
MAKE_HANDLER(152) MAKE_HANDLER(153) MAKE_HANDLER(154) MAKE_HANDLER(155)
MAKE_HANDLER(156) MAKE_HANDLER(157) MAKE_HANDLER(158) MAKE_HANDLER(159)
MAKE_HANDLER(160) MAKE_HANDLER(161) MAKE_HANDLER(162) MAKE_HANDLER(163)
MAKE_HANDLER(164) MAKE_HANDLER(165) MAKE_HANDLER(166) MAKE_HANDLER(167)
MAKE_HANDLER(168) MAKE_HANDLER(169) MAKE_HANDLER(170) MAKE_HANDLER(171)
MAKE_HANDLER(172) MAKE_HANDLER(173) MAKE_HANDLER(174) MAKE_HANDLER(175)
MAKE_HANDLER(176) MAKE_HANDLER(177) MAKE_HANDLER(178) MAKE_HANDLER(179)
MAKE_HANDLER(180) MAKE_HANDLER(181) MAKE_HANDLER(182) MAKE_HANDLER(183)
MAKE_HANDLER(184) MAKE_HANDLER(185) MAKE_HANDLER(186) MAKE_HANDLER(187)
MAKE_HANDLER(188) MAKE_HANDLER(189) MAKE_HANDLER(190) MAKE_HANDLER(191)
MAKE_HANDLER(192) MAKE_HANDLER(193) MAKE_HANDLER(194) MAKE_HANDLER(195)
MAKE_HANDLER(196) MAKE_HANDLER(197) MAKE_HANDLER(198) MAKE_HANDLER(199)
MAKE_HANDLER(200) MAKE_HANDLER(201) MAKE_HANDLER(202) MAKE_HANDLER(203)
MAKE_HANDLER(204) MAKE_HANDLER(205) MAKE_HANDLER(206) MAKE_HANDLER(207)
MAKE_HANDLER(208) MAKE_HANDLER(209) MAKE_HANDLER(210) MAKE_HANDLER(211)
MAKE_HANDLER(212) MAKE_HANDLER(213) MAKE_HANDLER(214) MAKE_HANDLER(215)
MAKE_HANDLER(216) MAKE_HANDLER(217) MAKE_HANDLER(218) MAKE_HANDLER(219)
MAKE_HANDLER(220) MAKE_HANDLER(221) MAKE_HANDLER(222) MAKE_HANDLER(223)
MAKE_HANDLER(224) MAKE_HANDLER(225) MAKE_HANDLER(226) MAKE_HANDLER(227)
MAKE_HANDLER(228) MAKE_HANDLER(229) MAKE_HANDLER(230) MAKE_HANDLER(231)
MAKE_HANDLER(232) MAKE_HANDLER(233) MAKE_HANDLER(234) MAKE_HANDLER(235)
MAKE_HANDLER(236) MAKE_HANDLER(237) MAKE_HANDLER(238) MAKE_HANDLER(239)
MAKE_HANDLER(240) MAKE_HANDLER(241) MAKE_HANDLER(242) MAKE_HANDLER(243)
MAKE_HANDLER(244) MAKE_HANDLER(245) MAKE_HANDLER(246) MAKE_HANDLER(247)
MAKE_HANDLER(248) MAKE_HANDLER(249) MAKE_HANDLER(250) MAKE_HANDLER(251)
MAKE_HANDLER(252) MAKE_HANDLER(253) MAKE_HANDLER(254) MAKE_HANDLER(255)

// Table of handler pointers for use in idt.c
void (*const catch_handler_table[256])(void *) = {
    catch_handler_0, catch_handler_1, catch_handler_2, catch_handler_3,
    catch_handler_4, catch_handler_5, catch_handler_6, catch_handler_7,
    catch_handler_8, catch_handler_9, catch_handler_10, catch_handler_11,
    catch_handler_12, catch_handler_13, catch_handler_14, catch_handler_15,
    catch_handler_16, catch_handler_17, catch_handler_18, catch_handler_19,
    catch_handler_20, catch_handler_21, catch_handler_22, catch_handler_23,
    catch_handler_24, catch_handler_25, catch_handler_26, catch_handler_27,
    catch_handler_28, catch_handler_29, catch_handler_30, catch_handler_31,
    catch_handler_32, catch_handler_33, catch_handler_34, catch_handler_35,
    catch_handler_36, catch_handler_37, catch_handler_38, catch_handler_39,
    catch_handler_40, catch_handler_41, catch_handler_42, catch_handler_43,
    catch_handler_44, catch_handler_45, catch_handler_46, catch_handler_47,
    catch_handler_48, catch_handler_49, catch_handler_50, catch_handler_51,
    catch_handler_52, catch_handler_53, catch_handler_54, catch_handler_55,
    catch_handler_56, catch_handler_57, catch_handler_58, catch_handler_59,
    catch_handler_60, catch_handler_61, catch_handler_62, catch_handler_63,
    catch_handler_64, catch_handler_65, catch_handler_66, catch_handler_67,
    catch_handler_68, catch_handler_69, catch_handler_70, catch_handler_71,
    catch_handler_72, catch_handler_73, catch_handler_74, catch_handler_75,
    catch_handler_76, catch_handler_77, catch_handler_78, catch_handler_79,
    catch_handler_80, catch_handler_81, catch_handler_82, catch_handler_83,
    catch_handler_84, catch_handler_85, catch_handler_86, catch_handler_87,
    catch_handler_88, catch_handler_89, catch_handler_90, catch_handler_91,
    catch_handler_92, catch_handler_93, catch_handler_94, catch_handler_95,
    catch_handler_96, catch_handler_97, catch_handler_98, catch_handler_99,
    catch_handler_100, catch_handler_101, catch_handler_102, catch_handler_103,
    catch_handler_104, catch_handler_105, catch_handler_106, catch_handler_107,
    catch_handler_108, catch_handler_109, catch_handler_110, catch_handler_111,
    catch_handler_112, catch_handler_113, catch_handler_114, catch_handler_115,
    catch_handler_116, catch_handler_117, catch_handler_118, catch_handler_119,
    catch_handler_120, catch_handler_121, catch_handler_122, catch_handler_123,
    catch_handler_124, catch_handler_125, catch_handler_126, catch_handler_127,
    catch_handler_128, catch_handler_129, catch_handler_130, catch_handler_131,
    catch_handler_132, catch_handler_133, catch_handler_134, catch_handler_135,
    catch_handler_136, catch_handler_137, catch_handler_138, catch_handler_139,
    catch_handler_140, catch_handler_141, catch_handler_142, catch_handler_143,
    catch_handler_144, catch_handler_145, catch_handler_146, catch_handler_147,
    catch_handler_148, catch_handler_149, catch_handler_150, catch_handler_151,
    catch_handler_152, catch_handler_153, catch_handler_154, catch_handler_155,
    catch_handler_156, catch_handler_157, catch_handler_158, catch_handler_159,
    catch_handler_160, catch_handler_161, catch_handler_162, catch_handler_163,
    catch_handler_164, catch_handler_165, catch_handler_166, catch_handler_167,
    catch_handler_168, catch_handler_169, catch_handler_170, catch_handler_171,
    catch_handler_172, catch_handler_173, catch_handler_174, catch_handler_175,
    catch_handler_176, catch_handler_177, catch_handler_178, catch_handler_179,
    catch_handler_180, catch_handler_181, catch_handler_182, catch_handler_183,
    catch_handler_184, catch_handler_185, catch_handler_186, catch_handler_187,
    catch_handler_188, catch_handler_189, catch_handler_190, catch_handler_191,
    catch_handler_192, catch_handler_193, catch_handler_194, catch_handler_195,
    catch_handler_196, catch_handler_197, catch_handler_198, catch_handler_199,
    catch_handler_200, catch_handler_201, catch_handler_202, catch_handler_203,
    catch_handler_204, catch_handler_205, catch_handler_206, catch_handler_207,
    catch_handler_208, catch_handler_209, catch_handler_210, catch_handler_211,
    catch_handler_212, catch_handler_213, catch_handler_214, catch_handler_215,
    catch_handler_216, catch_handler_217, catch_handler_218, catch_handler_219,
    catch_handler_220, catch_handler_221, catch_handler_222, catch_handler_223,
    catch_handler_224, catch_handler_225, catch_handler_226, catch_handler_227,
    catch_handler_228, catch_handler_229, catch_handler_230, catch_handler_231,
    catch_handler_232, catch_handler_233, catch_handler_234, catch_handler_235,
    catch_handler_236, catch_handler_237, catch_handler_238, catch_handler_239,
    catch_handler_240, catch_handler_241, catch_handler_242, catch_handler_243,
    catch_handler_244, catch_handler_245, catch_handler_246, catch_handler_247,
    catch_handler_248, catch_handler_249, catch_handler_250, catch_handler_251,
    catch_handler_252, catch_handler_253, catch_handler_254, catch_handler_255
};

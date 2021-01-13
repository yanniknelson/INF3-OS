/* SPDX-License-Identifier: MIT */

/*
 * include/arch/x86/irq-entry-points.h
 * 
 * InfOS
 * Copyright (C) University of Edinburgh 2016.  All Rights Reserved.
 * 
 * Tom Spink <tspink@inf.ed.ac.uk>
 */
#pragma once

#include <arch/x86/irq.h>

typedef void (*irq_entry_point_t)(void);

#define DECLARE_IRQ(__nr) extern void __irq##__nr(void)

extern "C" {
	DECLARE_IRQ(0);
	DECLARE_IRQ(1);
	DECLARE_IRQ(2);
	DECLARE_IRQ(3);
	DECLARE_IRQ(4);
	DECLARE_IRQ(5);
	DECLARE_IRQ(6);
	DECLARE_IRQ(7);
	DECLARE_IRQ(8);
	DECLARE_IRQ(9);
	DECLARE_IRQ(10);
	DECLARE_IRQ(11);
	DECLARE_IRQ(12);
	DECLARE_IRQ(13);
	DECLARE_IRQ(14);
	DECLARE_IRQ(15);
	DECLARE_IRQ(16);
	DECLARE_IRQ(17);
	DECLARE_IRQ(18);
	DECLARE_IRQ(19);
	DECLARE_IRQ(20);
	DECLARE_IRQ(21);
	DECLARE_IRQ(22);
	DECLARE_IRQ(23);
	DECLARE_IRQ(24);
	DECLARE_IRQ(25);
	DECLARE_IRQ(26);
	DECLARE_IRQ(27);
	DECLARE_IRQ(28);
	DECLARE_IRQ(29);
	DECLARE_IRQ(30);
	DECLARE_IRQ(31);
	DECLARE_IRQ(32);
	DECLARE_IRQ(33);
	DECLARE_IRQ(34);
	DECLARE_IRQ(35);
	DECLARE_IRQ(36);
	DECLARE_IRQ(37);
	DECLARE_IRQ(38);
	DECLARE_IRQ(39);
	DECLARE_IRQ(40);
	DECLARE_IRQ(41);
	DECLARE_IRQ(42);
	DECLARE_IRQ(43);
	DECLARE_IRQ(44);
	DECLARE_IRQ(45);
	DECLARE_IRQ(46);
	DECLARE_IRQ(47);
	DECLARE_IRQ(48);
	DECLARE_IRQ(49);
	DECLARE_IRQ(50);
	DECLARE_IRQ(51);
	DECLARE_IRQ(52);
	DECLARE_IRQ(53);
	DECLARE_IRQ(54);
	DECLARE_IRQ(55);
	DECLARE_IRQ(56);
	DECLARE_IRQ(57);
	DECLARE_IRQ(58);
	DECLARE_IRQ(59);
	DECLARE_IRQ(60);
	DECLARE_IRQ(61);
	DECLARE_IRQ(62);
	DECLARE_IRQ(63);
	DECLARE_IRQ(64);
	DECLARE_IRQ(65);
	DECLARE_IRQ(66);
	DECLARE_IRQ(67);
	DECLARE_IRQ(68);
	DECLARE_IRQ(69);
	DECLARE_IRQ(70);
	DECLARE_IRQ(71);
	DECLARE_IRQ(72);
	DECLARE_IRQ(73);
	DECLARE_IRQ(74);
	DECLARE_IRQ(75);
	DECLARE_IRQ(76);
	DECLARE_IRQ(77);
	DECLARE_IRQ(78);
	DECLARE_IRQ(79);
	DECLARE_IRQ(80);
	DECLARE_IRQ(81);
	DECLARE_IRQ(82);
	DECLARE_IRQ(83);
	DECLARE_IRQ(84);
	DECLARE_IRQ(85);
	DECLARE_IRQ(86);
	DECLARE_IRQ(87);
	DECLARE_IRQ(88);
	DECLARE_IRQ(89);
	DECLARE_IRQ(90);
	DECLARE_IRQ(91);
	DECLARE_IRQ(92);
	DECLARE_IRQ(93);
	DECLARE_IRQ(94);
	DECLARE_IRQ(95);
	DECLARE_IRQ(96);
	DECLARE_IRQ(97);
	DECLARE_IRQ(98);
	DECLARE_IRQ(99);
	DECLARE_IRQ(100);
	DECLARE_IRQ(101);
	DECLARE_IRQ(102);
	DECLARE_IRQ(103);
	DECLARE_IRQ(104);
	DECLARE_IRQ(105);
	DECLARE_IRQ(106);
	DECLARE_IRQ(107);
	DECLARE_IRQ(108);
	DECLARE_IRQ(109);
	DECLARE_IRQ(110);
	DECLARE_IRQ(111);
	DECLARE_IRQ(112);
	DECLARE_IRQ(113);
	DECLARE_IRQ(114);
	DECLARE_IRQ(115);
	DECLARE_IRQ(116);
	DECLARE_IRQ(117);
	DECLARE_IRQ(118);
	DECLARE_IRQ(119);
	DECLARE_IRQ(120);
	DECLARE_IRQ(121);
	DECLARE_IRQ(122);
	DECLARE_IRQ(123);
	DECLARE_IRQ(124);
	DECLARE_IRQ(125);
	DECLARE_IRQ(126);
	DECLARE_IRQ(127);
	DECLARE_IRQ(128);
	DECLARE_IRQ(129);
	DECLARE_IRQ(130);
	DECLARE_IRQ(131);
	DECLARE_IRQ(132);
	DECLARE_IRQ(133);
	DECLARE_IRQ(134);
	DECLARE_IRQ(135);
	DECLARE_IRQ(136);
	DECLARE_IRQ(137);
	DECLARE_IRQ(138);
	DECLARE_IRQ(139);
	DECLARE_IRQ(140);
	DECLARE_IRQ(141);
	DECLARE_IRQ(142);
	DECLARE_IRQ(143);
	DECLARE_IRQ(144);
	DECLARE_IRQ(145);
	DECLARE_IRQ(146);
	DECLARE_IRQ(147);
	DECLARE_IRQ(148);
	DECLARE_IRQ(149);
	DECLARE_IRQ(150);
	DECLARE_IRQ(151);
	DECLARE_IRQ(152);
	DECLARE_IRQ(153);
	DECLARE_IRQ(154);
	DECLARE_IRQ(155);
	DECLARE_IRQ(156);
	DECLARE_IRQ(157);
	DECLARE_IRQ(158);
	DECLARE_IRQ(159);
	DECLARE_IRQ(160);
	DECLARE_IRQ(161);
	DECLARE_IRQ(162);
	DECLARE_IRQ(163);
	DECLARE_IRQ(164);
	DECLARE_IRQ(165);
	DECLARE_IRQ(166);
	DECLARE_IRQ(167);
	DECLARE_IRQ(168);
	DECLARE_IRQ(169);
	DECLARE_IRQ(170);
	DECLARE_IRQ(171);
	DECLARE_IRQ(172);
	DECLARE_IRQ(173);
	DECLARE_IRQ(174);
	DECLARE_IRQ(175);
	DECLARE_IRQ(176);
	DECLARE_IRQ(177);
	DECLARE_IRQ(178);
	DECLARE_IRQ(179);
	DECLARE_IRQ(180);
	DECLARE_IRQ(181);
	DECLARE_IRQ(182);
	DECLARE_IRQ(183);
	DECLARE_IRQ(184);
	DECLARE_IRQ(185);
	DECLARE_IRQ(186);
	DECLARE_IRQ(187);
	DECLARE_IRQ(188);
	DECLARE_IRQ(189);
	DECLARE_IRQ(190);
	DECLARE_IRQ(191);
	DECLARE_IRQ(192);
	DECLARE_IRQ(193);
	DECLARE_IRQ(194);
	DECLARE_IRQ(195);
	DECLARE_IRQ(196);
	DECLARE_IRQ(197);
	DECLARE_IRQ(198);
	DECLARE_IRQ(199);
	DECLARE_IRQ(200);
	DECLARE_IRQ(201);
	DECLARE_IRQ(202);
	DECLARE_IRQ(203);
	DECLARE_IRQ(204);
	DECLARE_IRQ(205);
	DECLARE_IRQ(206);
	DECLARE_IRQ(207);
	DECLARE_IRQ(208);
	DECLARE_IRQ(209);
	DECLARE_IRQ(210);
	DECLARE_IRQ(211);
	DECLARE_IRQ(212);
	DECLARE_IRQ(213);
	DECLARE_IRQ(214);
	DECLARE_IRQ(215);
	DECLARE_IRQ(216);
	DECLARE_IRQ(217);
	DECLARE_IRQ(218);
	DECLARE_IRQ(219);
	DECLARE_IRQ(220);
	DECLARE_IRQ(221);
	DECLARE_IRQ(222);
	DECLARE_IRQ(223);
	DECLARE_IRQ(224);
	DECLARE_IRQ(225);
	DECLARE_IRQ(226);
	DECLARE_IRQ(227);
	DECLARE_IRQ(228);
	DECLARE_IRQ(229);
	DECLARE_IRQ(230);
	DECLARE_IRQ(231);
	DECLARE_IRQ(232);
	DECLARE_IRQ(233);
	DECLARE_IRQ(234);
	DECLARE_IRQ(235);
	DECLARE_IRQ(236);
	DECLARE_IRQ(237);
	DECLARE_IRQ(238);
	DECLARE_IRQ(239);
	DECLARE_IRQ(240);
	DECLARE_IRQ(241);
	DECLARE_IRQ(242);
	DECLARE_IRQ(243);
	DECLARE_IRQ(244);
	DECLARE_IRQ(245);
	DECLARE_IRQ(246);
	DECLARE_IRQ(247);
	DECLARE_IRQ(248);
	DECLARE_IRQ(249);
	DECLARE_IRQ(250);
	DECLARE_IRQ(251);
	DECLARE_IRQ(252);
	DECLARE_IRQ(253);
	DECLARE_IRQ(254);
	DECLARE_IRQ(255);
}

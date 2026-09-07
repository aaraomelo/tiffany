`timescale 1ns/1ps

/*
 * ecp5_sim_ext.v - functional models for ECP5 primitives that the OSS
 * toolchain ships only as blackboxes (cells_bb.v): DP16KD, MULT18X18D,
 * DCCA.  They are whitebox functional models written for the
 * post-P&R structural netlist (micro_divmc_post_struct.v).  This is a
 * functional verification aid, NOT a timing model.
 *
 * Port lists match the yosys/nextpnr cell views (scalar per-bit ports)
 * so the structural netlist binds without renaming.
 */

/* =====================================================================
 * DP16KD - Dual-port EBR RAM, 512x36 physical word.
 * Capacity is oversized on purpose (512 rows x 36 bits); address/data
 * slicing per DATA_WIDTH follows the yosys EBR mapping:
 *   w18: mem[row] 36-bit word, DIA[8:0]->lane0, DIA[17:9]->lane1;
 *        byte select = ADA[0]/ADA[1].
 *   w9 : word address w = ADA[13:3]; lane = ADA[4:3] (0..3).
 *   w4 : word address w = ADA[13:2]; lane = ADA[2:0] (0..7).
 *   w2 : word address w = ADA[13:1]; lane = ADA[1:0] (0..15).
 *   w1 : word address w = ADA[13:0]; lane = ADA[0].
 * These formulae agree with brams_map_16kd.v (ADA[i] = PORT_A_ADDR[i]),
 * i.e. pin w of the word address is presented on ADA[w+width_log2].
 * ===================================================================== */
module DP16KD(
	input  DIA17, DIA16, DIA15, DIA14, DIA13, DIA12, DIA11, DIA10, DIA9, DIA8, DIA7, DIA6, DIA5, DIA4, DIA3, DIA2, DIA1, DIA0,
	input  ADA13, ADA12, ADA11, ADA10, ADA9, ADA8, ADA7, ADA6, ADA5, ADA4, ADA3, ADA2, ADA1, ADA0,
	input  CEA, OCEA, CLKA, WEA, RSTA,
	input  CSA2, CSA1, CSA0,
	output DOA17, DOA16, DOA15, DOA14, DOA13, DOA12, DOA11, DOA10, DOA9, DOA8, DOA7, DOA6, DOA5, DOA4, DOA3, DOA2, DOA1, DOA0,

	input  DIB17, DIB16, DIB15, DIB14, DIB13, DIB12, DIB11, DIB10, DIB9, DIB8, DIB7, DIB6, DIB5, DIB4, DIB3, DIB2, DIB1, DIB0,
	input  ADB13, ADB12, ADB11, ADB10, ADB9, ADB8, ADB7, ADB6, ADB5, ADB4, ADB3, ADB2, ADB1, ADB0,
	input  CEB, OCEB, CLKB, WEB, RSTB,
	input  CSB2, CSB1, CSB0,
	output DOB17, DOB16, DOB15, DOB14, DOB13, DOB12, DOB11, DOB10, DOB9, DOB8, DOB7, DOB6, DOB5, DOB4, DOB3, DOB2, DOB1, DOB0
);
	parameter integer DATA_WIDTH_A = 18;
	parameter integer DATA_WIDTH_B = 18;

	parameter REGMODE_A = "NOREG";
	parameter REGMODE_B = "NOREG";

	parameter RESETMODE = "SYNC";
	parameter ASYNC_RESET_RELEASE = "SYNC";

	parameter CSDECODE_A = "0b000";
	parameter CSDECODE_B = "0b000";

	parameter WRITEMODE_A = "NORMAL";
	parameter WRITEMODE_B = "NORMAL";

	parameter DIA17MUX = "DIA17";
	parameter DIA16MUX = "DIA16";
	parameter DIA15MUX = "DIA15";
	parameter DIA14MUX = "DIA14";
	parameter DIA13MUX = "DIA13";
	parameter DIA12MUX = "DIA12";
	parameter DIA11MUX = "DIA11";
	parameter DIA10MUX = "DIA10";
	parameter DIA9MUX = "DIA9";
	parameter DIA8MUX = "DIA8";
	parameter DIA7MUX = "DIA7";
	parameter DIA6MUX = "DIA6";
	parameter DIA5MUX = "DIA5";
	parameter DIA4MUX = "DIA4";
	parameter DIA3MUX = "DIA3";
	parameter DIA2MUX = "DIA2";
	parameter DIA1MUX = "DIA1";
	parameter DIA0MUX = "DIA0";
	parameter ADA13MUX = "ADA13";
	parameter ADA12MUX = "ADA12";
	parameter ADA11MUX = "ADA11";
	parameter ADA10MUX = "ADA10";
	parameter ADA9MUX = "ADA9";
	parameter ADA8MUX = "ADA8";
	parameter ADA7MUX = "ADA7";
	parameter ADA6MUX = "ADA6";
	parameter ADA5MUX = "ADA5";
	parameter ADA4MUX = "ADA4";
	parameter ADA3MUX = "ADA3";
	parameter ADA2MUX = "ADA2";
	parameter ADA1MUX = "ADA1";
	parameter ADA0MUX = "ADA0";
	parameter DIB17MUX = "DIB17";
	parameter DIB16MUX = "DIB16";
	parameter DIB15MUX = "DIB15";
	parameter DIB14MUX = "DIB14";
	parameter DIB13MUX = "DIB13";
	parameter DIB12MUX = "DIB12";
	parameter DIB11MUX = "DIB11";
	parameter DIB10MUX = "DIB10";
	parameter DIB9MUX = "DIB9";
	parameter DIB8MUX = "DIB8";
	parameter DIB7MUX = "DIB7";
	parameter DIB6MUX = "DIB6";
	parameter DIB5MUX = "DIB5";
	parameter DIB4MUX = "DIB4";
	parameter DIB3MUX = "DIB3";
	parameter DIB2MUX = "DIB2";
	parameter DIB1MUX = "DIB1";
	parameter DIB0MUX = "DIB0";
	parameter ADB13MUX = "ADB13";
	parameter ADB12MUX = "ADB12";
	parameter ADB11MUX = "ADB11";
	parameter ADB10MUX = "ADB10";
	parameter ADB9MUX = "ADB9";
	parameter ADB8MUX = "ADB8";
	parameter ADB7MUX = "ADB7";
	parameter ADB6MUX = "ADB6";
	parameter ADB5MUX = "ADB5";
	parameter ADB4MUX = "ADB4";
	parameter ADB3MUX = "ADB3";
	parameter ADB2MUX = "ADB2";
	parameter ADB1MUX = "ADB1";
	parameter ADB0MUX = "ADB0";
	parameter CEAMUX = "CEA";
	parameter CEBMUX = "CEB";
	parameter OCEAMUX = "OCEA";
	parameter OCEBMUX = "OCEB";
	parameter CLKAMUX = "CLKA";
	parameter CLKBMUX = "CLKB";
	parameter WEAMUX = "WEA";
	parameter WEBMUX = "WEB";
	parameter RSTAMUX = "RSTA";
	parameter RSTBMUX = "RSTB";
	parameter CSA2MUX = "CSA2";
	parameter CSA1MUX = "CSA1";
	parameter CSA0MUX = "CSA0";
	parameter CSB2MUX = "CSB2";
	parameter CSB1MUX = "CSB1";
	parameter CSB0MUX = "CSB0";

	parameter GSR = "ENABLED";
	parameter WID = 0;

	parameter INITVAL_00 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_01 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_02 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_03 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_04 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_05 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_06 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_07 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_08 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_09 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_0A = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_0B = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_0C = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_0D = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_0E = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_0F = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_10 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_11 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_12 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_13 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_14 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_15 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_16 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_17 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_18 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_19 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_1A = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_1B = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_1C = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_1D = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_1E = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_1F = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_20 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_21 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_22 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_23 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_24 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_25 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_26 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_27 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_28 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_29 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_2A = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_2B = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_2C = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_2D = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_2E = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_2F = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_30 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_31 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_32 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_33 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_34 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_35 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_36 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_37 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_38 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_39 = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_3A = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_3B = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_3C = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_3D = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_3E = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;
	parameter INITVAL_3F = 320'h00000000000000000000000000000000000000000000000000000000000000000000000000000000;

	function [2:0] csdecode;
		input [47:0] s;
		begin
			csdecode = 3'b000;
			if (s == "0b000") csdecode = 3'b000;
			else if (s == "0b001") csdecode = 3'b001;
			else if (s == "0b010") csdecode = 3'b010;
			else if (s == "0b011") csdecode = 3'b011;
			else if (s == "0b100") csdecode = 3'b100;
			else if (s == "0b101") csdecode = 3'b101;
			else if (s == "0b110") csdecode = 3'b110;
			else csdecode = 3'b111;
		end
	endfunction

	/* effective value of a pin under its MUX control */
	function pin_eff;
		input muxinv;
		input muxone;
		input muxzero;
		input v;
		reg hi;
		begin
			hi = (v === 1'b1) || (v === 1'bx) || (v === 1'bz);
			if (muxone) pin_eff = 1'b1;
			else if (muxzero) pin_eff = 1'b0;
			else if (muxinv) pin_eff = ~hi;
			else pin_eff = hi;
		end
	endfunction

	function cs_sel;
		input [2:0] code;
		input [2:0] pin;
		begin
			cs_sel =
				((pin[0] === code[0]) || (pin[0] === 1'bx) || (pin[0] === 1'bz)) &&
				((pin[1] === code[1]) || (pin[1] === 1'bx) || (pin[1] === 1'bz)) &&
				((pin[2] === code[2]) || (pin[2] === 1'bx) || (pin[2] === 1'bz));
		end
	endfunction

	wire [17:0] dia = {
		pin_eff(DIA17MUX == "INV", DIA17MUX == "1", DIA17MUX == "0", DIA17),
		pin_eff(DIA16MUX == "INV", DIA16MUX == "1", DIA16MUX == "0", DIA16),
		pin_eff(DIA15MUX == "INV", DIA15MUX == "1", DIA15MUX == "0", DIA15),
		pin_eff(DIA14MUX == "INV", DIA14MUX == "1", DIA14MUX == "0", DIA14),
		pin_eff(DIA13MUX == "INV", DIA13MUX == "1", DIA13MUX == "0", DIA13),
		pin_eff(DIA12MUX == "INV", DIA12MUX == "1", DIA12MUX == "0", DIA12),
		pin_eff(DIA11MUX == "INV", DIA11MUX == "1", DIA11MUX == "0", DIA11),
		pin_eff(DIA10MUX == "INV", DIA10MUX == "1", DIA10MUX == "0", DIA10),
		pin_eff(DIA9MUX  == "INV", DIA9MUX  == "1", DIA9MUX  == "0", DIA9),
		pin_eff(DIA8MUX  == "INV", DIA8MUX  == "1", DIA8MUX  == "0", DIA8),
		pin_eff(DIA7MUX  == "INV", DIA7MUX  == "1", DIA7MUX  == "0", DIA7),
		pin_eff(DIA6MUX  == "INV", DIA6MUX  == "1", DIA6MUX  == "0", DIA6),
		pin_eff(DIA5MUX  == "INV", DIA5MUX  == "1", DIA5MUX  == "0", DIA5),
		pin_eff(DIA4MUX  == "INV", DIA4MUX  == "1", DIA4MUX  == "0", DIA4),
		pin_eff(DIA3MUX  == "INV", DIA3MUX  == "1", DIA3MUX  == "0", DIA3),
		pin_eff(DIA2MUX  == "INV", DIA2MUX  == "1", DIA2MUX  == "0", DIA2),
		pin_eff(DIA1MUX  == "INV", DIA1MUX  == "1", DIA1MUX  == "0", DIA1),
		pin_eff(DIA0MUX  == "INV", DIA0MUX  == "1", DIA0MUX  == "0", DIA0)};

	wire [17:0] dib = {
		pin_eff(DIB17MUX == "INV", DIB17MUX == "1", DIB17MUX == "0", DIB17),
		pin_eff(DIB16MUX == "INV", DIB16MUX == "1", DIB16MUX == "0", DIB16),
		pin_eff(DIB15MUX == "INV", DIB15MUX == "1", DIB15MUX == "0", DIB15),
		pin_eff(DIB14MUX == "INV", DIB14MUX == "1", DIB14MUX == "0", DIB14),
		pin_eff(DIB13MUX == "INV", DIB13MUX == "1", DIB13MUX == "0", DIB13),
		pin_eff(DIB12MUX == "INV", DIB12MUX == "1", DIB12MUX == "0", DIB12),
		pin_eff(DIB11MUX == "INV", DIB11MUX == "1", DIB11MUX == "0", DIB11),
		pin_eff(DIB10MUX == "INV", DIB10MUX == "1", DIB10MUX == "0", DIB10),
		pin_eff(DIB9MUX  == "INV", DIB9MUX  == "1", DIB9MUX  == "0", DIB9),
		pin_eff(DIB8MUX  == "INV", DIB8MUX  == "1", DIB8MUX  == "0", DIB8),
		pin_eff(DIB7MUX  == "INV", DIB7MUX  == "1", DIB7MUX  == "0", DIB7),
		pin_eff(DIB6MUX  == "INV", DIB6MUX  == "1", DIB6MUX  == "0", DIB6),
		pin_eff(DIB5MUX  == "INV", DIB5MUX  == "1", DIB5MUX  == "0", DIB5),
		pin_eff(DIB4MUX  == "INV", DIB4MUX  == "1", DIB4MUX  == "0", DIB4),
		pin_eff(DIB3MUX  == "INV", DIB3MUX  == "1", DIB3MUX  == "0", DIB3),
		pin_eff(DIB2MUX  == "INV", DIB2MUX  == "1", DIB2MUX  == "0", DIB2),
		pin_eff(DIB1MUX  == "INV", DIB1MUX  == "1", DIB1MUX  == "0", DIB1),
		pin_eff(DIB0MUX  == "INV", DIB0MUX  == "1", DIB0MUX  == "0", DIB0)};

	wire [13:0] ada = {
		pin_eff(ADA13MUX == "INV", ADA13MUX == "1", ADA13MUX == "0", ADA13),
		pin_eff(ADA12MUX == "INV", ADA12MUX == "1", ADA12MUX == "0", ADA12),
		pin_eff(ADA11MUX == "INV", ADA11MUX == "1", ADA11MUX == "0", ADA11),
		pin_eff(ADA10MUX == "INV", ADA10MUX == "1", ADA10MUX == "0", ADA10),
		pin_eff(ADA9MUX  == "INV", ADA9MUX  == "1", ADA9MUX  == "0", ADA9),
		pin_eff(ADA8MUX  == "INV", ADA8MUX  == "1", ADA8MUX  == "0", ADA8),
		pin_eff(ADA7MUX  == "INV", ADA7MUX  == "1", ADA7MUX  == "0", ADA7),
		pin_eff(ADA6MUX  == "INV", ADA6MUX  == "1", ADA6MUX  == "0", ADA6),
		pin_eff(ADA5MUX  == "INV", ADA5MUX  == "1", ADA5MUX  == "0", ADA5),
		pin_eff(ADA4MUX  == "INV", ADA4MUX  == "1", ADA4MUX  == "0", ADA4),
		pin_eff(ADA3MUX  == "INV", ADA3MUX  == "1", ADA3MUX  == "0", ADA3),
		pin_eff(ADA2MUX  == "INV", ADA2MUX  == "1", ADA2MUX  == "0", ADA2),
		pin_eff(ADA1MUX  == "INV", ADA1MUX  == "1", ADA1MUX  == "0", ADA1),
		pin_eff(ADA0MUX  == "INV", ADA0MUX  == "1", ADA0MUX  == "0", ADA0)};

	wire [13:0] adb = {
		pin_eff(ADB13MUX == "INV", ADB13MUX == "1", ADB13MUX == "0", ADB13),
		pin_eff(ADB12MUX == "INV", ADB12MUX == "1", ADB12MUX == "0", ADB12),
		pin_eff(ADB11MUX == "INV", ADB11MUX == "1", ADB11MUX == "0", ADB11),
		pin_eff(ADB10MUX == "INV", ADB10MUX == "1", ADB10MUX == "0", ADB10),
		pin_eff(ADB9MUX  == "INV", ADB9MUX  == "1", ADB9MUX  == "0", ADB9),
		pin_eff(ADB8MUX  == "INV", ADB8MUX  == "1", ADB8MUX  == "0", ADB8),
		pin_eff(ADB7MUX  == "INV", ADB7MUX  == "1", ADB7MUX  == "0", ADB7),
		pin_eff(ADB6MUX  == "INV", ADB6MUX  == "1", ADB6MUX  == "0", ADB6),
		pin_eff(ADB5MUX  == "INV", ADB5MUX  == "1", ADB5MUX  == "0", ADB5),
		pin_eff(ADB4MUX  == "INV", ADB4MUX  == "1", ADB4MUX  == "0", ADB4),
		pin_eff(ADB3MUX  == "INV", ADB3MUX  == "1", ADB3MUX  == "0", ADB3),
		pin_eff(ADB2MUX  == "INV", ADB2MUX  == "1", ADB2MUX  == "0", ADB2),
		pin_eff(ADB1MUX  == "INV", ADB1MUX  == "1", ADB1MUX  == "0", ADB1),
		pin_eff(ADB0MUX  == "INV", ADB0MUX  == "1", ADB0MUX  == "0", ADB0)};

	reg [35:0] mem [0:511];
	reg [17:0] doa_lat;
	reg [17:0] dob_lat;

	function [17:0] read_core;
		input integer dw;
		input [13:0] a;
		begin
			read_core = 18'b0;
			case (dw)
				1: read_core[0] = mem[a[13:5]][a[4:0]];
				2: read_core[1:0] = mem[a[13:5]][a[4:1]*2 +: 2];
				4: read_core[3:0] = mem[a[13:5]][a[4:2]*4 +: 4];
				9: read_core[8:0] = mem[a[13:5]][a[4:3]*9 +: 9];
				default: read_core[17:0] = mem[a[13:5]][a[4]*18 +: 18];
			endcase
		end
	endfunction

	function [17:0] w18;
		input [13:0] a;
		input [17:0] d;
		begin
			w18 = mem[a[13:5]];
			if (a[0]) w18[17:9]  = d[17:9];
			if (a[1]) w18[8:0]   = d[8:0];
		end
	endfunction

	integer iw, js, nit;
	reg [319:0] sl;

	initial begin
		for (iw = 0; iw < 512; iw = iw + 1)
			mem[iw] = 36'b0;
		for (nit = 0; nit < 64; nit = nit + 1) begin
			sl = 320'b0;
			case (nit)
				0: sl = INITVAL_00; 1: sl = INITVAL_01; 2: sl = INITVAL_02; 3: sl = INITVAL_03;
				4: sl = INITVAL_04; 5: sl = INITVAL_05; 6: sl = INITVAL_06; 7: sl = INITVAL_07;
				8: sl = INITVAL_08; 9: sl = INITVAL_09; 10: sl = INITVAL_0A; 11: sl = INITVAL_0B;
				12: sl = INITVAL_0C; 13: sl = INITVAL_0D; 14: sl = INITVAL_0E; 15: sl = INITVAL_0F;
				16: sl = INITVAL_10; 17: sl = INITVAL_11; 18: sl = INITVAL_12; 19: sl = INITVAL_13;
				20: sl = INITVAL_14; 21: sl = INITVAL_15; 22: sl = INITVAL_16; 23: sl = INITVAL_17;
				24: sl = INITVAL_18; 25: sl = INITVAL_19; 26: sl = INITVAL_1A; 27: sl = INITVAL_1B;
				28: sl = INITVAL_1C; 29: sl = INITVAL_1D; 30: sl = INITVAL_1E; 31: sl = INITVAL_1F;
				32: sl = INITVAL_20; 33: sl = INITVAL_21; 34: sl = INITVAL_22; 35: sl = INITVAL_23;
				36: sl = INITVAL_24; 37: sl = INITVAL_25; 38: sl = INITVAL_26; 39: sl = INITVAL_27;
				40: sl = INITVAL_28; 41: sl = INITVAL_29; 42: sl = INITVAL_2A; 43: sl = INITVAL_2B;
				44: sl = INITVAL_2C; 45: sl = INITVAL_2D; 46: sl = INITVAL_2E; 47: sl = INITVAL_2F;
				48: sl = INITVAL_30; 49: sl = INITVAL_31; 50: sl = INITVAL_32; 51: sl = INITVAL_33;
				52: sl = INITVAL_34; 53: sl = INITVAL_35; 54: sl = INITVAL_36; 55: sl = INITVAL_37;
				56: sl = INITVAL_38; 57: sl = INITVAL_39; 58: sl = INITVAL_3A; 59: sl = INITVAL_3B;
				60: sl = INITVAL_3C; 61: sl = INITVAL_3D; 62: sl = INITVAL_3E; 63: sl = INITVAL_3F;
			endcase
			for (js = 0; js < 16; js = js + 1) begin
				iw = nit*16 + js;
				mem[iw >> 1][(iw & 1)*18 +: 18] = sl[js*20 +: 18];
			end
		end
		doa_lat = 18'b0;
		dob_lat = 18'b0;
	end

	wire clka_eff = (CLKAMUX == "INV") ? ~CLKA : CLKA;
	wire clkb_eff = (CLKBMUX == "INV") ? ~CLKB : CLKB;

	wire cea_eff  = pin_eff(CEAMUX  == "INV", CEAMUX  == "1",  CEAMUX  == "0",  CEA);
	wire ceb_eff  = pin_eff(CEBMUX  == "INV", CEBMUX  == "1",  CEBMUX  == "0",  CEB);
	wire ocea_eff = pin_eff(OCEAMUX == "INV", OCEAMUX == "1",  OCEAMUX == "0",  OCEA);
	wire oceb_eff = pin_eff(OCEBMUX == "INV", OCEBMUX == "1",  OCEBMUX == "0",  OCEB);
	wire rsta_eff = pin_eff(RSTAMUX == "INV", RSTAMUX == "1",  RSTAMUX == "0",  RSTA);
	wire rstb_eff = pin_eff(RSTBMUX == "INV", RSTBMUX == "1",  RSTBMUX == "0",  RSTB);
	wire wea_eff  = pin_eff(WEAMUX  == "INV", WEAMUX  == "1",  WEAMUX  == "0",  WEA);
	wire web_eff  = pin_eff(WEBMUX  == "INV", WEBMUX  == "1",  WEBMUX  == "0",  WEB);

	wire [2:0] csa_eff = {
		pin_eff(CSA2MUX == "INV", CSA2MUX == "1", CSA2MUX == "0", CSA2),
		pin_eff(CSA1MUX == "INV", CSA1MUX == "1", CSA1MUX == "0", CSA1),
		pin_eff(CSA0MUX == "INV", CSA0MUX == "1", CSA0MUX == "0", CSA0)};
	wire [2:0] csb_eff = {
		pin_eff(CSB2MUX == "INV", CSB2MUX == "1", CSB2MUX == "0", CSB2),
		pin_eff(CSB1MUX == "INV", CSB1MUX == "1", CSB1MUX == "0", CSB1),
		pin_eff(CSB0MUX == "INV", CSB0MUX == "1", CSB0MUX == "0", CSB0)};

	wire sel_a = cs_sel(csdecode(CSDECODE_A), csa_eff);
	wire sel_b = cs_sel(csdecode(CSDECODE_B), csb_eff);

	wire rsta_async = (RESETMODE == "ASYNC") ? rsta_eff : 1'b0;
	wire rstb_async = (RESETMODE == "ASYNC") ? rstb_eff : 1'b0;

	always @(posedge clka_eff or posedge rsta_async) begin
		if (rsta_eff) begin
			doa_lat <= 18'b0;
		end else if (cea_eff && sel_a) begin
			if (wea_eff) begin
				case (DATA_WIDTH_A)
					1: mem[ada[13:5]][ada[4:0]] <= dia[0];
					2: mem[ada[13:5]][ada[4:1]*2 +: 2] <= dia[1:0];
					4: mem[ada[13:5]][ada[4:2]*4 +: 4] <= dia[3:0];
					9: mem[ada[13:5]][ada[4:3]*9 +: 9] <= dia[8:0];
					default: mem[ada[13:5]] <= w18(ada, dia);
				endcase
			end
			doa_lat <= read_core(DATA_WIDTH_A, ada);
		end
	end

	always @(posedge clkb_eff or posedge rstb_async) begin
		if (rstb_eff) begin
			dob_lat <= 18'b0;
		end else if (ceb_eff && sel_b) begin
			if (web_eff) begin
				case (DATA_WIDTH_B)
					1: mem[adb[13:5]][adb[4:0]] <= dib[0];
					2: mem[adb[13:5]][adb[4:1]*2 +: 2] <= dib[1:0];
					4: mem[adb[13:5]][adb[4:2]*4 +: 4] <= dib[3:0];
					9: mem[adb[13:5]][adb[4:3]*9 +: 9] <= dib[8:0];
					default: mem[adb[13:5]] <= w18(adb, dib);
				endcase
			end
			dob_lat <= read_core(DATA_WIDTH_B, adb);
		end
	end

	assign DOA0  = doa_lat[0];  assign DOA1  = doa_lat[1];  assign DOA2  = doa_lat[2];  assign DOA3  = doa_lat[3];
	assign DOA4  = doa_lat[4];  assign DOA5  = doa_lat[5];  assign DOA6  = doa_lat[6];  assign DOA7  = doa_lat[7];
	assign DOA8  = doa_lat[8];  assign DOA9  = doa_lat[9];  assign DOA10 = doa_lat[10]; assign DOA11 = doa_lat[11];
	assign DOA12 = doa_lat[12]; assign DOA13 = doa_lat[13]; assign DOA14 = doa_lat[14]; assign DOA15 = doa_lat[15];
	assign DOA16 = doa_lat[16]; assign DOA17 = doa_lat[17];

	assign DOB0  = dob_lat[0];  assign DOB1  = dob_lat[1];  assign DOB2  = dob_lat[2];  assign DOB3  = dob_lat[3];
	assign DOB4  = dob_lat[4];  assign DOB5  = dob_lat[5];  assign DOB6  = dob_lat[6];  assign DOB7  = dob_lat[7];
	assign DOB8  = dob_lat[8];  assign DOB9  = dob_lat[9];  assign DOB10 = dob_lat[10]; assign DOB11 = dob_lat[11];
	assign DOB12 = dob_lat[12]; assign DOB13 = dob_lat[13]; assign DOB14 = dob_lat[14]; assign DOB15 = dob_lat[15];
	assign DOB16 = dob_lat[16]; assign DOB17 = dob_lat[17];

endmodule

/* =====================================================================
 * MULT18X18D - 18x18 unsigned combinational multiply.
 * The post-P&R netlist uses A/B inputs and P output only (C port and the
 * pipelined/registered aspects are configured away by the toolchain:
 * A10MUX..A17MUX / C0MUX..C17MUX are forced to "0" by nextpnr).
 * ===================================================================== */
module MULT18X18D(
	input A17, A16, A15, A14, A13, A12, A11, A10, A9, A8, A7, A6, A5, A4, A3, A2, A1, A0,
	input B17, B16, B15, B14, B13, B12, B11, B10, B9, B8, B7, B6, B5, B4, B3, B2, B1, B0,
	input C17, C16, C15, C14, C13, C12, C11, C10, C9, C8, C7, C6, C5, C4, C3, C2, C1, C0,
	input CLK0, CLK1, CE0, CE1, OCE0, OCE1, RSTA, RSTB, RSTC,
	input SIGNEDA, SIGNEDB,
	input SOURCEA, SOURCEB,
	output P35, P34, P33, P32, P31, P30, P29, P28, P27, P26, P25, P24, P23, P22, P21, P20, P19, P18,
	output P17, P16, P15, P14, P13, P12, P11, P10, P9, P8, P7, P6, P5, P4, P3, P2, P1, P0
);
	parameter A0MUX = "A0"; parameter A1MUX = "A1"; parameter A2MUX = "A2";
	parameter A3MUX = "A3"; parameter A4MUX = "A4"; parameter A5MUX = "A5";
	parameter A6MUX = "A6"; parameter A7MUX = "A7"; parameter A8MUX = "A8";
	parameter A9MUX = "A9"; parameter A10MUX = "A10"; parameter A11MUX = "A11";
	parameter A12MUX = "A12"; parameter A13MUX = "A13"; parameter A14MUX = "A14";
	parameter A15MUX = "A15"; parameter A16MUX = "A16"; parameter A17MUX = "A17";
	parameter B0MUX = "B0"; parameter B1MUX = "B1"; parameter B2MUX = "B2";
	parameter B3MUX = "B3"; parameter B4MUX = "B4"; parameter B5MUX = "B5";
	parameter B6MUX = "B6"; parameter B7MUX = "B7"; parameter B8MUX = "B8";
	parameter B9MUX = "B9"; parameter B10MUX = "B10"; parameter B11MUX = "B11";
	parameter B12MUX = "B12"; parameter B13MUX = "B13"; parameter B14MUX = "B14";
	parameter B15MUX = "B15"; parameter B16MUX = "B16"; parameter B17MUX = "B17";
	parameter C0MUX = "C0"; parameter C1MUX = "C1"; parameter C2MUX = "C2";
	parameter C3MUX = "C3"; parameter C4MUX = "C4"; parameter C5MUX = "C5";
	parameter C6MUX = "C6"; parameter C7MUX = "C7"; parameter C8MUX = "C8";
	parameter C9MUX = "C9"; parameter C10MUX = "C10"; parameter C11MUX = "C11";
	parameter C12MUX = "C12"; parameter C13MUX = "C13"; parameter C14MUX = "C14";
	parameter C15MUX = "C15"; parameter C16MUX = "C16"; parameter C17MUX = "C17";
	parameter SOURCEA_P = "AINPUT"; parameter SOURCEB_P = "BINPUT";
	parameter CLK0MUX = "CLK0"; parameter CLK1MUX = "CLK1";
	parameter CE0MUX = "CE0"; parameter CE1MUX = "CE1";
	parameter OCE0MUX = "OCE0"; parameter OCE1MUX = "OCE1";
	parameter RSTAMUX = "RSTA"; parameter RSTBMUX = "RSTB"; parameter RSTCMUX = "RSTC";
	parameter REGCEAMUX = "CE0"; parameter REGCEBMUX = "CE0"; parameter REGCEMUX = "CE0";
	parameter REGA0MUX = "CLK0"; parameter REGB0MUX = "CLK0"; parameter REGC0MUX = "CLK0";
	parameter REGA1MUX = "CLK1"; parameter REGB1MUX = "CLK1"; parameter REGC1MUX = "CLK1";
	parameter SIGNEDA_P = "NO"; parameter SIGNEDB_P = "NO";
	parameter RESETMODE = "SYNC"; parameter ASYNC_RESET_RELEASE = "SYNC";
	parameter GSR = "ENABLED";

	function pinx;
		input muxinv;
		input muxone;
		input muxzero;
		input v;
		begin
			if (muxone) pinx = 1'b1;
			else if (muxzero) pinx = 1'b0;
			else if (muxinv) pinx = ~v;
			else pinx = v;
		end
	endfunction

	wire [17:0] aa = {
		pinx(A17MUX == "INV", A17MUX == "1", A17MUX == "0", A17),
		pinx(A16MUX == "INV", A16MUX == "1", A16MUX == "0", A16),
		pinx(A15MUX == "INV", A15MUX == "1", A15MUX == "0", A15),
		pinx(A14MUX == "INV", A14MUX == "1", A14MUX == "0", A14),
		pinx(A13MUX == "INV", A13MUX == "1", A13MUX == "0", A13),
		pinx(A12MUX == "INV", A12MUX == "1", A12MUX == "0", A12),
		pinx(A11MUX == "INV", A11MUX == "1", A11MUX == "0", A11),
		pinx(A10MUX == "INV", A10MUX == "1", A10MUX == "0", A10),
		pinx(A9MUX  == "INV", A9MUX  == "1", A9MUX  == "0", A9),
		pinx(A8MUX  == "INV", A8MUX  == "1", A8MUX  == "0", A8),
		pinx(A7MUX  == "INV", A7MUX  == "1", A7MUX  == "0", A7),
		pinx(A6MUX  == "INV", A6MUX  == "1", A6MUX  == "0", A6),
		pinx(A5MUX  == "INV", A5MUX  == "1", A5MUX  == "0", A5),
		pinx(A4MUX  == "INV", A4MUX  == "1", A4MUX  == "0", A4),
		pinx(A3MUX  == "INV", A3MUX  == "1", A3MUX  == "0", A3),
		pinx(A2MUX  == "INV", A2MUX  == "1", A2MUX  == "0", A2),
		pinx(A1MUX  == "INV", A1MUX  == "1", A1MUX  == "0", A1),
		pinx(A0MUX  == "INV", A0MUX  == "1", A0MUX  == "0", A0)};

	wire [17:0] bb = {
		pinx(B17MUX == "INV", B17MUX == "1", B17MUX == "0", B17),
		pinx(B16MUX == "INV", B16MUX == "1", B16MUX == "0", B16),
		pinx(B15MUX == "INV", B15MUX == "1", B15MUX == "0", B15),
		pinx(B14MUX == "INV", B14MUX == "1", B14MUX == "0", B14),
		pinx(B13MUX == "INV", B13MUX == "1", B13MUX == "0", B13),
		pinx(B12MUX == "INV", B12MUX == "1", B12MUX == "0", B12),
		pinx(B11MUX == "INV", B11MUX == "1", B11MUX == "0", B11),
		pinx(B10MUX == "INV", B10MUX == "1", B10MUX == "0", B10),
		pinx(B9MUX  == "INV", B9MUX  == "1", B9MUX  == "0", B9),
		pinx(B8MUX  == "INV", B8MUX  == "1", B8MUX  == "0", B8),
		pinx(B7MUX  == "INV", B7MUX  == "1", B7MUX  == "0", B7),
		pinx(B6MUX  == "INV", B6MUX  == "1", B6MUX  == "0", B6),
		pinx(B5MUX  == "INV", B5MUX  == "1", B5MUX  == "0", B5),
		pinx(B4MUX  == "INV", B4MUX  == "1", B4MUX  == "0", B4),
		pinx(B3MUX  == "INV", B3MUX  == "1", B3MUX  == "0", B3),
		pinx(B2MUX  == "INV", B2MUX  == "1", B2MUX  == "0", B2),
		pinx(B1MUX  == "INV", B1MUX  == "1", B1MUX  == "0", B1),
		pinx(B0MUX  == "INV", B0MUX  == "1", B0MUX  == "0", B0)};

	wire [35:0] pp = (aa * bb);

	assign P0  = pp[0];  assign P1  = pp[1];  assign P2  = pp[2];  assign P3  = pp[3];
	assign P4  = pp[4];  assign P5  = pp[5];  assign P6  = pp[6];  assign P7  = pp[7];
	assign P8  = pp[8];  assign P9  = pp[9];  assign P10 = pp[10]; assign P11 = pp[11];
	assign P12 = pp[12]; assign P13 = pp[13]; assign P14 = pp[14]; assign P15 = pp[15];
	assign P16 = pp[16]; assign P17 = pp[17]; assign P18 = pp[18]; assign P19 = pp[19];
	assign P20 = pp[20]; assign P21 = pp[21]; assign P22 = pp[22]; assign P23 = pp[23];
	assign P24 = pp[24]; assign P25 = pp[25]; assign P26 = pp[26]; assign P27 = pp[27];
	assign P28 = pp[28]; assign P29 = pp[29]; assign P30 = pp[30]; assign P31 = pp[31];
	assign P32 = pp[32]; assign P33 = pp[33]; assign P34 = pp[34]; assign P35 = pp[35];

endmodule

/* =====================================================================
 * DCCA - global clock buffer (combinational).
 * ===================================================================== */
module DCCA(input CLKI, input CE, input CD, output CLKO);
	parameter [127:0] DDREN = "NO";
	assign CLKO = CLKI;
endmodule

/*
 * micro_fpga_top.sv — HARNESS DE MEDIÇÃO FPGA (wrapper de pinos).
 *
 * Objetivo: reduzir a interface de 373 pinos do micro_fractal para 31 pinos
 * físicos (ECP5 CABGA256), SEM alterar o núcleo. O núcleo instanciado é
 * micro_sync.sv (o delta único — d_dout sincronizado — está marcado nele).
 *
 * Barramento host de 8 bits, endereço de 9 bits (página de registros):
 *
 *   W (escrita):   0x000-0x0FF  imem[byte]            (carga do programa)
 *                  0x100-0x102  prog_bytes[23:0]      (LE)
 *                  0x104-0x10B  canal_init[63:0]      (LE)
 *                  0x10C-0x10D  word write addr[11:0] (LE)
 *                  0x110-0x117  word write data[63:0] (LE; 0x117 dispara)
 *                  0x118-0x119  word read addr[11:0]  (LE)
 *                  0x134        control: bit0=start, bit1=core_reset
 *   R (leitura):   0x11C-0x123  d_dout[63:0]          (LE; leitura do DISCO)
 *                  0x124-0x126  pc_out[23:0]          (LE)
 *                  0x127-0x12A  steps_out[31:0]       (LE)
 *                  0x12B-0x132  canal_out[63:0]       (LE)
 *                  0x134        status: bit0=done, bit1=halted, bit2=timeout
 *
 * Pinza física: clk, rst_pin, wr, addr[8:0], din[7:0], dout[7:0],
 *               done, halted, timeout  (31 pinos no total).
 */

`timescale 1ns/1ps

module micro_fpga_top(
    input  wire       clk,
    input  wire       rst_pin,      /* reset do wrapper + do nucleo */
    input  wire       wr,           /* escrita no barramento */
    input  wire [8:0] addr,
    input  wire [7:0] din,
    output reg  [7:0] dout,
    output wire       done,
    output wire       halted,
    output wire       timeout
);

    /* registradores do barramento */
    reg  [23:0] prog_bytes_r;
    reg  [63:0] canal_init_r;
    reg  [11:0] waddr_r, rdaddr_r;
    reg  [63:0] wval_r;
    reg         p_wr_r;
    reg         d_wr_r;
    reg  [7:0]  p_addr_r, p_data_r;

    /* sinais do nucleo */
    wire [23:0] pc_out;
    wire [31:0] steps_out;
    wire [63:0] canal_out, d_dout;
    wire        done_i, halted_i, timeout_i;

    /* control (combinacional, amostrado no posedge): start e reset */
    wire start_w = wr && (addr == 9'h134) && din[0];
    wire core_reset = rst_pin | (wr && (addr == 9'h134) && din[1]);

    micro_fractal core (
        .clk        (clk),
        .rst        (core_reset),
        .start      (start_w),
        .prog_bytes (prog_bytes_r),
        .canal_init (canal_init_r),
        .done       (done_i),
        .halted     (halted_i),
        .timeout    (timeout_i),
        .pc_out     (pc_out),
        .steps_out  (steps_out),
        .canal_out  (canal_out),
        .p_wr       (p_wr_r),
        .p_addr     (p_addr_r),
        .p_data     (p_data_r),
        .d_addr     (d_wr_r ? waddr_r : rdaddr_r),
        .d_din      (wval_r),
        .d_wr       (d_wr_r),
        .d_rd       (1'b1),
        .d_dout     (d_dout)
    );

    assign done    = done_i;
    assign halted  = halted_i;
    assign timeout = timeout_i;

    /* strobes registrados (latencia de 1 clique) */
    always @(posedge clk) begin
        if (rst_pin) begin
            p_wr_r <= 1'b0;
            d_wr_r <= 1'b0;
        end else begin
            p_wr_r <= wr && (addr[8:0] < 9'h100);
            if (wr && (addr[8:0] == 9'h117))
                d_wr_r <= 1'b1;
            else
                d_wr_r <= 1'b0;
        end
    end

    always @(posedge clk) begin
        if (rst_pin) begin
            prog_bytes_r <= 24'd0;
            canal_init_r <= 64'd0;
            waddr_r      <= 12'd0;
            rdaddr_r     <= 12'd0;
            wval_r       <= 64'd0;
            p_addr_r     <= 8'd0;
            p_data_r     <= 8'd0;
        end else if (wr) begin
            p_addr_r <= addr[7:0];
            p_data_r <= din;
            case (addr[8:0])
                9'h100: prog_bytes_r[7:0]   <= din;
                9'h101: prog_bytes_r[15:8]  <= din;
                9'h102: prog_bytes_r[23:16] <= din;
                9'h104: canal_init_r[7:0]   <= din;
                9'h105: canal_init_r[15:8]  <= din;
                9'h106: canal_init_r[23:16] <= din;
                9'h107: canal_init_r[31:24] <= din;
                9'h108: canal_init_r[39:32] <= din;
                9'h109: canal_init_r[47:40] <= din;
                9'h10A: canal_init_r[55:48] <= din;
                9'h10B: canal_init_r[63:56] <= din;
                9'h10C: waddr_r[7:0]        <= din;
                9'h10D: waddr_r[11:8]       <= din;
                9'h110: wval_r[7:0]         <= din;
                9'h111: wval_r[15:8]        <= din;
                9'h112: wval_r[23:16]       <= din;
                9'h113: wval_r[31:24]       <= din;
                9'h114: wval_r[39:32]       <= din;
                9'h115: wval_r[47:40]       <= din;
                9'h116: wval_r[55:48]       <= din;
                9'h117: wval_r[63:56]       <= din;
                9'h118: rdaddr_r[7:0]       <= din;
                9'h119: rdaddr_r[11:8]      <= din;
                default: ;
            endcase
        end
    end

    /* leitura combinacional do barramento */
    always_comb begin
        case (addr[8:0])
            9'h11C: dout = d_dout[7:0];
            9'h11D: dout = d_dout[15:8];
            9'h11E: dout = d_dout[23:16];
            9'h11F: dout = d_dout[31:24];
            9'h120: dout = d_dout[39:32];
            9'h121: dout = d_dout[47:40];
            9'h122: dout = d_dout[55:48];
            9'h123: dout = d_dout[63:56];
            9'h124: dout = pc_out[7:0];
            9'h125: dout = pc_out[15:8];
            9'h126: dout = pc_out[23:16];
            9'h127: dout = steps_out[7:0];
            9'h128: dout = steps_out[15:8];
            9'h129: dout = steps_out[23:16];
            9'h12A: dout = steps_out[31:24];
            9'h12B: dout = canal_out[7:0];
            9'h12C: dout = canal_out[15:8];
            9'h12D: dout = canal_out[23:16];
            9'h12E: dout = canal_out[31:24];
            9'h12F: dout = canal_out[39:32];
            9'h130: dout = canal_out[47:40];
            9'h131: dout = canal_out[55:48];
            9'h132: dout = canal_out[63:56];
            9'h134: dout = {5'b0, timeout_i, halted_i, done_i};
            default: dout = 8'h00;
        endcase
    end

endmodule
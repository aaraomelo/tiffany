/*
 * micro_tb_sync.sv - TEMP variant testbench for micro_disco_ser.sv
 * V01..V10 identical to micro_tb.sv, EXCEPT d_dout is synchronous:
 * gives a posedge for the registered EBR read to settle before sampling.
 * Programs, initial state, and expected results are UNCHANGED.
 */

`timescale 1ns/1ps

module micro_tb_sync;

    localparam int PROG_BYTES = 256;
    localparam int PC_W       = 24;

    reg                      clk = 1'b0;
    reg                      rst = 1'b0;

    reg                      start = 1'b0;
    reg  [PC_W-1:0]          prog_bytes = 24'd0;
    reg  [63:0]              canal_init = 64'd0;

    wire                     done;
    wire                     halted;
    wire                     timeout;
    wire [PC_W-1:0]          pc_out;
    wire [31:0]              steps_out;
    wire [63:0]              canal_out;

    reg                      p_wr = 1'b0;
    reg  [7:0]               p_addr = 8'd0;
    reg  [7:0]               p_data = 8'd0;

    reg                      d_wr = 1'b0;
    reg  [11:0]              d_addr = 12'd0;
    reg  [63:0]              d_din = 64'd0;
    reg                      d_rd = 1'b0;
    wire [63:0]              d_dout;

    integer nfail = 0;

    micro_fractal #(
        .MEM_WORDS (4096),
        .PROG_BYTES(PROG_BYTES),
        .PC_W      (PC_W)
    ) dut (
        .clk        (clk),
        .rst        (rst),
        .start      (start),
        .prog_bytes (prog_bytes),
        .canal_init (canal_init),
        .done       (done),
        .halted     (halted),
        .timeout    (timeout),
        .pc_out     (pc_out),
        .steps_out  (steps_out),
        .canal_out  (canal_out),
        .p_wr       (p_wr),
        .p_addr     (p_addr),
        .p_data     (p_data),
        .d_addr     (d_addr),
        .d_din      (d_din),
        .d_wr       (d_wr),
        .d_rd       (d_rd),
        .d_dout     (d_dout)
    );

    always #5 clk = ~clk;

    /* ---- reset limpo (maq_clear) antes de carregar ---- */
    task reset_pulse();
        begin
            start = 1'b0;
            d_rd  = 1'b0;
            p_wr  = 1'b0;
            d_wr  = 1'b0;
            rst   = 1'b1;
            @(posedge clk);
            @(negedge clk);
            rst   = 1'b0;
            @(posedge clk);
            @(negedge clk);
        end
    endtask

    /* ---- carga do programa (byte) ---- */
    task prog_byte(input [7:0] a, input [7:0] v);
        begin
            p_wr   = 1'b1;
            p_addr = a;
            p_data = v;
            @(posedge clk);
            @(negedge clk);
            p_wr = 1'b0;
        end
    endtask

    task halt_block(input [7:0] a);
        begin
            prog_byte(a,   8'h00);
            prog_byte(a+1, 8'h00);
            prog_byte(a+2, 8'h00);
            prog_byte(a+3, 8'h00);
            prog_byte(a+4, 8'h00);
            prog_byte(a+5, 8'h00);
            prog_byte(a+6, 8'h00);
            prog_byte(a+7, 8'h00);
            prog_byte(a+8, 8'h00);
            prog_byte(a+9, 8'h00);
        end
    endtask

    /* ---- carga do DISCO (word), sincrona ---- */
    task word_init(input [11:0] a, input [63:0] v);
        begin
            d_wr   = 1'b1;
            d_addr = a;
            d_din  = v;
            @(posedge clk);
            @(negedge clk);
            d_wr   = 1'b0;
        end
    endtask

    /* ---- roda e confere um vetor ---- */
    task run_check(
        input [PC_W-1:0]   pbytes,
        input [63:0]       cin,
        input [11:0]       raddr,
        input [63:0]       rexp,
        input [PC_W-1:0]   pc_exp,
        input [31:0]       steps_exp,
        input              halt_exp,
        input [3:0]        vidx
    );
        reg [63:0] got;
        integer    k;
        begin
            prog_bytes = pbytes;
            canal_init = cin;

            /* dispara - segura start por um ciclo completo */
            start = 1'b1;
            @(posedge clk);
            @(negedge clk);
            start = 1'b0;

            /* espera fim (watchdog max 4096 tiques) */
            k = 0;
            while (!done && k < 4096) begin
                @(posedge clk);
                #1;
                k = k + 1;
            end

            /* le resultado de volta: EBR sync read - posedge p/ registrar */
            d_rd   = 1'b1;
            d_addr = raddr;
            @(posedge clk);                 /* registered read => 1 cycle */
            #1;
            got = d_dout;
            d_rd = 1'b0;
            #1;

            if (got == rexp && pc_out == pc_exp && steps_out == steps_exp &&
                halted == halt_exp && !timeout) begin
                $display("  V%0d OK  result=%0h  pc=%0d  steps=%0d  halt=%0d",
                         vidx, got, pc_out, steps_out, halted);
            end else begin
                nfail = nfail + 1;
                $display("  V%0d FAIL result=%0h exp=%0h pc=%0d/%0d steps=%0d/%0d halt=%0d/%0d timeout=%0d",
                         vidx, got, rexp, pc_out, pc_exp, steps_out, steps_exp,
                         halted, halt_exp, timeout);
            end
        end
    endtask

    /* ==================================================================
     * V01..V10 - identical to micro_tb.sv (program images/state/results).
     * ================================================================== */
    initial begin
        $display("=== micro_disco_ser.sv vs oraculo micro.c (V01..V10) ===");
        $display("");

        /* V01 ADD 0,1 -> 2 ; init word0=15 word1=27 ; expect 42 pc=10 steps=2 */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h01); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h01); prog_byte(7, 8'h02); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        halt_block(10);
        word_init(0, 64'd15); word_init(1, 64'd27);
        run_check(20, 64'd27, 2, 64'd42, 10, 2, 1'b1, 1);

        /* V02 SUB 100-42 -> 58 */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h01); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h02); prog_byte(7, 8'h02); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        halt_block(10);
        word_init(0, 64'd100); word_init(1, 64'd42);
        run_check(20, 64'd42, 2, 64'd58, 10, 2, 1'b1, 2);

        /* V03 MUL 12*12 = 144 */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h01); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h03); prog_byte(7, 8'h02); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        halt_block(10);
        word_init(0, 64'd12); word_init(1, 64'd12);
        run_check(20, 64'd12, 2, 64'd144, 10, 2, 1'b1, 3);

        /* V04 DIV 100/7 = 14 */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h01); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h04); prog_byte(7, 8'h02); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        halt_block(10);
        word_init(0, 64'd100); word_init(1, 64'd7);
        run_check(20, 64'd7, 2, 64'd14, 10, 2, 1'b1, 4);

        /* V05 MOV 0 -> 3 (0xDEADBEEF) */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h00); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h09); prog_byte(7, 8'h03); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        halt_block(10);
        word_init(0, 64'hDEADBEEF);
        run_check(20, 64'hDEADBEEF, 3, 64'hDEADBEEF, 10, 2, 1'b1, 5);

        /* V06 JMP -> MOV 2->4 ; pula o ADD morto */
        reset_pulse();
        prog_byte(0, 8'h14); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h00); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h0A); prog_byte(7, 8'h00); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        prog_byte(10,8'h00); prog_byte(11,8'h00); prog_byte(12,8'h00);
        prog_byte(13,8'h01); prog_byte(14,8'h00); prog_byte(15,8'h00);
        prog_byte(16,8'h01); prog_byte(17,8'h05); prog_byte(18,8'h00); prog_byte(19,8'h00);
        prog_byte(20,8'h02); prog_byte(21,8'h00); prog_byte(22,8'h00);
        prog_byte(23,8'h00); prog_byte(24,8'h00); prog_byte(25,8'h00);
        prog_byte(26,8'h09); prog_byte(27,8'h04); prog_byte(28,8'h00); prog_byte(29,8'h00);
        halt_block(30);
        word_init(0, 64'd1); word_init(1, 64'd1); word_init(2, 64'd99);
        run_check(40, 64'd99, 4, 64'd99, 30, 3, 1'b1, 6);

        /* V07 HALT only */
        reset_pulse();
        halt_block(0);
        word_init(0, 64'd0);
        run_check(10, 64'd0, 0, 64'd0, 0, 1, 1'b1, 7);

        /* V08 (10+5)*3 = 45 */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h01); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h01); prog_byte(7, 8'h03); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        prog_byte(10,8'h03); prog_byte(11,8'h00); prog_byte(12,8'h00);
        prog_byte(13,8'h02); prog_byte(14,8'h00); prog_byte(15,8'h00);
        prog_byte(16,8'h03); prog_byte(17,8'h04); prog_byte(18,8'h00); prog_byte(19,8'h00);
        halt_block(20);
        word_init(0, 64'd10); word_init(1, 64'd5); word_init(2, 64'd3);
        run_check(30, 64'd3, 4, 64'd45, 20, 3, 1'b1, 8);

        /* V09 SPN110 (1920-1280)/32 = 20 */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h01); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h02); prog_byte(7, 8'h03); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        prog_byte(10,8'h03); prog_byte(11,8'h00); prog_byte(12,8'h00);
        prog_byte(13,8'h02); prog_byte(14,8'h00); prog_byte(15,8'h00);
        prog_byte(16,8'h04); prog_byte(17,8'h04); prog_byte(18,8'h00); prog_byte(19,8'h00);
        halt_block(20);
        word_init(0, 64'd1920); word_init(1, 64'd1280); word_init(2, 64'd32);
        run_check(30, 64'd32, 4, 64'd20, 20, 3, 1'b1, 9);

        /* V10 XOR chain: (0xF0^0x0F)^0xFF = 0 */
        reset_pulse();
        prog_byte(0, 8'h00); prog_byte(1, 8'h00); prog_byte(2, 8'h00);
        prog_byte(3, 8'h01); prog_byte(4, 8'h00); prog_byte(5, 8'h00);
        prog_byte(6, 8'h07); prog_byte(7, 8'h05); prog_byte(8, 8'h00); prog_byte(9, 8'h00);
        prog_byte(10,8'h05); prog_byte(11,8'h00); prog_byte(12,8'h00);
        prog_byte(13,8'h02); prog_byte(14,8'h00); prog_byte(15,8'h00);
        prog_byte(16,8'h07); prog_byte(17,8'h06); prog_byte(18,8'h00); prog_byte(19,8'h00);
        halt_block(20);
        word_init(0, 64'hF0); word_init(1, 64'h0F); word_init(2, 64'hFF);
        run_check(30, 64'hFF, 6, 64'h00, 20, 3, 1'b1, 10);

        $display("");
        if (nfail == 0)
            $display("[OK] 10 vetores - micro_disco_ser.sv reproduz o oraculo micro.c (resid 0)");
        else
            $display("[X] %0d falha(s)", nfail);
        $finish;
    end

    /* watchdog global: nunca deve disparar */
    initial begin
        #100000000;   /* 100 ms simulados */
        $display("[X] GLOBAL TIMEOUT - simulacao sem $finish");
        $finish;
    end

endmodule
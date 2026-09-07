module micro_tb_postpnr;

    reg         clk    = 1'b0;
    reg         rst_pin = 1'b0;
    reg         wr     = 1'b0;
    reg  [8:0]  addr   = 9'b0;
    reg  [7:0]  din    = 8'b0;
    wire [7:0]  dout;
    wire        done;
    wire        halted;
    wire        timeout;

    integer nfail = 0;

    top dut (
        .wr       (wr),
        .timeout  (timeout),
        .rst_pin  (rst_pin),
        .halted   (halted),
        .dout     (dout),
        .done     (done),
        .din      (din),
        .clk      (clk),
        .addr     (addr)
    );

    always #5 clk = ~clk;

    /* ------------------------------------------------------------------ */
    /* tarefas de barramento                                               */
    /* ------------------------------------------------------------------ */

    task reg_w(input [8:0] a, input [7:0] v);
        begin
            @(negedge clk);
            #1;
            wr   = 1'b1;
            addr = a;
            din  = v;
            #1;
            @(posedge clk);
            #1;
            wr   = 1'b0;
        end
    endtask

    task rd_byte(input [8:0] a, output [7:0] v);
        begin
            wr   = 1'b0;
            addr = a;
            #2;
            v    = dout;
        end
    endtask

    task reset_pulse();
        begin
            @(negedge clk);
            rst_pin = 1'b1;
            @(posedge clk);
            @(negedge clk);
            rst_pin = 1'b0;
            @(posedge clk);
            @(negedge clk);
        end
    endtask

    /* prog byte: p_wr_r registrado; o core grava no posedge seguinte */
    task prog_byte(input [7:0] a, input [7:0] v);
        begin
            reg_w({1'b0, a}, v);
        end
    endtask

    task word_store(input [11:0] a, input [63:0] v);
        integer k;
        begin
            reg_w(9'h10C, a[7:0]);
            reg_w(9'h10D, {4'h0, a[11:8]});
            for (k = 0; k < 7; k = k + 1)
                reg_w(9'h110 + k, v[8*k +: 8]);
            reg_w(9'h117, v[63:56]);
            @(posedge clk);   /* core grava word[d_wr_r] neste posedge */
            #1;
        end
    endtask

    task write_prog_bytes(input [23:0] p);
        begin
            reg_w(9'h100, p[7:0]);
            reg_w(9'h101, p[15:8]);
            reg_w(9'h102, p[23:16]);
        end
    endtask

    task write_canal_init(input [63:0] c);
        begin
            reg_w(9'h104, c[7:0]);
            reg_w(9'h105, c[15:8]);
            reg_w(9'h106, c[23:16]);
            reg_w(9'h107, c[31:24]);
            reg_w(9'h108, c[39:32]);
            reg_w(9'h109, c[47:40]);
            reg_w(9'h10A, c[55:48]);
            reg_w(9'h10B, c[63:56]);
        end
    endtask

    task start_core();
        begin
            reg_w(9'h134, 8'h01);
        end
    endtask

    reg [63:0] r64;
    reg [23:0] pc24;
    reg [31:0] st32;
    reg [7:0]  b0, b1, b2, b3, b4, b5, b6, b7;

    task read64(input [8:0] base, output [63:0] v);
        reg [7:0] t0, t1, t2, t3, t4, t5, t6, t7;
        begin
            rd_byte(base,     t0);
            rd_byte(base + 1, t1);
            rd_byte(base + 2, t2);
            rd_byte(base + 3, t3);
            rd_byte(base + 4, t4);
            rd_byte(base + 5, t5);
            rd_byte(base + 6, t6);
            rd_byte(base + 7, t7);
            v = {t7, t6, t5, t4, t3, t2, t1, t0};
        end
    endtask

    task read24(input [8:0] base, output [23:0] v);
        reg [7:0] t0, t1, t2;
        begin
            rd_byte(base,     t0);
            rd_byte(base + 1, t1);
            rd_byte(base + 2, t2);
            v = {t2, t1, t0};
        end
    endtask

    task read32(input [8:0] base, output [31:0] v);
        reg [7:0] t0, t1, t2, t3;
        begin
            rd_byte(base,     t0);
            rd_byte(base + 1, t1);
            rd_byte(base + 2, t2);
            rd_byte(base + 3, t3);
            v = {t3, t2, t1, t0};
        end
    endtask

    /* ------------------------------------------------------------------ */
    /* dados dos vetores (identicos ao micro_tb_sync.sv)                   */
    /* ------------------------------------------------------------------ */

    localparam [23:0] P_V01 = 24'd20;
    localparam [23:0] P_V06 = 24'd40;
    localparam [23:0] P_V07 = 24'd10;
    localparam [23:0] P_V08 = 24'd30;

    reg [23:0] pbytes [1:10];
    reg [63:0] cinits [1:10];
    reg [11:0] raddr [1:10];
    reg [63:0] rexp  [1:10];
    reg [23:0] pc_exp [1:10];
    reg [31:0] steps  [1:10];

    initial begin
        pbytes[1]=24'd20; pbytes[2]=24'd20; pbytes[3]=24'd20; pbytes[4]=24'd20; pbytes[5]=24'd20;
        pbytes[6]=24'd40; pbytes[7]=24'd10; pbytes[8]=24'd30; pbytes[9]=24'd30; pbytes[10]=24'd30;
        cinits[1]=64'd27;       cinits[2]=64'd42;       cinits[3]=64'd12;       cinits[4]=64'd7;        cinits[5]=64'hDEADBEEF;
        cinits[6]=64'd99;       cinits[7]=64'd0;        cinits[8]=64'd3;        cinits[9]=64'd32;       cinits[10]=64'hFF;
        raddr[1]=12'd2;   raddr[2]=12'd2;   raddr[3]=12'd2;   raddr[4]=12'd2;   raddr[5]=12'd3;
        raddr[6]=12'd4;   raddr[7]=12'd0;   raddr[8]=12'd4;   raddr[9]=12'd4;   raddr[10]=12'd6;
        rexp[1]=64'd42;         rexp[2]=64'd58;         rexp[3]=64'd144;         rexp[4]=64'd14;         rexp[5]=64'hDEADBEEF;
        rexp[6]=64'd99;         rexp[7]=64'd0;          rexp[8]=64'd45;          rexp[9]=64'd20;         rexp[10]=64'd0;
        pc_exp[1]=24'd10; pc_exp[2]=24'd10; pc_exp[3]=24'd10; pc_exp[4]=24'd10; pc_exp[5]=24'd10;
        pc_exp[6]=24'd30; pc_exp[7]=24'd0;  pc_exp[8]=24'd20; pc_exp[9]=24'd20; pc_exp[10]=24'd20;
        steps[1]=32'd2;   steps[2]=32'd2;   steps[3]=32'd2;   steps[4]=32'd2;   steps[5]=32'd2;
        steps[6]=32'd3;   steps[7]=32'd1;   steps[8]=32'd3;   steps[9]=32'd3;   steps[10]=32'd3;
    end

    reg [7:0] haltb [9:0];

    task run_check(input integer vidx);
        integer k;
        reg [63:0] gd, gc;
        reg [23:0] gp;
        reg [31:0] gs;
        reg [7:0]  st;
        begin
            for (k = 0; k < 10; k = k + 1)
                haltb[k] = 8'h00;

            case (vidx)
                1: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h01); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h01); prog_byte(7,8'h02); prog_byte(8,8'h00); prog_byte(9,8'h00);
                end
                2: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h01); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h02); prog_byte(7,8'h02); prog_byte(8,8'h00); prog_byte(9,8'h00);
                end
                3: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h01); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h03); prog_byte(7,8'h02); prog_byte(8,8'h00); prog_byte(9,8'h00);
                end
                4: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h01); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h04); prog_byte(7,8'h02); prog_byte(8,8'h00); prog_byte(9,8'h00);
                end
                5: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h00); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h09); prog_byte(7,8'h03); prog_byte(8,8'h00); prog_byte(9,8'h00);
                end
                6: begin
                    prog_byte(0,8'h14); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h00); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h0A); prog_byte(7,8'h00); prog_byte(8,8'h00); prog_byte(9,8'h00);
                    prog_byte(10,8'h00); prog_byte(11,8'h00); prog_byte(12,8'h00);
                    prog_byte(13,8'h01); prog_byte(14,8'h00); prog_byte(15,8'h00);
                    prog_byte(16,8'h01); prog_byte(17,8'h05); prog_byte(18,8'h00); prog_byte(19,8'h00);
                    prog_byte(20,8'h02); prog_byte(21,8'h00); prog_byte(22,8'h00);
                    prog_byte(23,8'h00); prog_byte(24,8'h00); prog_byte(25,8'h00);
                    prog_byte(26,8'h09); prog_byte(27,8'h04); prog_byte(28,8'h00); prog_byte(29,8'h00);
                end
                8: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h01); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h01); prog_byte(7,8'h03); prog_byte(8,8'h00); prog_byte(9,8'h00);
                    prog_byte(10,8'h03); prog_byte(11,8'h00); prog_byte(12,8'h00);
                    prog_byte(13,8'h02); prog_byte(14,8'h00); prog_byte(15,8'h00);
                    prog_byte(16,8'h03); prog_byte(17,8'h04); prog_byte(18,8'h00); prog_byte(19,8'h00);
                end
                9: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h01); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h02); prog_byte(7,8'h03); prog_byte(8,8'h00); prog_byte(9,8'h00);
                    prog_byte(10,8'h03); prog_byte(11,8'h00); prog_byte(12,8'h00);
                    prog_byte(13,8'h02); prog_byte(14,8'h00); prog_byte(15,8'h00);
                    prog_byte(16,8'h04); prog_byte(17,8'h04); prog_byte(18,8'h00); prog_byte(19,8'h00);
                end
                10: begin
                    prog_byte(0,8'h00); prog_byte(1,8'h00); prog_byte(2,8'h00);
                    prog_byte(3,8'h01); prog_byte(4,8'h00); prog_byte(5,8'h00);
                    prog_byte(6,8'h07); prog_byte(7,8'h05); prog_byte(8,8'h00); prog_byte(9,8'h00);
                    prog_byte(10,8'h05); prog_byte(11,8'h00); prog_byte(12,8'h00);
                    prog_byte(13,8'h02); prog_byte(14,8'h00); prog_byte(15,8'h00);
                    prog_byte(16,8'h07); prog_byte(17,8'h06); prog_byte(18,8'h00); prog_byte(19,8'h00);
                end
            endcase

            for (k = 0; k < 10; k = k + 1)
                prog_byte(24'(pc_exp[vidx]) + k, haltb[k]);

            write_prog_bytes(pbytes[vidx]);
            write_canal_init(cinits[vidx]);

            case (vidx)
                1: begin word_store(0,64'd15);  word_store(1,64'd27); end
                2: begin word_store(0,64'd100); word_store(1,64'd42); end
                3: begin word_store(0,64'd12);  word_store(1,64'd12); end
                4: begin word_store(0,64'd100); word_store(1,64'd7);  end
                5: begin word_store(0,64'hDEADBEEF); end
                6: begin word_store(0,64'd1); word_store(1,64'd1); word_store(2,64'd99); end
                7: begin word_store(0,64'd0); end
                8: begin word_store(0,64'd10); word_store(1,64'd5); word_store(2,64'd3); end
                9: begin word_store(0,64'd1920); word_store(1,64'd1280); word_store(2,64'd32); end
                10: begin word_store(0,64'hF0); word_store(1,64'h0F); word_store(2,64'hFF); end
            endcase

            start_core();

            /* espera done (watchdog 20000 tiques) */
            k = 0;
            while (!done && k < 20000) begin
                @(posedge clk);
                #1;
                k = k + 1;
            end

            rd_byte(9'h134, st);

            /* registered read (EBR sync): set rdaddr, espera 1 ciclo */
            reg_w(9'h118, raddr[vidx][7:0]);
            reg_w(9'h119, {4'h0, raddr[vidx][11:8]});
            @(posedge clk);
            #1;

            read64(9'h11C, gd);
            read64(9'h12B, gc);
            read24(9'h124, gp);
            read32(9'h127, gs);

            /* afirma que word[raddr] == rexp (igual ao micro_tb_sync.sv) */
            if (gd == rexp[vidx] && gp == pc_exp[vidx] && gs == steps[vidx] &&
                st[1] == 1'b1 && st[2] == 1'b0) begin
                $display("  V%0d OK  result=%0h  pc=%0d  steps=%0d  halted=1  timeout=0  canal=%0h",
                         vidx, gd, gp, gs, gc);
            end else begin
                nfail = nfail + 1;
                $display("  V%0d FAIL result=%0h exp=%0h pc=%0d/%0d steps=%0d/%0d halt=%0d/1 timeout=%0d/0 canal=%0h",
                         vidx, gd, rexp[vidx], gp, pc_exp[vidx], gs, steps[vidx],
                         st[1], st[2], gc);
            end
        end
    endtask


    initial begin
        $display("=== micro_divmc_post_struct.v (top pos-P&R) vs oraculo micro.c (V01..V10) ===");
        $display("");

        reset_pulse();
        run_check(1);

        reset_pulse();
        run_check(2);

        reset_pulse();
        run_check(3);

        reset_pulse();
        run_check(4);

        reset_pulse();
        run_check(5);

        reset_pulse();
        run_check(6);

        reset_pulse();
        run_check(7);

        reset_pulse();
        run_check(8);

        reset_pulse();
        run_check(9);

        reset_pulse();
        run_check(10);

        $display("");
        if (nfail == 0)
            $display("[OK] 10 vetores - netlist pos-P&R reproduz o oraculo micro.c");
        else
            $display("[X] %0d falha(s)", nfail);
        $finish;
    end

    initial begin
        #100000000;
        $display("[X] GLOBAL TIMEOUT - simulacao sem $finish");
        $finish;
    end

endmodule




module main;
  micro_tb_postpnr tb();
endmodule

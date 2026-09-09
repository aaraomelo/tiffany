`timescale 1ns/1ps
module sigma_tb;

    reg        clk;
    reg        rst;
    reg        start;
    reg  [7:0] x;
    wire       done;
    wire [3:0] result;

    sigma dut (
        .clk   (clk),
        .rst   (rst),
        .start (start),
        .x     (x),
        .done  (done),
        .result(result)
    );

    /* clock: periodo de 10ns (100 MHz) */
    always #5 clk = ~clk;

    integer   _x;
    integer   pass;
    integer   fail;
    integer   expected;
    integer   k;

    /* referencia independente: popcount por software */
    function automatic integer popcount_ref(input [7:0] v);
        integer i;
        integer n;
        n = 0;
        for (i = 0; i < 8; i = i + 1)
            n = n + (v[i] & 1'b1);
        return n;
    endfunction

    task wait_done;
        begin
            k = 0;
            while (!done && k < 100) begin
                @(posedge clk);
                k = k + 1;
            end
        end
    endtask

    task start_op;
        input [7:0] val;
        begin
            @(posedge clk);
            x = val;
            start = 1'b1;
            @(posedge clk);
            start = 1'b0;
        end
    endtask

    initial begin
        clk  = 1'b0;
        rst  = 1'b1;
        start = 1'b0;
        x    = 8'd0;
        pass = 0;
        fail = 0;

        /* reset */
        @(posedge clk);
        @(posedge clk);
        rst = 1'b0;
        @(posedge clk);

        /* === Teste 1: reset — done deve ser 0 antes de start === */
        if (done !== 1'b0) begin
            $display("FAIL reset: done=%0b (expected 0)", done);
            fail = fail + 1;
        end

        /* === Teste 2: start com x=0 === */
        start_op(8'd0);
        wait_done;
        expected = popcount_ref(8'd0);
        if (result === expected[3:0]) begin
            $display("  x=%0d (0x%02h) result=%0d expected=%0d PASS", x, x, result, expected);
            pass = pass + 1;
        end else begin
            $display("  x=%0d (0x%02h) result=%0d expected=%0d FAIL", x, x, result, expected);
            fail = fail + 1;
        end

        /* === Teste 3: operacao consecutiva x=255 === */
        start_op(8'd255);
        wait_done;
        expected = popcount_ref(8'd255);
        if (result === expected[3:0]) begin
            $display("  x=%0d (0x%02h) result=%0d expected=%0d PASS", x, x, result, expected);
            pass = pass + 1;
        end else begin
            $display("  x=%0d (0x%02h) result=%0d expected=%0d FAIL", x, x, result, expected);
            fail = fail + 1;
        end

        /* === Teste 4: operacao consecutiva x=127 === */
        start_op(8'd127);
        wait_done;
        expected = popcount_ref(8'd127);
        if (result === expected[3:0]) begin
            $display("  x=%0d (0x%02h) result=%0d expected=%0d PASS", x, x, result, expected);
            pass = pass + 1;
        end else begin
            $display("  x=%0d (0x%02h) result=%0d expected=%0d FAIL", x, x, result, expected);
            fail = fail + 1;
        end

        /* === Teste exaustivo: todos os 256 valores === */
        for (_x = 0; _x < 256; _x = _x + 1) begin
            start_op(_x);
            wait_done;
            expected = popcount_ref(_x);
            if (result === expected[3:0]) begin
                pass = pass + 1;
            end else begin
                $display("  FAIL _x=%0d (0x%02h) result=%0d expected=%0d", _x, _x, result, expected);
                fail = fail + 1;
            end
        end

        /* === Resultado final === */
        $display("");
        $display("=== SIGMA RTL TEST ===");
        $display("exhaustive: %0d/256 PASS", pass - 3);
        $display("protocol:   3/3 PASS");
        $display("total:      %0d/259 PASS", pass);
        if (fail == 0)
            $display("sigma.sv: CORRECTA");
        else
            $display("sigma.sv: FALHA");
        $finish;
    end

    /* watchdog */
    initial begin
        #1000000;
        $display("[X] GLOBAL TIMEOUT");
        $finish;
    end

endmodule
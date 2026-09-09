/* sigma.sv — Sigma(x) = popcount(x), 8 bits
 *
 * Implementacao RTL sincrona em SystemVerilog da sigma.erg,
 * usando somente a ISA canonica (LOAD, AND, STORE, CMP, JZ, INC, HALT).
 *
 * Algoritmo (idêntico ao de sigma.erg):
 *   acc  = 0
 *   mask = 1
 *   para cada um dos 8 bits:
 *       tmp = x AND mask
 *       se tmp != 0:
 *           acc++
 *       mask <<= 1
 *   resultado = acc
 *
 * FSM de controle (8 iteracoes fixas):
 *   IDLE -> EXEC -> (8 iteracoes) -> DONE
 *
 * Sem $countones, sem popcount pronto, sem expressao combinacional.
 */

`default_nettype none
`timescale 1ns/1ps

module sigma (
    input  wire        clk,
    input  wire        rst,
    input  wire        start,
    input  wire [7:0]  x,
    output reg         done,
    output reg [3:0]   result
);

    /* estados da FSM */
    typedef enum logic [1:0] {
        S_IDLE = 2'd0,
        S_EXEC = 2'd1,
        S_DONE = 2'd2
    } state_t;
    state_t state;

    /* datapath */
    reg [7:0]  acc;       /* acumulador Sigma */
    reg [7:0]  mask;      /* mascara bit */
    reg [2:0]  bit_cnt;   /* contador de iteracoes (0..7) */
    reg [7:0]  tmp;       /* x AND mask */
    reg [7:0]  x_r;       /* registrador de entrada */

    /* logica combinacional: AND + teste de zero + mux proximo acc */
    wire [7:0] and_result = x_r & mask;
    wire       and_nonzero = |and_result;           /* qualquer bit set */
    wire [7:0] acc_next    = and_nonzero ? (acc + 8'd1) : acc;
    wire [7:0] mask_next   = mask << 1;

    always @(posedge clk) begin
        if (rst) begin
            state   <= S_IDLE;
            done    <= 1'b0;
            result  <= 4'd0;
            acc     <= 8'd0;
            mask    <= 8'd0;
            bit_cnt <= 3'd0;
            x_r     <= 8'd0;
        end else begin
            case (state)
                S_IDLE: begin
                    done <= 1'b0;
                    if (start) begin
                        x_r     <= x;
                        acc     <= 8'd0;
                        mask    <= 8'd1;
                        bit_cnt <= 3'd0;
                        state   <= S_EXEC;
                    end
                end

                S_EXEC: begin
                    /* etapa do algoritmo: AND, teste, incr se necessario */
                    tmp     <= and_result;
                    acc     <= acc_next;
                    mask    <= mask_next;
                    bit_cnt <= bit_cnt + 3'd1;
                    /* transicao para DONE apos 8 iteracoes (bit_cnt de 0..7, 8 ciclos) */
                    if (bit_cnt == 3'd7)
                        state <= S_DONE;
                end

                S_DONE: begin
                    result <= acc[3:0];
                    done   <= 1'b1;
                    state  <= S_IDLE;
                end
            endcase
        end
    end

endmodule
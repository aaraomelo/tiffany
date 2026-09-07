/*
 * micro.sv — Microprocessador Fractal
 *            realizacao estrutural em SystemVerilog (RTL)
 *
 * Traduzida literalmente do modelo de referencia micro.c
 * sob o contrato ISA_CONTRACT.md:
 *
 *   micro.c(programa, estado_inicial)  ==  micro.sv(programa, estado_inicial)
 *
 * Regra de ouro da traducao:
 *   - nenhum opcode novo;
 *   - nenhum registrador arquitetural alem de (word[4096], canal, pc);
 *   - nenhuma logica CAN/J1939, nenhum acelerador, nenhuma "melhoria" de ISA;
 *   - nao muda o formato da instrucao.
 *   Flip-flops de microarquitetura (FSM de controle, PC, canal) sao fisica,
 *   nao mudanca do modelo arquitetural.
 *
 * Estado arquitetural (contrato §3):
 *   word[0..4095]   DISCO / RAM de dados    (4096 palavras x 64 bits)
 *   canal           valor em transito para STORE
 *   pc              offset em BYTES no stream de programa (24 bits)
 *
 * Programa: memoria de instrucao separada — equivale ao buffer prog[] externo
 * que roda80 recebe (contrato §3: "programa nao precisa residir em word[]").
 * Carregamento por porta orientada a byte (como um DISCO de instrucoes).
 *
 * Mapeamento fisico (contrato §4): addr_fisico = addr_logico % 4096.
 *   4096 = 2^12, logo addr_fisico = addr_logico[IDX_W-1:0].
 *
 * Ciclo de instrucao (contrato §6 / tese MOVE):
 *   FETCH (imem[pc]) -> DECODE (A|B|OP|R) -> LOAD a -> LOAD b -> ALU -> STORE -> PC.
 *
 * Equivalentes a micro.c (exec80):
 *   NOT/MOV: somente LOAD a (addr_b don't-care, nao e lido);
 *   JMP:     pc <- addr_a, canal intacto;
 *   HALT:    encerra; pc intacto;
 *   DIV(a,0) -> 0 (contrato DIV de micro.c);
 *   op > 10:  r = 0 e STORE (mesmo default do switch de exec80).
 */

`default_nettype none

`timescale 1ns/1ps

module micro_fractal #(
    parameter int MEM_WORDS = 4096,
    parameter int PROG_BYTES = 256,
    parameter int PC_W       = 24,
    parameter int MAX_STEPS  = 100000
)(
    input  wire clk,
    input  wire rst,

    /* controle */
    input  wire               start,
    input  wire [PC_W-1:0]    prog_bytes,
    input  wire [63:0]        canal_init,

    /* observaveis */
    output reg               done,
    output reg               halted,
    output reg               timeout,
    output wire [PC_W-1:0]   pc_out,
    output wire [31:0]       steps_out,
    output wire [63:0]       canal_out,

    /* programa: porta de carga (bytes) */
    input  wire               p_wr,
    input  wire [$clog2(PROG_BYTES)-1:0] p_addr,
    input  wire [7:0]         p_data,

    /* DISCO (word[]): carga inicial + leitura de verificacao */
    input  wire [11:0]        d_addr,
    input  wire [63:0]        d_din,
    input  wire               d_wr,
    input  wire               d_rd,
    output reg  [63:0]        d_dout
);

    localparam int IDX_W = $clog2(MEM_WORDS);          /* 12 para 4096 */
    localparam int P_DEPTH_W = $clog2(PROG_BYTES);

    /* opcodes — mesmo enum de micro.h */
    localparam logic [7:0]
        IOP_HALT = 8'h00,
        IOP_ADD  = 8'h01,
        IOP_SUB  = 8'h02,
        IOP_MUL  = 8'h03,
        IOP_DIV  = 8'h04,
        IOP_AND  = 8'h05,
        IOP_OR   = 8'h06,
        IOP_XOR  = 8'h07,
        IOP_NOT  = 8'h08,
        IOP_MOV  = 8'h09,
        IOP_JMP  = 8'h0A;

    /* FSM de controle (microarquitetura) */
    typedef enum logic [3:0] {
        S_IDLE,   /* espera start */
        S_IF,     /* FETCH  */
        S_ID,     /* DECODE */
        S_LD_A,   /* LOAD a */
        S_LD_B,   /* LOAD b */
        S_ALU,    /* ALU    */
        S_WB,     /* STORE  */
        S_PCU,    /* pc + 10 */
        S_END     /* terminado */
    } state_t;
    state_t state;

    /* memoria arquitetural */
    logic [63:0] word [0:MEM_WORDS-1];   /* DISCO */
    logic [7:0]  imem [0:PROG_BYTES-1];  /* program stream (bytes) */

    /* instr decodificada */
    logic [23:0] ra, rb, rr;
    logic [7:0]  rop;

    /* microarquitetura */
    logic [PC_W-1:0] pc;
    logic [31:0]     steps;
    logic [63:0]     canal;
    logic [63:0]     a_tmp, b_tmp;

    logic [63:0]     alu_r;

    /* ------------------------------------------------------------------
     * ALU 64-bit — transformacao funcional (api.h), SAME as exec80.
     * DIV(a,0)=0; op desconhecido -> r=0 (default do switch em micro.c).
     * ------------------------------------------------------------------ */
    always_comb begin
        case (rop)
            IOP_ADD: alu_r = a_tmp + b_tmp;
            IOP_SUB: alu_r = a_tmp - b_tmp;
            IOP_MUL: alu_r = a_tmp * b_tmp;
            IOP_DIV: alu_r = (b_tmp == 64'd0) ? 64'd0 : (a_tmp / b_tmp);
            IOP_AND: alu_r = a_tmp & b_tmp;
            IOP_OR : alu_r = a_tmp | b_tmp;
            IOP_XOR: alu_r = a_tmp ^ b_tmp;
            IOP_NOT: alu_r = ~a_tmp;
            IOP_MOV: alu_r = a_tmp;
            default: alu_r = 64'd0;
        endcase
    end

    /* ------------------------------------------------------------------
     * Portas de escrita (loads externos, antes do start): carregam o
     * programa (imem) e o DISCO (word[]). Always blocks independentes,
     * desacoplados da FSM de execucao.
     * ------------------------------------------------------------------ */
    always @(posedge clk) begin
        if (p_wr) imem[p_addr] <= p_data;
    end

    always @(posedge clk) begin
        if (d_wr) word[d_addr] <= d_din;
    end

    /* ------------------------------------------------------------------
     * FSM principal — espelha roda80/exec80 (sequencial, sem pipeline).
     * ------------------------------------------------------------------ */
    always @(posedge clk) begin
        if (rst) begin
            state  <= S_IDLE;
            done   <= 1'b0;
            halted <= 1'b0;
            timeout<= 1'b0;
            pc     <= {PC_W{1'b0}};
            steps  <= 32'd0;
            canal  <= 64'd0;
            a_tmp  <= 64'd0;
            b_tmp  <= 64'd0;
            for (int i = 0; i < MEM_WORDS; i = i + 1)
                word[i] <= 64'd0;          /* maq_clear: todos zeros */
        end else begin
            case (state)
                S_IDLE: begin
                    if (start) begin
                        pc     <= {PC_W{1'b0}};   /* roda80: pc = 0 */
                        steps  <= 32'd0;
                        halted <= 1'b0;
                        timeout<= 1'b0;
                        done   <= 1'b0;
                        canal  <= canal_init;
                        state  <= S_IF;
                    end
                end

                S_IF: begin
                    /* loop de roda80: pc+10 <= nbytes, senao termina sem HALT */
                    if (({1'b0, pc} + 10) > {1'b0, prog_bytes}) begin
                        state <= S_END;
                    end else if (steps >= MAX_STEPS) begin
                        timeout <= 1'b1;
                        state   <= S_END;
                    end else begin
                        steps <= steps + 32'd1;    /* 1 por instr (inclui HALT) */
                        /* FETCH: bytes little-endian por campo (encode80) */
                        ra <= {imem[pc+2], imem[pc+1], imem[pc]};
                        rb <= {imem[pc+5], imem[pc+4], imem[pc+3]};
                        rop<= imem[pc+6];
                        rr <= {imem[pc+9], imem[pc+8], imem[pc+7]};
                        state <= S_ID;
                    end
                end

                S_ID: begin
                    if (rop == IOP_HALT) begin     /* controle da maquina */
                        halted <= 1'b1;
                        state  <= S_END;
                    end else if (rop == IOP_JMP) begin
                        pc <= {3'b0, ra};           /* MOVE(pc, 0) */
                        state <= S_IF;              /* re-checa pc+10<=nbytes */
                    end else begin
                        state <= S_LD_A;
                    end
                end

                S_LD_A: begin
                    /* LOAD a = MOVE(a, +1) */
                    a_tmp <= word[ra[IDX_W-1:0]];
                    canal <= word[ra[IDX_W-1:0]];
                    if (rop == IOP_NOT || rop == IOP_MOV)  /* unario: b nao e lido */
                        state <= S_ALU;
                    else
                        state <= S_LD_B;
                end

                S_LD_B: begin
                    /* LOAD b = MOVE(b, +1) */
                    b_tmp <= word[rb[IDX_W-1:0]];
                    canal <= word[rb[IDX_W-1:0]];
                    state <= S_ALU;
                end

                S_ALU: begin
                    canal <= alu_r;
                    state <= S_WB;
                end

                S_WB: begin
                    /* STORE r = MOVE(r, -1); canal <- r (STORE_VAL) */
                    word[rr[IDX_W-1:0]] <= alu_r;
                    state <= S_PCU;
                end

                S_PCU: begin
                    pc    <= pc + 10;               /* fluxo linear */
                    state <= S_IF;
                end

                S_END: begin
                    done <= 1'b1;
                end
            endcase
        end
    end

    /* observaveis (estado arquitetural continuo) */
    assign pc_out    = pc;
    assign steps_out = steps;
    assign canal_out = canal;

    /* porta de leitura do DISCO (verificacao / I/O mapeado) */
    always_comb begin
        if (d_rd) d_dout = word[d_addr];
        else      d_dout = 64'd0;
    end

endmodule

`default_nettype wire
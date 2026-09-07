/*
 * micro_disco_ser.sv — Variante TEMP: DISCO serializado para EBR (ECP5)
 *                      HARNESS DE MEDIÇÃO — NÃO é o micro.sv contratual.
 *
 * Delta único vs micro.sv (contrato intacto):
 *   (1) Os acessos a word[] sao serializados em UMA porta de leitura +
 *       UMA porta de escrita (1R/1W), via estados FSM adicionais
 *       S_RD_A / S_RD_B. Sequencia fisica por instrucao:
 *         FETCH -> READ-A -> LOAD-A -> READ-B -> LOAD-B -> ALU -> WRITE-R -> PC
 *       (mais 2 ciclos por instrucao binaria; unarias +1; steps_out conta
 *        INSTRUCOES, nao ciclos — inalterado).
 *   (2) Reset NÃO zera word[] (BRAM/EBR não tem reset de conteúdo).
 *       Equivalencia preservada: em V01..V10 todo word lido é pre-carregado
 *       pelo host via d_wr (maq_clear+word_init do testbench). Delta
 *       registrado no milestone; ISA e semantica LOAD->LOAD->ALU->STORE
 *       inalteradas, mesma aposta de data nas portas.
 *   (3) d_dout passa a ser leitura SÍNCRONA (registered read): valida um
 *       relogio apos d_rd/d_addr (equivalente a EBR sync-read). O TB TEMP
 *       espera o posedge antes de amostrar.
 *
 * SEM alteracoes de api.h, opcode, formato 80-bit, semantica, programas,
 * CAN/J1939, snapshots.
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

    /* DISCO (word[]): carga inicial sincrona + leitura de verificacao sincrona */
    input  wire [11:0]        d_addr,
    input  wire [63:0]        d_din,
    input  wire               d_wr,
    input  wire               d_rd,
    output wire [63:0]        d_dout
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
        S_RD_A,   /* arma leitura A (endereco ra na porta unica) */
        S_LD_A,   /* LOAD a : a_tmp <- rdata_q */
        S_RD_B,   /* arma leitura B (endereco rb na porta unica) */
        S_LD_B,   /* LOAD b : b_tmp <- rdata_q */
        S_ALU,    /* ALU    */
        S_WB,     /* STORE  (escrita via porta unica) */
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
    logic [63:0]     rdata_q;              /* saída registrada da porta unica */

    logic [63:0]     alu_r;

    /* ------------------------------------------------------------------
     * Porta unica de LEITURA do DISCO (registered read, EBR sync):
     * um unico endereco fisico, muxado entre host (d_addr) e FSM
     * (ra em S_RD_A, rb em S_RD_B). rdata_q amostra word[mraddr] 1 ciclo
     * apos o endereco ficar estavel.
     * ------------------------------------------------------------------ */
    wire [IDX_W-1:0] mraddr =
        (state == S_RD_A) ? ra[IDX_W-1:0]   :
        (state == S_RD_B) ? rb[IDX_W-1:0]   :
        d_addr;

    always @(posedge clk) begin
        if (rst)      rdata_q <= 64'd0;
        else          rdata_q <= word[mraddr];
    end

    /* ------------------------------------------------------------------
     * Porta unica de ESCRITA do DISCO (EBR write port):
     * arbitrada entre host (d_wr) e FSM (S_WB). Nunca coincidem no contrato.
     * ------------------------------------------------------------------ */
    wire [IDX_W-1:0] mwaddr = d_wr ? d_addr : rr[IDX_W-1:0];
    wire [63:0]      mwdata = d_wr ? d_din  : alu_r;
    wire             mwe    = d_wr || (state == S_WB);

    always @(posedge clk) begin
        if (mwe) word[mwaddr] <= mwdata;
    end

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

    /* programa: porta de carga (imem) — independente do DISCO */
    always @(posedge clk) begin
        if (p_wr) imem[p_addr] <= p_data;
    end

    /* ------------------------------------------------------------------
     * FSM principal — espelha roda80/exec80 (sequencial, sem pipeline).
     * Acessos ao DISCO serializados: READ-A/LOAD-A/READ-B/LOAD-B/WRITE-R.
     * Reset NAO zera word[] (BRAM); ISA/resultado inalterados em V01..V10.
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
                        state <= S_RD_A;
                    end
                end

                S_RD_A: begin
                    /* arma endereco ra na porta unica (amostrado 1 ciclo depois) */
                    state <= S_LD_A;
                end

                S_LD_A: begin
                    /* LOAD a = MOVE(a, +1) */
                    a_tmp <= rdata_q;
                    canal <= rdata_q;
                    if (rop == IOP_NOT || rop == IOP_MOV)  /* unario: b nao e lido */
                        state <= S_ALU;
                    else
                        state <= S_RD_B;
                end

                S_RD_B: begin
                    /* arma endereco rb na porta unica (amostrado 1 ciclo depois) */
                    state <= S_LD_B;
                end

                S_LD_B: begin
                    /* LOAD b = MOVE(b, +1) */
                    b_tmp <= rdata_q;
                    canal <= rdata_q;
                    state <= S_ALU;
                end

                S_ALU: begin
                    canal <= alu_r;
                    state <= S_WB;
                end

                S_WB: begin
                    /* STORE r = MOVE(r, -1); canal <- r (STORE_VAL) */
                    /* word[rr] <= alu_r escrito pela porta unica (mwe=1) */
                    /* canal permanece = alu_r (STORE_VAL) */
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

    /* porta de leitura do DISCO (verificacao): now synchronous (EBR) */
    assign d_dout = d_rd ? rdata_q : 64'd0;

endmodule

`default_nettype wire
`timescale 1ns/1ps
module main;
  reg clk=0, rst_pin=0, wr=0;
  reg [8:0] addr=0;
  reg [7:0] din=0;
  wire [7:0] dout;
  wire done, halted, timeout;

  always #5 clk = ~clk;

  top dut(.clk(clk), .rst_pin(rst_pin), .wr(wr), .addr(addr), .din(din),
          .dout(dout), .done(done), .halted(halted), .timeout(timeout));

  task reg_w(input [8:0] a, input [7:0] v);
    begin @(negedge clk); #1; wr=1; addr=a; din=v; #1; @(posedge clk); #1; wr=0; end
  endtask

  task word_store(input [11:0] a, input [63:0] v);
    integer k;
    begin
      reg_w(9'h10C, a[7:0]);
      reg_w(9'h10D, {4'h0, a[11:8]});
      for (k = 0; k < 7; k = k + 1) reg_w(9'h110+k, v[8*k +: 8]);
      reg_w(9'h117, v[63:56]);
      @(posedge clk); #1;
      $display("store word[%0d]=%0d | ADA3..13=%b%b%b%b%b%b%b%b%b%b%b",
        a, v,
        dut.u.\core.word.0.0 .ADA3, dut.u.\core.word.0.0 .ADA4,
        dut.u.\core.word.0.0 .ADA5, dut.u.\core.word.0.0 .ADA6,
        dut.u.\core.word.0.0 .ADA7, dut.u.\core.word.0.0 .ADA8,
        dut.u.\core.word.0.0 .ADA9, dut.u.\core.word.0.0 .ADA10,
        dut.u.\core.word.0.0 .ADA11, dut.u.\core.word.0.0 .ADA12,
        dut.u.\core.word.0.0 .ADA13);
    end
  endtask

  task reset_pulse();
    begin @(negedge clk); rst_pin=1; @(posedge clk); @(negedge clk); rst_pin=0; @(posedge clk); @(negedge clk); end
  endtask

  task rd(input [8:0] a, output [7:0] v);
    begin wr=0; addr=a; #2; v=dout; end
  endtask

  integer k; reg [7:0] b;

  initial begin
    reset_pulse();
    word_store(0, 64'd10);
    word_store(1, 64'd5);
    word_store(2, 64'd3);
    word_store(4, 64'd45);
    word_store(6, 64'd77);
    word_store(8, 64'd99);

    for (k = 0; k <= 8; k = k + 1) begin
      reg_w(9'h118, k[7:0]); reg_w(9'h119, {4'h0, k[11:8]});
      @(posedge clk); #1;
      rd(9'h11C, b);
      $display("read word[%0d] = %02x | ADB0..13=%b%b%b%b %b%b%b%b%b %b%b%b%b%b %b%b | DOB[3:0]=%b%b%b%b",
        k, b,
        dut.u.\core.word.0.0 .ADB0, dut.u.\core.word.0.0 .ADB1, dut.u.\core.word.0.0 .ADB2,
        dut.u.\core.word.0.0 .ADB3, dut.u.\core.word.0.0 .ADB4, dut.u.\core.word.0.0 .ADB5,
        dut.u.\core.word.0.0 .ADB6, dut.u.\core.word.0.0 .ADB7, dut.u.\core.word.0.0 .ADB8,
        dut.u.\core.word.0.0 .ADB9, dut.u.\core.word.0.0 .ADB10, dut.u.\core.word.0.0 .ADB11,
        dut.u.\core.word.0.0 .ADB12, dut.u.\core.word.0.0 .ADB13,
        dut.u.\core.word.0.0 .DOB3, dut.u.\core.word.0.0 .DOB2, dut.u.\core.word.0.0 .DOB1, dut.u.\core.word.0.0 .DOB0);
    end
    #100 $finish;
  end
endmodule
module top(wr, timeout, rst_pin, halted, dout, done, din, clk, addr);
  input wr;
  output timeout;
  input rst_pin;
  output halted;
  output [7:0] dout;
  output done;
  input [7:0] din;
  input clk;
  input [8:0] addr;
  micro_fpga_top u(
    .clk(clk), .rst_pin(rst_pin), .wr(wr), .addr(addr), .din(din),
    .dout(dout), .done(done), .halted(halted), .timeout(timeout));
endmodule
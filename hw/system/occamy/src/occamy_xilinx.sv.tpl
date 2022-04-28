// Copyright 2020 ETH Zurich and University of Bologna.
// Solderpad Hardware License, Version 0.51, see LICENSE for details.
// SPDX-License-Identifier: SHL-0.51

// Author: Nils Wistoff <nwistoff@iis.ee.ethz.ch>
//
// AUTOMATICALLY GENERATED by genoccamy.py; edit the script instead.

`include "axi_flat.svh"

module ${name}_xilinx
import ${name}_pkg::*;
(
  input  logic        clk_i,
  input  logic        rst_ni,
  /// Peripheral clock
  input  logic        clk_periph_i,
  input  logic        rst_periph_ni,
  /// Real-time clock (for time keeping)
  input  logic        rtc_i,
  input  logic        test_mode_i,
  input  logic [1:0]  chip_id_i,
  input  logic [1:0]  boot_mode_i,
  // pad cfg
  output logic [31:0]      pad_slw_o,
  output logic [31:0]      pad_smt_o,
  output logic [31:0][1:0] pad_drv_o,
  // `uart` Interface
  output logic        uart_tx_o,
  input  logic        uart_rx_i,
  // `gpio` Interface
  input  logic [31:0] gpio_d_i,
  output logic [31:0] gpio_d_o,
  output logic [31:0] gpio_oe_o,
  output logic [31:0] gpio_puen_o,
  output logic [31:0] gpio_pden_o,
  // `jtag` Interface
  input  logic        jtag_trst_ni,
  input  logic        jtag_tck_i,
  input  logic        jtag_tms_i,
  input  logic        jtag_tdi_i,
  output logic        jtag_tdo_o,
  // `i2c` Interface
  output logic        i2c_sda_o,
  input  logic        i2c_sda_i,
  output logic        i2c_sda_en_o,
  output logic        i2c_scl_o,
  input  logic        i2c_scl_i,
  output logic        i2c_scl_en_o,
  // `SPI Host` Interface
  output logic        spim_sck_o,
  output logic        spim_sck_en_o,
  output logic [1:0]  spim_csb_o,
  output logic [1:0]  spim_csb_en_o,
  output logic [3:0]  spim_sd_o,
  output logic [3:0]  spim_sd_en_o,
  input        [3:0]  spim_sd_i,

  input  logic [11:0]  ext_irq_i,

  // Boot ROM
  output logic                                             bootrom_en_o,
  // This is actually too wide. But the address width depends on the ROM size, so let Vivado handle
  // this for now
  output logic [47:0] bootrom_addr_o,
  input  logic [31:0] bootrom_data_i,

  /// HBM2e Ports
% for i in range(8):
  ${hbm_xbar.__dict__["out_hbm_{}".format(i)].emit_flat_master_port("hbm_{}".format(i))},
% endfor

  /// PCIe Ports
  ${soc_narrow_xbar.out_pcie.emit_flat_master_port("pcie")},
  ${soc_narrow_xbar.in_pcie.emit_flat_slave_port("pcie")}
  /// HBI Ports
);

  // AXI ports of Occamy top-level
  //////////
  // PCIe //
  //////////
  ${soc_narrow_xbar.out_pcie.req_type()} pcie_axi_req_o;
  ${soc_narrow_xbar.out_pcie.rsp_type()} pcie_axi_rsp_i;

  ${soc_narrow_xbar.in_pcie.req_type()} pcie_axi_req_i;
  ${soc_narrow_xbar.in_pcie.rsp_type()} pcie_axi_rsp_o;

  // Assign structs to flattened ports
  `AXI_FLATTEN_MASTER(pcie, pcie_axi_req_o, pcie_axi_rsp_i)
  `AXI_FLATTEN_SLAVE(pcie, pcie_axi_req_i, pcie_axi_rsp_o)

  /////////
  // HBM //
  /////////
% for i in range(8):
  ${hbm_xbar.__dict__["out_hbm_{}".format(i)].req_type()} hbm_${i}_req_o;
  ${hbm_xbar.__dict__["out_hbm_{}".format(i)].rsp_type()} hbm_${i}_rsp_i;

  // Assign structs to flattened ports
  `AXI_FLATTEN_MASTER(hbm_${i}, hbm_${i}_req_o, hbm_${i}_rsp_i)
% endfor

  ///////////////////
  // Boot ROM      //
  ///////////////////
  // TODO(niwis, aottaviano) This is a temporary solution. Either put this in a dedicated module for
  // regbus <-> Xilinx memory conversion and add support to solder, or replace by a different ROM

  ${soc_axi_lite_narrow_periph_xbar.out_bootrom.req_type()} bootrom_axi_lite_req;
  ${soc_axi_lite_narrow_periph_xbar.out_bootrom.rsp_type()} bootrom_axi_lite_rsp;

  <% regbus_bootrom = soc_axi_lite_narrow_periph_xbar.out_bootrom.to_reg(context, "bootrom", fr="bootrom_axi_lite") %>
  ${regbus_bootrom.req_type()} bootrom_req;
  ${regbus_bootrom.rsp_type()} bootrom_rsp;

  logic bootrom_req_ready_d, bootrom_req_ready_q;

  assign bootrom_en_o        = bootrom_req.valid;
  assign bootrom_addr_o      = bootrom_req.addr;
  assign bootrom_rsp.ready   = bootrom_req_ready_q;
  assign bootrom_rsp.rdata   = bootrom_data_i;
  assign bootrom_rsp.error   = '0;
  assign bootrom_req_ready_d = bootrom_req.valid & ~bootrom_req_ready_q;

  always_ff @(posedge clk_i or negedge rst_ni) begin
    if (!rst_ni) begin
      bootrom_req_ready_q <= 1'b0;
    end else begin
      bootrom_req_ready_q <= bootrom_req_ready_d;
    end
  end

  /// FLLs
  ${soc_axi_lite_narrow_periph_xbar.out_fll_system.req_type()} fll_system_axi_lite_req;
  ${soc_axi_lite_narrow_periph_xbar.out_fll_system.rsp_type()} fll_system_axi_lite_rsp;

  ${soc_axi_lite_narrow_periph_xbar.out_fll_periph.req_type()} fll_periph_axi_lite_req;
  ${soc_axi_lite_narrow_periph_xbar.out_fll_periph.rsp_type()} fll_periph_axi_lite_rsp;

  ${soc_axi_lite_narrow_periph_xbar.out_fll_hbm2e.req_type()} fll_hbm2e_axi_lite_req;
  ${soc_axi_lite_narrow_periph_xbar.out_fll_hbm2e.rsp_type()} fll_hbm2e_axi_lite_rsp;

  <% regbus_fll_system = soc_axi_lite_narrow_periph_xbar.out_fll_system.to_reg(context, "fll_system", fr="fll_system_axi_lite") %>
  <% regbus_fll_periph = soc_axi_lite_narrow_periph_xbar.out_fll_periph.to_reg(context, "fll_periph", fr="fll_periph_axi_lite") %>
  <% regbus_fll_hbm2e = soc_axi_lite_narrow_periph_xbar.out_fll_hbm2e.to_reg(context,   "fll_hbm2e", fr="fll_hbm2e_axi_lite") %>

  // Occamy top-level
  ${name}_top i_${name} (
    .bootrom_req_o   (bootrom_axi_lite_req),
    .bootrom_rsp_i   (bootrom_axi_lite_rsp),
    .fll_system_req_o(fll_system_axi_lite_req),
    .fll_system_rsp_i(fll_system_axi_lite_rsp),
    .fll_periph_req_o(fll_periph_axi_lite_req),
    .fll_periph_rsp_i(fll_periph_axi_lite_rsp),
    .fll_hbm2e_req_o (fll_hbm2e_axi_lite_req),
    .fll_hbm2e_rsp_i (fll_hbm2e_axi_lite_rsp),
    .pcie_cfg_req_o  (),
    .pcie_cfg_rsp_i  ('0),
    .ext_irq_i(ext_irq_i),
    // Tie-off unused ports
    .pcie_cfg_rsp_i  ('0),
    .hbi_wide_cfg_rsp_i ('0),
    .hbi_narrow_cfg_rsp_i ('0),
    .hbm_cfg_rsp_i ('0),
    .chip_ctrl_rsp_i ('0),
    .hbi_wide_req_i ('0),
    .hbi_wide_rsp_i ('0),
    .hbi_narrow_req_i ('0),
    .hbi_narrow_rsp_i ('0),
    .*
  );

  assign fll_system_axi_lite_rsp = '0;
  assign fll_periph_axi_lite_rsp = '0;
  assign fll_hbm2e_axi_lite_rsp = '0;

endmodule

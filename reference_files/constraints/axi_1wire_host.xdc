# Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT

create_generated_clock -name CLK_1MHz -source [get_pins -filter {REF_PIN_NAME =~ s00_axi_aclk} -of_objects [get_cells -hierarchical -filter {NAME =~ *axi_1wire_host_slave_lite_v0_1_S00_AXI_inst}]] -divide_by [expr round([expr (1000/double([get_property PERIOD [get_clocks -of_objects [get_pins -filter {REF_PIN_NAME =~ s00_axi_aclk} -of_objects [get_cells -hierarchical -filter {NAME =~ *axi_1wire_host_slave_lite_v0_1_S00_AXI_inst}]]]]))])] [get_pins -filter {REF_PIN_NAME =~ Q} -of_objects [get_cells -hierarchical -filter {NAME =~ *axi_1wire_host_slave_lite_v0_1_S00_AXI_inst/CLK_DIVIDER/clk_out_reg}]]
create_generated_clock -name CLK_50KHz -source [get_pins -filter {REF_PIN_NAME =~ *clk} -of_objects [get_cells -hierarchical -filter {NAME =~ *jc_1us_20us}]] -divide_by 20 [get_pins -filter {REF_PIN_NAME =~ *q[9]} -of_objects [get_cells -hierarchical -filter {NAME =~ *jc_1us_20us}]]

set_clock_groups -name 1wire_clk -physically_exclusive -group [get_clocks CLK_50KHz] -group [get_clocks -of_objects [get_pins -filter {REF_PIN_NAME =~ s00_axi_aclk} -of_objects [get_cells -hierarchical -filter {NAME =~ *axi_1wire_host_slave_lite_v0_1_S00_AXI_inst}]]]

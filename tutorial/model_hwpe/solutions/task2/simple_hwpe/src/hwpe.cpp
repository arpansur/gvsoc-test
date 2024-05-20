/*
 * Copyright (C) 2020-2022  GreenWaves Technologies, ETH Zurich, University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * Authors: Francesco Conti, University of Bologna & GreenWaves Technologies (f.conti@unibo.it)
 *          Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 *          Arpan Suravi Prasad, ETH Zurich (prasadar@iis.ee.ethz.ch)
 */

#include "hwpe.hpp"

Hwpe::Hwpe(vp::ComponentConf &config)
    : vp::Component(config)
{
  this->traces.new_trace("trace", &this->trace, vp::DEBUG);
  this->new_slave_port("config", &this->cfg_port_); 
  this->new_master_port("irq", &this->irq);
  this->new_master_port("tcdm", &this->tcdm_port );

  // contructor of the RegConfigManager class with access to the Hwpe
  this->regconfig_manager_instance = RegConfigManager<Hwpe>(this);
}

void Hwpe::clear(){
  // An empty clear  function. Needs to be extended in later tasks
}

// The `hwpe_slave` member function models an access to the HWPE SLAVE interface
vp::IoReqStatus Hwpe::hwpe_slave(vp::Block *__this, vp::IoReq *req)
{
  Hwpe *_this = (Hwpe *)__this;
  uint8_t *data = req->get_data(); 
  uint32_t addr = req->get_addr();
  
  _this->trace.msg("Received request (addr: 0x%x, size: 0x%x, is_write: %d, data: 0x%x\n", req->get_addr(), req->get_size(), req->get_is_write(), *(uint32_t *)(req->get_data()));
  
  // Dispatch the register file access to the correct function
  if(req->get_is_write()) {
    //The write could be to special reigsters or the configuration registers
    if(addr == HWPE_REG_COMMIT_AND_TRIGGER) {
      // Needs to be handled in the upcoming tasks
    }
    else if(addr == HWPE_REG_SOFT_CLEAR) {
      ////////////////////////////////////////   SOLUTION - 1  //////////////////////////////////////// 
      // call the clear function. We will build on the contents of the clear function later
      _this->clear();
    }
    else if(addr >= HWPE_REGISTER_OFFS) {
      // write to the configuration registers 
      int register_index = (addr - HWPE_REGISTER_OFFS) >> 2;
      _this->trace.msg("Setting Register offset: %d data: %08x\n", register_index, *(uint32_t *) data);
      _this->regconfig_manager_instance.regfile_write(register_index, *(uint32_t *) data);
    }
    else {
      _this->trace.fatal("Unsupported register write\n");
    }
  }
  else {
    if(addr == HWPE_REG_STATUS) {
      // Needs to be handled in the upcoming tasks
    }
    else if(addr >= HWPE_REGISTER_OFFS)
    {
      ////////////////////////////////////////   SOLUTION - 2  //////////////////////////////////////// 
      // write code to assign the register_read value to the data. Take inspiration from the register write. 

      *(uint32_t *) data = _this->regconfig_manager_instance.regfile_read((addr - HWPE_REGISTER_OFFS) >> 2);
      _this->trace.msg("Reading Register :  %x\n", *(uint32_t *) data);

    }
    else {
      _this->trace.fatal("Unsupported register read\n");
    }
  }
  return vp::IO_REQ_OK;
}


extern "C" vp::Component *gv_new(vp::ComponentConf &config)
{
  return new Hwpe(config);
}
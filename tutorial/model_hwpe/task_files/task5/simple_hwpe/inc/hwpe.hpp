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
 * Authors: Arpan Suravi Prasad, ETH Zurich (prasadar@iis.ee.ethz.ch)
 */
#ifndef __HWPE_HPP__
#define __HWPE_HPP__
#include <vp/vp.hpp>
#include <vp/itf/io.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <assert.h>
#include <string>
#include <bitset>
#include "params.hpp"
#include "datatype.hpp"
#include "regconfig_manager.hpp"
class Hwpe : public vp::Component
{
public:
  Hwpe(vp::ComponentConf &config);
  vp::IoMaster tcdm_port;
  vp::Trace trace;
  vp::reg_32 state;
  vp::IoReq io_req;


  //register configuration instance
  RegConfigManager<Hwpe> regconfig_manager_instance;

private:
  vp::IoSlave cfg_port_;
  vp::WireMaster<bool> irq;

  Regconfig reg_config_;


  Iterator input, weight, output;
  
  void clear();
  int  fsm();
  void fsm_loop();

  int64_t input_load();
  void    input_layout();

  int64_t weight_load();
  void    weight_layout();
  
  int64_t weight_offset();
  
  int64_t compute_output();
  int64_t output_store();

  static vp::IoReqStatus hwpe_slave(vp::Block *__this, vp::IoReq *req);

  vp::ClockEvent *fsm_start_event;
  vp::ClockEvent *fsm_event;
  vp::ClockEvent *fsm_end_event;

  static void FsmStartHandler(vp::Block *__this, vp::ClockEvent *event);
  static void FsmHandler(vp::Block *__this, vp::ClockEvent *event);
  static void FsmEndHandler(vp::Block *__this, vp::ClockEvent *event);
};
#endif /* __HWPE_HPP__ */
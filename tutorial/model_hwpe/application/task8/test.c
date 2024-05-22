/*
 * Copyright (C) 2020 ETH Zurich, University of Bologna and GreenWaves Technologies
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
 * Authors:  Francesco Conti <fconti@iis.ee.ethz.ch>
 *           Gianna Paulin <pauling@iis.ee.ethz.ch>
 *           Renzo Andri <andrire@iis.ee.ethz.ch>
 * Main Test Program for the HWPE
 */

#include "pmsis.h"
#include "pmsis/cluster/dma/cl_dma.h"
#include <stdint.h>
#include <stdio.h>

#include "hal.h"
#include "inc/input.h"
#include "inc/weight.h"
#include "inc/actual_output.h"
#include "inc/expected_output.h"

#define NB_ITER 10

static int glob_errors;

int run_test() {

  uint8_t* input_ptr   = hwpe_input;
  uint8_t* weight_ptr  = hwpe_weight;
  uint32_t* actual_output_ptr = hwpe_actual_output;
  uint32_t* expected_output_ptr = hwpe_expected_output;

  // soft-clear HWPE
  HWPE_WRITE_CMD(HWPE_SOFT_CLEAR, HWPE_SOFT_CLEAR_ALL);
  for(volatile int kk=0; kk<10; kk++);

  // program HWPE
  HWPE_WRITE_REG(HWPE_REG_INPUT_PTR,     input_ptr);
  HWPE_WRITE_REG(HWPE_REG_WEIGHT_PTR,    weight_ptr);
  HWPE_WRITE_REG(HWPE_REG_OUTPUT_PTR,    actual_output_ptr);
  HWPE_WRITE_REG(HWPE_REG_WEIGHT_OFFS,    -128);

//   // configure & reset perf counters
//   pi_perf_conf(1 << PI_PERF_CYCLES);
//   pi_perf_reset();

  // commit HWPE computation
  HWPE_WRITE_CMD(HWPE_COMMIT_AND_TRIGGER, HWPE_TRIGGER_CMD);

//   // start perf counter
//   pi_perf_start();

  // wait on barrier
  HWPE_BARRIER();

//   // stop perf counter
//   pi_perf_stop();

//   printf("%d cycles\n", pi_perf_read(PI_PERF_CYCLES));

  int errors = hwpe_compare_int(actual_output_ptr, expected_output_ptr, STIM_HWPE_EXPECTED_OUTPUT_SIZE);
  return errors;
}

static struct pi_cluster_task task[1];
static struct pi_task events[1];

static void pe_entry(void *arg) {
  if(pi_core_id() == 0) {
    glob_errors = run_test();
  }
  pi_cl_team_barrier();
}

static void cluster_entry(void *arg) {
  pi_cl_team_fork(0, pe_entry, 0);
}

static int launch_cluster_task() {
  struct pi_device cluster_dev;
  struct pi_cluster_conf conf;
  struct pi_cluster_task task;

  pi_cluster_conf_init(&conf);
  conf.id = 0;
  glob_errors = 0;

  pi_open_from_conf(&cluster_dev, &conf);
  pi_cluster_open(&cluster_dev);

  pi_cluster_task(&task, cluster_entry, NULL);
  pi_cluster_send_task_to_cl(&cluster_dev, &task);
  pi_cluster_close(&cluster_dev);

  return glob_errors;
}

int test_entry() {
  volatile int errors = launch_cluster_task();
  
  if (errors)
    printf("Test failure\n");
  else
    printf("Test success\n");
  return errors;
}

void test_kickoff(void *arg) {
  int ret = test_entry();
  pmsis_exit(ret);
}

int main() {
  return pmsis_kickoff((void *)test_kickoff);
}
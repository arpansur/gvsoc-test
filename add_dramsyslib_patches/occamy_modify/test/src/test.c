#include "snrt.h"
#include "hal_redmule.h"
#include "inc/w_input.h"
#include "inc/x_input.h"
#include "inc/y_input.h"
#include "inc/z_output.h"
#include "inc/config.h"

uint32_t L = 16;

float a = 2;

float x[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

float y[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,  1,  1,  1,  1,  1};

float z[16];

float w[16];

void axpy(uint32_t l, float a, float *x, float *y, float *z) {
    for (uint32_t i = 0; i < l ; i++) {
        z[i] = a * x[i] + y[i];
    }
    snrt_fpu_fence();
}

int main()
{
	float *local_x, *remote_x;
	local_x  = 0x10000000 + (0x40000 * snrt_cluster_idx());
	remote_x = 0xc0000000;
	// Copy data in TCDM
	if (snrt_cluster_idx() < 24)
	{
		if (snrt_is_dm_core()) {
	        size_t size = 65536;
	        snrt_dma_start_1d(local_x, remote_x, size);
	        snrt_dma_wait_all();
	    }
	}
	    
    snrt_cluster_hw_barrier();

	if (snrt_cluster_core_idx() == 0)
	{
		redmule_cfg(M_SIZE, N_SIZE, K_SIZE, gemm_ops);
		redmule_x_add_set(x);
		redmule_w_add_set(w);
		redmule_z_add_set(z);
		redmule_y_add_set(y);
		hwpe_trigger_job();
		hwpe_get_status();
		hwpe_soft_clear();
		redmule_cfg(M_SIZE, N_SIZE, K_SIZE, gemm_ops);
		redmule_x_add_set(x);
		redmule_w_add_set(w);
		redmule_z_add_set(z);
		redmule_y_add_set(y);
		hwpe_trigger_job();
		hwpe_get_status();
	}
	return 0;
}
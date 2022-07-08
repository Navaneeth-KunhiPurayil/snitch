#include "SpVV.h"
#include "data-spv-spv.h"

int32_t *ptr_a, *ptr_b;
int32_t *ptr_nz_a, *ptr_nz_b, *ptr_coo_a, *ptr_coo_b;
int32_t *ptr_nz_rle_a,*ptr_nz_rle_b, *ptr_rle_a, *ptr_rle_b;
int32_t *ptr_bitmap_a, *ptr_bitmap_b;

size_t benchmark_get_cycle() { return read_csr(mcycle); } 

void assign_memory(int32_t *ptr, int32_t arr[], uint32_t len) {
	for(uint32_t i=0; i < len; i++) {
		*ptr = arr[i];
		ptr++;
	}
}

void setup_memory() {
	int32_t *ptr = snrt_cluster_memory().start;
	ptr_a = ptr; 
	assign_memory(ptr_a, a, len);
	ptr += len + 1;
	
	ptr_b = ptr; 
	assign_memory(ptr_b, b, len); 
	ptr += len + 1;

	ptr_nz_a = ptr; 
	assign_memory(ptr_nz_a, nz_a, nz_len);
	ptr += nz_len + 1;

	ptr_nz_b = ptr; 
	assign_memory(ptr_nz_b, nz_b, nz_len);
	ptr += nz_len + 1;

	ptr_coo_a = ptr; 
	assign_memory(ptr_coo_a, coo_a, nz_len);
	ptr += nz_len + 1;

	ptr_coo_b = ptr; 
	assign_memory(ptr_coo_b, coo_b, nz_len);
	ptr += nz_len + 1;

	ptr_nz_rle_a = ptr; 
	assign_memory(ptr_nz_rle_a, nz_rle_a, rle_len_a);
	ptr += rle_len_a + 1;

	ptr_nz_rle_b = ptr; 
	assign_memory(ptr_nz_rle_b, nz_rle_b, rle_len_b);
	ptr += rle_len_b + 1;
	
        ptr_rle_a = ptr; 
	assign_memory(ptr_rle_a, rle_a, rle_len_a);
	ptr += rle_len_a + 1;
        
        ptr_rle_b = ptr; 
	assign_memory(ptr_rle_b, rle_b, rle_len_b);
	ptr += rle_len_b + 1;

	ptr_bitmap_a = ptr; 
	assign_memory(ptr_bitmap_a, bitmap_a, len);
	ptr += len + 1;
	
        ptr_bitmap_b = ptr; 
	assign_memory(ptr_bitmap_b, bitmap_b, len);
	ptr += len + 1;
}

int32_t spVspV_baseline() {
    
    int32_t c = 0;
    for (uint32_t i=0; i<len; i++)
    {
    	c += ptr_a[i]*ptr_b[i];
    }
    return c;
}

int32_t spVspV_coordinate() {
    int32_t c=0;
    int32_t *idx_a = ptr_coo_a;
    int32_t *idx_b = ptr_coo_b; 
    /* while(idx_a < (ptr_coo_a + nz_len) && idx_b < (ptr_coo_b + nz_len)) {
        if (*idx_a < *idx_b) {
        	idx_a++;
        } else if (*idx_a > *idx_b) {
        	idx_b++;
        } else {
        	c+=ptr_a[*idx_a] * ptr_b[*idx_b];
        	idx_a++;
        	idx_b++;
        }
    }
    return c; */

    int32_t b_val;
    uint32_t idx; 
    for(uint32_t i=0; i < nz_len; i++) {
        idx = ptr_coo_a[i];
        b_val = ptr_b[idx]; 
        if (b_val) {
            c += b_val * ptr_nz_a[i];
        }
    }
    return c;

}

int main()
{
    int32_t res_act;
    size_t t1,t2,d;
    setup_memory();

    t1 = benchmark_get_cycle(); // dummy statement
    
    // Baseline
    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVspV_baseline();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("baseline: %u res_act:%u\n",t2-t1,res_act);
    
    
    snrt_cluster_hw_barrier();
    t1 = benchmark_get_cycle();
    res_act = spVspV_coordinate();
    t2 = benchmark_get_cycle();
    snrt_cluster_hw_barrier();
    printf_("coordinate: %u res_act:%u\n",t2-t1,res_act);

    return 0;
}

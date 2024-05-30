#include <stdio.h>
#include <stdlib.h>
#include <rte_eal.h>
#include <rte_common.h>
#include <rte_log.h>
#include <rte_mbuf.h>

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250

/* echo.c: Basic DPDK UDP echo server */

int main(int argc, char *argv[]) {
	struct rte_mempool *mbuf_pool;

	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS, 
			MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	rte_log(RTE_LOG_INFO, RTE_LOGTYPE_USER1, "Initialisation complete\n");

	rte_eal_cleanup();

	return 0;
}

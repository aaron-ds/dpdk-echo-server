#include <stdio.h>
#include <stdlib.h>
#include <rte_eal.h>
#include <rte_common.h>
#include <rte_log.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250

#define RTE_LOGTYPE_APP RTE_LOGTYPE_USER1

/* echo.c: Basic DPDK UDP echo server */


int
port_init(uint16_t port_id, struct rte_mempool *mbuf_pool) {
	int retval = 0;
	struct rte_eth_conf port_conf;
	const uint16_t rx_rings = 1, tx_rings = 1;
	uint16_t nb_rx_desc = RX_RING_SIZE;
	uint16_t nb_tx_desc = TX_RING_SIZE;
	struct rte_eth_dev_info dev_info;
	struct rte_eth_txconf txconf;
	
	if (!rte_eth_dev_is_valid_port(port_id))
		return -1;

	memset(&port_conf, 0, sizeof(struct rte_eth_conf));

	// query the port for its capabilities
	retval = rte_eth_dev_info_get(port_id, &dev_info);
	if (retval != 0) {
		printf("Error getting device info for port %u: %s\n", port_id, strerror(-retval));
		return retval;
	}

	RTE_LOG(INFO, APP, "0x%"PRIx64"\n", port_conf.rxmode.offloads);
	if (dev_info.tx_offload_capa & RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE) {
		RTE_LOG(INFO, APP, "Enabling RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE\n");
		port_conf.txmode.offloads |= RTE_ETH_TX_OFFLOAD_MBUF_FAST_FREE;
	}

	retval = rte_eth_dev_configure(port_id, rx_rings, tx_rings, &port_conf);
	if (retval != 0)
		return retval;

	retval = rte_eth_dev_adjust_nb_rx_tx_desc(port_id, &nb_rx_desc, &nb_tx_desc);
	if (retval != 0)
		return retval;	

	retval = rte_eth_rx_queue_setup(port_id, 0, nb_rx_desc, rte_eth_dev_socket_id(port_id), NULL, mbuf_pool);
	if (retval < 0)
		return retval;

	txconf = dev_info.default_txconf;
	txconf.offloads = port_conf.txmode.offloads;

	retval = rte_eth_tx_queue_setup(port_id, 0, nb_tx_desc, rte_eth_dev_socket_id(port_id), &txconf);
	if (retval < 0)
		return retval;

	retval = rte_eth_dev_start(port_id);
	if (retval < 0)
		return retval;

	struct rte_ether_addr addr;
	retval = rte_eth_macaddr_get(port_id, &addr);
	if (retval < 0)
		return retval;

	RTE_LOG(INFO, APP, "Port %u MAC Address: %02" PRIx8 " %02" PRIx8 " %02" PRIx8
			" %02" PRIx8 " %02" PRIx8 " %02" PRIx8 "\n",
			port_id, RTE_ETHER_ADDR_BYTES(&addr));

	retval = rte_eth_promiscuous_enable(port_id);
	if (retval != 0)
		return retval;

	return 0;
}

int 
main(int argc, char *argv[]) {
	struct rte_mempool *mbuf_pool;
	uint16_t port_id;

	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

	mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS, 
			MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

	if (mbuf_pool == NULL)
		rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

	RTE_ETH_FOREACH_DEV(port_id) {
		if (port_init(port_id, mbuf_pool) != 0)
			rte_exit(EXIT_FAILURE, "Error initialising port %u\n", port_id);
		RTE_LOG(INFO, APP, "Initialised port id %u\n", port_id);
	}

	RTE_LOG(INFO, APP, "Initialisation complete\n");

	rte_eal_cleanup();

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <rte_eal.h>
#include <rte_common.h>
#include <rte_log.h>
#include <rte_mbuf.h>
#include <rte_ethdev.h>
#include <rte_byteorder.h>
#include <rte_udp.h>

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024

#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

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


__rte_noreturn void
lcore_main(void) {
	uint16_t port_id = 0;
	for (;;) {
		struct rte_mbuf *bufs[BURST_SIZE];
		const uint16_t nb_rx = rte_eth_rx_burst(port_id, 0, bufs, BURST_SIZE);
		if (unlikely(nb_rx == 0))
			continue;

		struct rte_ether_hdr *ether_hdr;
		struct rte_ipv4_hdr *ipv4_hdr;
		struct rte_udp_hdr *udp_hdr;
		u_int8_t src_mac[RTE_ETHER_ADDR_LEN];
		u_int8_t dest_mac[RTE_ETHER_ADDR_LEN];
		u_int16_t ether_type;
		u_int16_t src_port;
		u_int16_t dest_port;
		for (int i = 0; i < nb_rx; i++) {
			RTE_LOG(INFO, APP, "Received a frame\n");
			ether_hdr = rte_pktmbuf_mtod(bufs[i], struct rte_ether_hdr *);
			ether_type = ether_hdr->ether_type;
			rte_memcpy(src_mac, &ether_hdr->src_addr, sizeof(u_int8_t) * RTE_ETHER_ADDR_LEN);
			rte_memcpy(dest_mac, &ether_hdr->dst_addr, sizeof(u_int8_t) * RTE_ETHER_ADDR_LEN);
                        RTE_LOG(INFO, APP, "Frame received from MAC Address: %02x:%02x:%02x:%02x:%02x:%02x with type: %04x\n",
                                        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5], rte_bswap16(ether_type));
			if (rte_bswap16(ether_type) == RTE_ETHER_TYPE_IPV4) {
				ipv4_hdr = rte_pktmbuf_mtod_offset(bufs[i], struct rte_ipv4_hdr*, sizeof(struct rte_ether_hdr));
				u_int8_t next_proto = ipv4_hdr->next_proto_id;
				RTE_LOG(INFO, APP, "next proto is %x\n", next_proto);
				if (next_proto == IPPROTO_UDP) {
					RTE_LOG(INFO, APP, "Received UDP message\n");				
					udp_hdr = rte_pktmbuf_mtod_offset(bufs[i], struct rte_udp_hdr*, sizeof(struct rte_ether_hdr) + sizeof(struct rte_ipv4_hdr));
					src_port = udp_hdr->src_port;
					dest_port = udp_hdr->dst_port;
					RTE_LOG(INFO, APP, "Source port: %u, Destination port: %u\n", rte_bswap16(udp_hdr->src_port), rte_bswap16(udp_hdr->dst_port));	
				}
			}	
		}
	}

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

	lcore_main();

	rte_eal_cleanup();

	return 0;
}

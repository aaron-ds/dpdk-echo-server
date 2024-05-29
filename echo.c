#include <stdio.h>
#include <stdlib.h>
#include <rte_eal.h>
#include <rte_common.h>

int main(int argc, char *argv[]) {
	int ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
	return 1;
}

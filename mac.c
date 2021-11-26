#include "mac.h"
#include "log.h"

#include <stdlib.h>
#include <string.h>

mac_port_map_t mac_port_map;

// initialize mac_port table
void init_mac_port_table()
{
	bzero(&mac_port_map, sizeof(mac_port_map_t));	

	// 初始化链表头
	for (int i = 0; i < HASH_8BITS; i++) {
		init_list_head(&mac_port_map.hash_table[i]);
	}

	pthread_mutex_init(&mac_port_map.lock, NULL);
	
	//创建线程
	pthread_create(&mac_port_map.thread, NULL, sweeping_mac_port_thread, NULL);
}

// destroy mac_port table
void destory_mac_port_table()
{
	pthread_mutex_lock(&mac_port_map.lock);
	mac_port_entry_t *entry, *q;
	for (int i = 0; i < HASH_8BITS; i++) {
		list_for_each_entry_safe(entry, q, &mac_port_map.hash_table[i], list) {
			list_delete_entry(&entry->list);
			free(entry);
		}
	}
	pthread_mutex_unlock(&mac_port_map.lock);
}

// lookup the mac address in mac_port table
iface_info_t *lookup_port(u8 mac[ETH_ALEN])
{
	// TODO: implement the lookup process here
	fprintf(stdout, "TODO: implement the lookup process here.\n");
	u8 index = hash8(mac,ETH_ALEN);	//compute hash value
	//printf("MyFlag: hash index is %d\n",index);

	mac_port_entry_t *entry;
	int findFlag = 0;
	
	pthread_mutex_lock(&mac_port_map.lock);		//lock the map
	list_for_each_entry(entry, &mac_port_map.hash_table[index], list) {	 //遍历对应hash表
		findFlag = 0;
		for(int i = 0; i < ETH_ALEN; i++){
			if(entry->mac[i] == mac[i])
				findFlag++;
			else
				break;
		}
		if(findFlag == ETH_ALEN){	// 找到了mac addr
			break;
		}
	}
	if(findFlag == ETH_ALEN){
		entry->visited = time(NULL);		// refresh time
		pthread_mutex_unlock(&mac_port_map.lock);	// unlock
		fprintf(stdout, "MyFlag: mac addr have benn found.\n");
		return entry->iface;
	}else{
		pthread_mutex_unlock(&mac_port_map.lock);	// unlock
		fprintf(stdout, "MyFlag: mac addr not found.\n");
		return NULL;
	}
}

// insert the mac -> iface mapping into mac_port table
void insert_mac_port(u8 mac[ETH_ALEN], iface_info_t *iface)
{
	// TODO: implement the insertion process here
	fprintf(stdout, "TODO: implement the insertion process here.\n");
	mac_port_entry_t *insert_port;
	insert_port = (mac_port_entry_t*)malloc(sizeof(mac_port_entry_t));	//为端口信息开辟新空间
	for(int i = 0; i < ETH_ALEN; i++)		// 记录端口信息
		insert_port->mac[i] = mac[i];
	insert_port->iface = iface;
	insert_port->visited = time(NULL);

	u8 index = hash8(mac,ETH_ALEN);			// 计算hash值
	
	pthread_mutex_lock(&mac_port_map.lock);		// 上锁
	list_add_head(&insert_port->list, &mac_port_map.hash_table[index]);	//	插入该端口
	pthread_mutex_unlock(&mac_port_map.lock);	// 解锁
}

// dumping mac_port table
void dump_mac_port_table()
{
	mac_port_entry_t *entry = NULL;
	time_t now = time(NULL);

	fprintf(stdout, "dumping the mac_port table:\n");
	pthread_mutex_lock(&mac_port_map.lock);
	for (int i = 0; i < HASH_8BITS; i++) {
		list_for_each_entry(entry, &mac_port_map.hash_table[i], list) {
			fprintf(stdout, ETHER_STRING " -> %s, %d\n", ETHER_FMT(entry->mac), \
					entry->iface->name, (int)(now - entry->visited));
		}
	}

	pthread_mutex_unlock(&mac_port_map.lock);
}

// sweeping mac_port table, remove the entry which has not been visited in the
// last 30 seconds.
int sweep_aged_mac_port_entry()
{
	// TODO: implement the sweeping process here
	pthread_mutex_lock(&mac_port_map.lock);
	int del_count = 0;
	mac_port_entry_t *entry, *q;
	for (int i = 0; i < HASH_8BITS; i++) {
		list_for_each_entry_safe(entry, q, &mac_port_map.hash_table[i], list) {
			if((int)(time(NULL) - entry->visited) > 30){
				del_count++;
				list_delete_entry(&entry->list);
				free(entry);  //非safe方法，此处free会异常
			}
		}
	}
	pthread_mutex_unlock(&mac_port_map.lock);
	return del_count;
}

// sweeping mac_port table periodically, by calling sweep_aged_mac_port_entry
void *sweeping_mac_port_thread(void *nil)
{
	while (1) {
		sleep(1);
		int n = sweep_aged_mac_port_entry();
		if (n > 0)
			log(DEBUG, "%d aged entries in mac_port table are removed.", n);
	}

	return NULL;
}

<img src="C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211101202101635.png" alt="image-20211101202101635" style="zoom:50%;" />

<img src="C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211101202143627.png" alt="image-20211101202143627" style="zoom:50%;" />



存放端口号

```c
typedef struct {
	struct list_head iface_list;	// the list of interfaces, 双向链表
	int nifs;						// number of interfaces
	struct pollfd *fds;				// structure used to poll packets among 
								    // all the interfaces
} ustack_t;
```



mac端口表，使用双向内核链表list_head，通过宏操作链表

```c
typedef struct {
	struct list_head hash_table[HASH_8BITS];
	pthread_mutex_t lock;
	pthread_t thread;
} mac_port_map_t;
```





![](C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211109104816968.png)

![image-20211109104845000](C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211109104845000.png)

![image-20211109104902592](C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211109104902592.png)

![image-20211109104927975](C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211109104927975.png)

![image-20211109104951129](C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211109104951129.png)

![image-20211109105005174](C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211109105005174.png)

![image-20211109105524725](C:\Users\SuperACE\AppData\Roaming\Typora\typora-user-images\image-20211109105524725.png)

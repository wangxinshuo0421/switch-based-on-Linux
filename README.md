一、	实验题目
交换机转发实验

二、	实验目的
1.	学习交换机的工作原理；
2.	学习交换机转发和广播数据包的流程；
3.	学习转发表的数据结构；
4.	学习转发表的维护操作。

三、	实验环境
虚拟平台：Vmware Workstation Pro
操作系统：Ubuntu 16.04 LTS
gcc：5.4.0
python：2.7.12
Mininet：2.3.0
Wireshark：2.6.10

四、	实验内容
1.	在主机h2和h3上分别打开wireshark程序，监听各自主机的eth0端口（h2-eth0和h3-eth0）。
2.	在h1主机上分别ping h2和h3两个主机，在h2和h3两个主机上的wireshark捕获的应该只包含自己节点和h1产生的数据包。

五、	实验流程
1.	首先分析交换机中所包含的数据结构：
①	：端口信息结构体：iface_info_t

![image](https://user-images.githubusercontent.com/40001579/143566258-acd0ce30-caab-4c73-ac59-991525fa6aff.png)

	该结构体存放了交换机上的端口信息，包括文件描述符、端口ID、mac地址、端口名称，并通过内核链表进行连接。
	
②	：mac地址与端口映射表：mac_port_entry
 ![image](https://user-images.githubusercontent.com/40001579/143566276-98f87b5d-e5e6-4947-af1e-e2d6808f25e7.png)

	该结构体存放了交换机记录的转发mac地址，与mac地址对应的转发端口信息，以及老化时间visited，也是通过内核链表进行连接。
	
③	：mac地址与hash映射表：mac_port_map_t
 ![image](https://user-images.githubusercontent.com/40001579/143566286-705e9599-6394-4e90-a10f-8c44fc102d63.png)

	该结构体通过内存放了256个hash key，每个hash key 通过内核链表串联起映射到自己的mac地址表（即上述第②个结构体），结构体内还包含互斥锁供多线程操作。

2.	实现对数据结构mac_port_map的所有操作，以及数据包的转发和广播操作。

①. 首先实现iface_info_t *lookup_port(u8 mac[ETH_ALEN])函数，该函数的难点在于遍历hash表来找到对应的端口号，但内核链表给我们提供了宏函数list_for_each_entry来进行遍历操作。

 ![image](https://user-images.githubusercontent.com/40001579/143566331-b8668478-61bf-45e7-b9b7-14aed033d2f1.png)

我的实现过程是首先根据mac地址计算对应的hash值，然后通过list_for_each_entry宏函数对对应hash值下的链表进行遍历，遍历时加互斥锁，退出时解锁，通过findFlag作为找到对应mac地址的标志。若找到，则在退出时更新老化时间visit，并返回转发端口；若未找到，则在退出时返回null。

②. 接下来实现void insert_mac_port(u8 mac[ETH_ALEN], iface_info_t *iface)函数。
 ![image](https://user-images.githubusercontent.com/40001579/143566344-a3ad693c-01e0-416e-9271-6347b608f5a9.png)

该函数主要是对结构体mac_port_entry_t的操作，首先构建要插入的端口信息结构体，然后计算该收到包的mac地址对应的hash值，将该结构体插入到对应hash值下的链表，该插入操作是通过内核链表提供的宏函数list_add_head所完成的，执行插入操作时，对映射表加互斥锁。

③. 接下来是int sweep_aged_mac_port_entry()函数的实现。
 ![image](https://user-images.githubusercontent.com/40001579/143566362-c239ce61-c80a-4286-905f-26235e00482f.png)

该函数的难点在于遍历并删除超过30秒未访问的转发条目，通过内核链表提供的list_for_each_entry_safe宏函数对链表进行遍历，找到与当前时间差超过30的条目并执行删除操作，删除操作使用的也是系统提供的宏函数。在调试过程中发现，执行free操作必须要使用safe的遍历方法，否则会产生指针错误。

④. 接下来是void broadcast_packet(iface_info_t *iface, char *packet, int len)函数的实现。
 ![image](https://user-images.githubusercontent.com/40001579/143566368-a6473278-7130-4017-b788-db861a906e9a.png)

该函数要实现的是广播操作，通过list_for_each_entry函数进行遍历，找到非发送端口并将数据包发送出去，实现广播操作。

⑤. 最后是实现void handle_packet(iface_info_t *iface, char *packet, int len)函数。
 ![image](https://user-images.githubusercontent.com/40001579/143566380-b4fef795-d6ee-494f-aaac-d1c426eba2f7.png)

该函数要实现的是包处理功能，收到数据包后首先获取包内的目的地址，通过lookup_port函数查找转发表中是否存在对应的转发端口，若存在，则通过对应端口转发，否则广播该数据包。之后获取数据包的源地址，在转发表中若未找到该地址的条目，则将其mac地址以及对应端口插入至转发表。

3. 基于Ubuntu系统，在mininet平台验证交换机程序的正确性
①. 首先通过make命令，对所有代码进行联合编译。
②. 打开终端，运行sudo python2 three_nodes_bw.py命令，启动老师给定的拓扑结构。
③. 在mininet平台上，通过xterm s1 h2 h3命令，启动s1,h2,h3的虚拟终端。
④. 在s1上运行./switch命令，启动交换机程序。
⑤. 在h2和h3上运行sudo wireshark命令，启动wireshark软件，并监听各自的eth0端口的网络数据。至此的运行界面如图1所示。
⑥. 在mininet中运行h1 ping h2和h1 ping h3命令。
h1 ping h2的结果如图2所示，在h2-eth0端口捕获到了源地址为h1目的地址为h2的数据包，同时，在h3-eth0端口没有捕捉到来自h1的数据包，只有ARP的广播地址包。
h1 ping h3的结果如图3所示，在h3-eth0端口捕获到了源地址为h1目的地址为h3的数据包，同时，在h2-eth0端口没有捕捉到来自h1的数据包，只有ARP的广播地址包。
老化操作如图4所示，s1主机展示了删除表项的信息（最后一行）。
实验结果表明，基本符合实验要求，实现了交换机对数据包的转发功能以及转发表的维护。

 ![image](https://user-images.githubusercontent.com/40001579/143566406-7e7110b9-eb7b-47e0-9a55-bf9e70cb5818.png)
图1 实验初始化环境展示

![image](https://user-images.githubusercontent.com/40001579/143566419-e81963fc-3848-42fd-894d-6f6186ead78a.png)
图2 h1 ping h2 实验结果

 ![image](https://user-images.githubusercontent.com/40001579/143566441-65bffc07-7c4c-4c13-b595-5088e242b2f8.png)
图3 h1 ping h3 实验结果

![image](https://user-images.githubusercontent.com/40001579/143566449-406502d5-eb45-4835-8b01-7c9f14c527e6.png)
图4 s1交换机删除表项信息

六、	实验问题分析
1. 实验过程中遇到的第一个问题是内核链表的使用，通过资料的查找和学习，理解了其工作原理，通过计算结构体中链表的地址以及结构体的地址来找到结构体指针应该指向的首位置，以此来实现对结构体的访问以及修改等操作。
2. 代码编写过程中，在遍历链表时卡了一段时间，主要原因是因为先使用了系统提供的宏函数list_for_each_entry(),并在遍历过程中free()了传入的指针，之后查找了更多的资料后，发现了系统也提供了一种safe的遍历方法，即多借用一个中转指针来保存遍历指针位置，可以安全的调用free()方法且不会引起错误。

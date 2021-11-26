#!/usr/bin/python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.link import TCLink
from mininet.cli import CLI

# Mininet will assign an IP address for each interface of a node 
# automatically, but hub or switch does not need IP address.
def clearIP(n):
    for iface in n.intfList():
        n.cmd('ifconfig %s 0.0.0.0' % (iface))

class BroadcastTopo(Topo):
    def build(self):
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')
        h3 = self.addHost('h3')
        s1 = self.addHost('s1')

        self.addLink(h1, s1, bw=20)
        self.addLink(h2, s1, bw=10)
        self.addLink(h3, s1, bw=10)

if __name__ == '__main__':
    topo = BroadcastTopo()
    net = Mininet(topo = topo, link = TCLink, controller = None) 

    h1, h2, h3, s1 = net.get('h1', 'h2', 'h3', 's1')
    h1.cmd('ifconfig h1-eth0 10.0.0.1/8')
    h2.cmd('ifconfig h2-eth0 10.0.0.2/8')
    h3.cmd('ifconfig h3-eth0 10.0.0.3/8')
    clearIP(s1)

    for h in net.get('h1', 'h2', 'h3'):
        h.cmd('./disable_offloading.sh')

    for n in net.get('h1', 'h2', 'h3', 's1'):
        n.cmd('./disable_ipv6.sh')

    net.start()
    CLI(net)
    net.stop()

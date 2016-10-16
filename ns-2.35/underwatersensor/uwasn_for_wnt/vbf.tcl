set opt(chan)		                    Channel/UnderwaterChannel
set opt(prop)		                    Propagation/UnderwaterPropagation
set opt(netif)		                    Phy/UnderwaterPhy
set opt(mac)		                    Mac/UnderwaterMac/BroadcastMac
set opt(ifq)		                    Queue/DropTail/PriQueue
set opt(ll)		                        LL
set opt(energy)                         EnergyModel
set opt(txpower)                        2.0
set opt(rxpower)                        0.75
set opt(initialenergy)                  10000
set opt(idlepower)                      0.008
set opt(ant)                            Antenna/OmniAntenna
set opt(minspeed)                       0 ;#minimum speed of node
set opt(maxspeed)                       3 ;#maximum speed of node
set opt(speed)                          0.5 ;#speed of node
set opt(position_update_interval)       0.3 ;# the length of period to update node's position
set opt(packet_size)                    100 ;#50 bytes
set opt(routing_control_packet_size)    20  ;#bytes 
set opt(ifqlen)		                    50	;# max queue length in if
set opt(nn)		                        6	;# number of nodes 
set opt(x)		                        100	;# X dimension of the topography
set opt(y)	                            10  ;# Y dimension of the topography
set opt(z)                              10
set opt(seed)		                    11
set opt(stop)		                    500	;# simulation time
set opt(prestop)                        90  ;# time to prepare to stop
set opt(tr)		                        "vbf.tr"
set opt(datafile)	                    "vbf.data"
set opt(nam)                            "vbf.nam"
set opt(adhocRouting)                   Vectorbasedforward
set opt(width)                          100
set opt(interval)                       1.0
set opt(range)                          120    ;#range of each node in meters

if { $argc > 0 } {
    set opt(seed) [lindex $argv 0]
    set opt(nn) [lindex $argv 1]
    set opt(datafile) [lindex $argv 2]
}

# ==================================================================
Queue/DropTail/PriQueue set Prefer_Routing_Protocols    1
Agent/Vectorbasedforward set hop_by_hop_ 0
LL set mindelay_		                50us
LL set delay_			                25us
LL set bandwidth_		                0	;# not used
Antenna/OmniAntenna set X_              0
Antenna/OmniAntenna set Y_              0
Antenna/OmniAntenna set Z_              0.05
Antenna/OmniAntenna set Gt_             1.0
Antenna/OmniAntenna set Gr_             1.0
Mac/UnderwaterMac set bit_rate_         1.0e6 ;#1Mbps
Mac/UnderwaterMac set encoding_efficiency_ 1
Mac/UnderwaterMac/BroadcastMac set packetheader_size_ 0 ;# #of bytes
# Initialize the SharedMedia interface with parameters to make
# it work like the 914MHz Lucent WaveLAN DSSS radio interface
Phy/UnderwaterPhy set CPThresh_         10  ;#10.0
Phy/UnderwaterPhy set CSThresh_         0   ;#1.559e-11
Phy/UnderwaterPhy set RXThresh_         0   ;#3.652e-10
Phy/UnderwaterPhy set Pt_               0.2818
Phy/UnderwaterPhy set freq_             25  ;# 25khz  
Phy/UnderwaterPhy set K_                2.0 ;# spherical spreading

# ==================================================================
# Main Program
# =================================================================
set ns [new Simulator]
set topo  [new Topography]

$topo load_cubicgrid $opt(x) $opt(y) $opt(z)
set tracefd	[open $opt(tr) w]
$ns trace-all $tracefd

set nf [open $opt(nam) w]
$ns namtrace-all-wireless $nf $opt(x) $opt(y)


set data [open $opt(datafile) a]
set total_number  [expr $opt(nn)-1]
set god [create-god  $opt(nn)]
$ns at 0.0 "$god set_filename $opt(datafile)"

set chan_1 [new $opt(chan)]
global defaultRNG
$defaultRNG seed $opt(seed)
$ns node-config -adhocRouting $opt(adhocRouting) \
		         -llType $opt(ll) \
		         -macType $opt(mac) \
		         -ifqType $opt(ifq) \
		         -ifqLen $opt(ifqlen) \
		         -antType $opt(ant) \
		         -propType $opt(prop) \
		         -phyType $opt(netif) \
		         -agentTrace OFF \
                 -routerTrace OFF \
                 -macTrace ON \
                 -topoInstance $topo \
                 -energyModel $opt(energy) \
                 -txPower $opt(txpower) \
                 -rxPower $opt(rxpower) \
                 -initialEnergy $opt(initialenergy) \
                 -idlePower $opt(idlepower) \
                 -channel $chan_1
                 
set n(0) [$ns node 0]
$n(0) set sinkStatus_ 1
$god new_node $n(0)
$n(0) set X_  500
$n(0) set Y_  0
$n(0) set Z_  0
$n(0) set passive 1
set rt [$n(0) set ragent_]
$rt set control_packet_size  $opt(routing_control_packet_size)
set a(0) [new Agent/UWSink]
$ns attach-agent $n(0) $a(0)
$a(0) attach-vectorbasedforward $opt(width)
$a(0) cmd set-range $opt(range) 
$a(0) cmd set-target-x -20
$a(0) cmd set-target-y -10
$a(0) cmd set-target-z -20
$a(0) cmd set-filename $opt(datafile)
$a(0) cmd set-packetsize $opt(packet_size) ;# # of bytes

set n(1) [$ns node 1]
$n(1) set sinkStatus_ 1
$god new_node $n(1)
$n(1) set X_  440
$n(1) set Y_  0
$n(1) set Z_  0
$n(1) set passive 1
set rt [$n(1) set ragent_]
$rt set control_packet_size  $opt(routing_control_packet_size)
$n(1) set max_speed $opt(maxspeed)
$n(1) set min_speed $opt(minspeed)
$n(1) set position_update_interval_ $opt(position_update_interval)
set a(1) [new Agent/UWSink]
$ns attach-agent $n(1) $a(1)
$a(1) attach-vectorbasedforward $opt(width)
$a(1) cmd set-range $opt(range) 
$a(1) cmd set-target-x -20
$a(1) cmd set-target-y -10
$a(1) cmd set-target-z -20
$a(1) cmd set-filename $opt(datafile)
$a(1) cmd set-packetsize $opt(packet_size) ;# # of bytes

set n(2) [$ns node 2]
$n(2) set sinkStatus_ 1
$n(2) random-motion 1
$n(2) set max_speed $opt(maxspeed)
$n(2) set min_speed $opt(minspeed)
$n(2) set position_update_interval_ $opt(position_update_interval)
$god new_node $n(2)
$n(2) set X_  450
$n(2) set Y_  50
$n(2) set Z_  0
$n(2) set passive 1
set rt [$n(2) set ragent_]
$rt set control_packet_size  $opt(routing_control_packet_size)
set a(2) [new Agent/UWSink]
$ns attach-agent $n(2) $a(2)
$a(2) attach-vectorbasedforward $opt(width)
$a(2) cmd set-range $opt(range) 
$a(2) cmd set-target-x -20
$a(2) cmd set-target-y -10
$a(2) cmd set-target-z -20
$a(2) cmd set-filename $opt(datafile)
$a(2) cmd set-packetsize $opt(packet_size) ;# # of bytes


set n(3) [$ns node 3]
$n(3) set sinkStatus_ 1
$n(3) random-motion 1
$n(3) set max_speed $opt(maxspeed)
$n(3) set min_speed $opt(minspeed)
$n(3) set position_update_interval_ $opt(position_update_interval)
$god new_node $n(3)
$n(3) set X_  500
$n(3) set Y_  70
$n(3) set Z_  0
$n(3) set passive 1
set rt [$n(3) set ragent_]
$rt set control_packet_size  $opt(routing_control_packet_size)
set a(3) [new Agent/UWSink]
$ns attach-agent $n(3) $a(3)
$a(3) attach-vectorbasedforward $opt(width)
$a(3) cmd set-range $opt(range) 
$a(3) cmd set-target-x -20
$a(3) cmd set-target-y -10
$a(3) cmd set-target-z -20
$a(3) cmd set-filename $opt(datafile)
$a(3) cmd set-packetsize $opt(packet_size) ;# # of bytes

set n(4) [$ns node 4]
$n(4) set sinkStatus_ 1
$n(4) random-motion 1
$n(4) set max_speed $opt(maxspeed)
$n(4) set min_speed $opt(minspeed)
$n(4) set position_update_interval_ $opt(position_update_interval)
$god new_node $n(4)
$n(4) set X_  470
$n(4) set Y_  20
$n(4) set Z_  30
$n(4) set passive 1
set rt [$n(4) set ragent_]
$rt set control_packet_size  $opt(routing_control_packet_size)
set a(4) [new Agent/UWSink]
$ns attach-agent $n(4) $a(4)
$a(4) attach-vectorbasedforward $opt(width)
$a(4) cmd set-range $opt(range) 
$a(4) cmd set-target-x -20
$a(4) cmd set-target-y -10
$a(4) cmd set-target-z -20
$a(4) cmd set-filename $opt(datafile)
$a(4) cmd set-packetsize $opt(packet_size) ;# # of bytes

#Set the source node
set n($total_number) [$ns node $total_number]
$god new_node $n($total_number)
$n($total_number) set  sinkStatus_ 1
$n($total_number) set X_  490
$n($total_number) set Y_  10
$n($total_number) set Z_  40
$n($total_number) set-cx  200
$n($total_number) set-cy  0
$n($total_number) set-cz  0
set rt [$n($total_number) set ragent_]
$rt set control_packet_size  $opt(routing_control_packet_size)
set a($total_number) [new Agent/UWSink]
$ns attach-agent $n($total_number) $a($total_number)
$a($total_number) attach-vectorbasedforward $opt(width)
$a($total_number) cmd set-range $opt(range)
$a($total_number) cmd set-target-x 500
$a($total_number) cmd set-target-y 0
$a($total_number) cmd set-target-z 0
$a($total_number) cmd set-filename $opt(datafile)
$a($total_number) cmd set-packetsize $opt(packet_size) ;# # of bytes

set start_time 1.33
for { set i 1 } { $i<$opt(nn) } {incr i } {
	$a($i) set data_rate_ [expr 1.0/$opt(interval)]
	$ns at $start_time "$a($i) cbr-start"
	set start_time [expr $start_time+0.5 ]
	$ns at $opt(stop).001 "$a($i) terminate"
}

# make nam workable
set node_size 10
for {set k 0} { $k<$opt(nn)} {incr k} {
    $ns initial_node_pos $n($k) $node_size
}


set opt(stop2) [expr $opt(stop)+200]
puts "Node $total_number is sending first!!"

$ns at $opt(stop2).002 "$a(0) terminate"
$ns at $opt(stop2).003 "$god compute_energy"
$ns at $opt(stop2).004 "$ns nam-end-wireless $opt(stop)"
$ns at $opt(stop2).005 "puts \"NS EXISTING...\"; $ns halt"


puts $data  "New simulation...."
puts $data "nodes  = $opt(nn), maxspeed = $opt(maxspeed), minspeed = $opt(minspeed), random_seed = $opt(seed), sending_interval_=$opt(interval), width=$opt(width)"
puts $data "x= $opt(x) y= $opt(y) z= $opt(z)"
close $data
puts "starting Simulation..."
$ns run

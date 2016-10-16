#Simulation parameters setup
set val(chan)   Channel/WirelessChannel    ;# channel type
set val(prop)   Propagation/TwoRayGround   ;# radio-propagation model
set val(netif)  Phy/WirelessPhy            ;# network interface type
set val(mac)    Mac/802_11                 ;# MAC type
set val(ifq)    Queue/DropTail/PriQueue    ;# interface queue type
set val(ll)     LL                         ;# link layer type
set val(ant)    Antenna/OmniAntenna        ;# antenna model
set val(ifqlen) 100                         ;# max packet in ifq
set val(nn)     4                          ;# number of mobilenodes
set val(rp)     AODV                       ;# routing protocol
set val(x)      1000                      ;# X dimension of topography
set val(y)      500                      ;# Y dimension of topography
set val(stop)   100.0                         ;# time of simulation end
Agent/AODV set sufix_ 1
#Initialization
set ns [new Simulator]
set topo       [new Topography]
$topo load_flatgrid $val(x) $val(y)
create-god $val(nn)
set tracefile [open blackhole.tr w]
$ns trace-all $tracefile
set namfile [open blackhole.nam w]
$ns namtrace-all $namfile
$ns namtrace-all-wireless $namfile $val(x) $val(y)

#Mobile node parameter setup
set chan [new $val(chan)]
$ns node-config -adhocRouting  $val(rp) \
                -llType        $val(ll) \
                -macType       $val(mac) \
                -ifqType       $val(ifq) \
                -ifqLen        $val(ifqlen) \
                -antType       $val(ant) \
                -propType      $val(prop) \
                -phyType       $val(netif) \
                -channel       $chan \
                -topoInstance  $topo \
                -agentTrace    ON \
                -routerTrace   ON \
                -macTrace      OFF \
                -movementTrace OFF

#Nodes Definition        
set n0 [$ns node]
$n0 set X_ 200
$n0 set Y_ 250
$n0 set Z_ 0.0
$ns initial_node_pos $n0 40
set n1 [$ns node]
$n1 set X_ 400
$n1 set Y_ 250
$n1 set Z_ 0.0
$ns initial_node_pos $n1 40
set n2 [$ns node]
$n2 set X_ 600
$n2 set Y_ 250
$n2 set Z_ 0.0
$ns initial_node_pos $n2 40
set n3 [$ns node]
$n3 set X_ 800
$n3 set Y_ 250
$n3 set Z_ 0.0
$ns initial_node_pos $n3 40
set n4 [$ns node]
$n4 set X_ 500
$n4 set Y_ 200
$n4 set Z_ 0.0
$ns initial_node_pos $n4 40
set n5 [$ns node]
$n4 set X_ 500
$n4 set Y_ 300
$n4 set Z_ 0.0
$ns initial_node_pos $n4 40
set n6 [$ns node]
$n4 set X_ 350
$n4 set Y_ 200
$n4 set Z_ 0.0
$ns initial_node_pos $n4 40
set n7 [$ns node]
$n4 set X_ 350
$n4 set Y_ 300
$n4 set Z_ 0.0
$ns initial_node_pos $n4 40

set tapagent [$n4 agent 255]
$tapagent install-tap [$n4 set mac_(0)]
$ns at 0.0 "$n4 label \"Watchdog Node\""

$ns at 20.0 "[$n1 set ragent_] blackhole"
$ns at 20.0 "$n1 label \"Blackhole Node\""

#UDP Flow
set udp0 [new Agent/UDP]
$ns attach-agent $n0 $udp0
set null1 [new Agent/Null]
$ns attach-agent $n3 $null1
$ns connect $udp0 $null1
$udp0 set packetSize_ 1500
set cbr0 [new Application/Traffic/CBR]
$cbr0 attach-agent $udp0
$cbr0 set packetSize_ 1000
$cbr0 set rate_ 1Mb
$cbr0 set random_ null
$ns at 1.0 "$cbr0 start"
$ns at $val(stop) "$cbr0 stop"

#Define a 'finish' procedure
proc finish {} {
    global ns tracefile namfile
    $ns flush-trace
    close $tracefile
    close $namfile
    exec nam blackhole.nam &
    exit 0
}
for {set i 0} {$i < $val(nn) } { incr i } {
    $ns at $val(stop) "\$n$i reset"
}
$ns at $val(stop) "$ns nam-end-wireless $val(stop)"
$ns at $val(stop) "finish"
$ns at $val(stop) "puts \"done\" ; $ns halt"
$ns run

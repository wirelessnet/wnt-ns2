#                          fsr.tcl  -  description
#                             -------------------
#    begin                : Thu Jul 29 2004
#    copyright            : (C) 2004 by Sven Jaap
#    email                : jaap@ibr.cs.tu-bs.de
# ***************************************************************************/
#
#/***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# ***************************************************************************/
#
set opt(ragent)		Agent/FSR
set opt(pos)		NONE			;# Box or NONE


Agent/FSR set sport_        0
Agent/FSR set dport_        0


if { $opt(pos) == "Box" } {
	puts "*** FSR using Box configuration..."
}

# ======================================================================
Agent instproc init args {
        eval $self next $args
}

Agent/FSR instproc init args {
        eval $self next $args
}

# ===== Get rid of the warnings in bind ================================

# ======================================================================

proc create-fsr-routing-agent { node id } {
    puts "Create routing agent\n"
    global ns_ ragent_ tracefd opt

    #
    #  Create the Routing Agent and attach it to port 255.
    #
    set ragent_($id) [new $opt(ragent) $id]
    set ragent $ragent_($id)
    $node attach $ragent 255

    $ragent if-queue [$node set ifq(0)] 	;# ifq between LL and MAC
    $ns_ at 0.$id "$ragent_($id) start-fsr"	;# start updates

    #
    # Drop Target (always on regardless of other tracing)
    #
    set drpT [cmu-trace Drop "RTR" $node]
    $ragent drop-target $drpT

    #
    # Log Target
    #
    set T [new Trace/Generic]
    $T target [$ns_ set nullAgent_]
    $T attach $tracefd
    $T set src_ $id
    $ragent tracetarget $T
}


proc fsr-create-mobile-node { id args } {
	puts "Create mobile node\n"
	global ns ns_ chan prop topo tracefd opt node_
	global chan prop tracefd topo opt

	set node_($id) [MobileNode]

	set node $node_($id)
	$node random-motion 0		;# disable random motion
	$node topography $topo

	#
	# This Trace Target is used to log changes in direction
	# and velocity for the mobile node.
	#
	set T [new Trace/Generic]
	$T target [$ns_ set nullAgent_]
	$T attach $tracefd
	$T set src_ $id
	$node log-target $T

	$node add-interface $chan $prop $opt(ll) $opt(mac)	\
	    $opt(ifq) $opt(ifqlen) $opt(netif) $opt(ant)
	#
	# Create a Routing Agent for the Node
	#
	create-$opt(rp)-routing-agent $node $id

	if { $opt(pos) == "Box" } {
		#
		# Box Configuration
		#
		set spacing 200
		set maxrow 3
		set col [expr ($id - 1) % $maxrow]
		set row [expr ($id - 1) / $maxrow]
		$node set X_ [expr $col * $spacing]
		$node set Y_ [expr $row * $spacing]
		$node set Z_ 0.0
		$node set speed_ 0.0

		$ns_ at 0.0 "$node_($id) start"

	} elseif { $opt(pos) == "Random" }{

		$node random-motion 1

		$ns_ at 0.0 "$node_($id) start"

	}
}

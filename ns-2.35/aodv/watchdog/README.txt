How to install the watchdog on NS-2.

- Follow the instructions for install the TAP in AODV (http://www.cse.msu.edu/~wangbo1/ns2/index.html)
- Add to the file ns-2.34/tcl/lib/ns-mobilenode.tcl the following code:

			#Blackhole
			Node/MobileNode instproc set_Blackhole {} {
				$self instvar ragent_
				puts "Installing new blackhole -------------"
				return  $ragent_
			}
			
- Copy the watchdog's folder into the AODV folder (located in ns-2.34 folder).
- Change the makefile into the ns-2.34 to compile the watchdog objects.

The folder aodv attached is an example of how to use it the watchdog. 
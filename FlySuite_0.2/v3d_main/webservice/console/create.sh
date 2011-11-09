#!/bin/bash

# Clean up from previous generation
rm *.cpp *.h *.nsmap

# Parse WSDL
wsdl2h -t typemap.dat -o obsHeader.h obs.wsdl 
wsdl2h -t typemap.dat -o cdsHeader.h cds.wsdl 

# Rename services
cat obsHeader.h | sed 's/ObsPortBinding/ConsoleObserver/' > obsModHeader.h
cat cdsHeader.h | sed 's/CdsPortBinding/ConsoleDataService/' > cdsModHeader.h

# Generate C++ code
soapcpp2 -penv ../impl/env.h
soapcpp2 -iSx -I$HOME/share/gsoap/import -qobs obsModHeader.h
soapcpp2 -iCx -I$HOME/share/gsoap/import -qcds cdsModHeader.h

# Fix stdsoap2 imports
for f in *Stub.h; do 
    echo "Fixing $f...";
    cat $f | sed 's/stdsoap2/..\/gsoap2\/stdsoap2/' > $f.new
    mv $f.new $f
done;


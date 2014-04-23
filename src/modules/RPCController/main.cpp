/* 
 * Copyright (C) 2014 Francesco Giovannini, iCub Facility - Istituto Italiano di Tecnologia
 * Authors: Francesco Giovannini
 * email:   francesco.giovannini@iit.it
 * website: www.robotcub.org 
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */



#include <iostream>

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>

#include "RPCControllerModule.h"

using iCub::RPCController::RPCControllerModule;

using std::cerr;

using yarp::os::Network;
using yarp::os::ResourceFinder;

int main(int argc, char *argv[]) {
    // Open network
    Network yarp;
    if (!yarp.checkNetwork()) {
        cerr << "Error: yarp server is not available. \n";
        return -1;
    }     

    // Using modules
    RPCControllerModule mod;

    // Create resource finder
    ResourceFinder rf;
    rf.setVerbose();
    rf.setDefaultConfigFile("confRPCController.ini");
    rf.setDefaultContext("RPCController");
    rf.configure("ICUB_ROOT", argc, argv);

    // Configure and run module
    mod.runModule(rf);

    return 0;
}

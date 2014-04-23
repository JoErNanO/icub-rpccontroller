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



#include "RPCControllerModule.h"

#include <iostream>

#include <yarp/os/Network.h>
#include <yarp/os/Time.h>

using iCub::RPCController::RPCControllerModule;

using std::cerr;
using std::cout;

using yarp::os::ResourceFinder;
using yarp::os::Value;
using yarp::os::Bottle;


/* *********************************************************************************************************************** */
/* ******* Constructor                                                      ********************************************** */   
RPCControllerModule::RPCControllerModule() : RFModule() {
    dbgTag = "RPCControllerModule: ";

    stepCounter = 0;
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Destructor                                                       ********************************************** */   
RPCControllerModule::~RPCControllerModule() {}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Get Period                                                       ********************************************** */   
double RPCControllerModule::getPeriod() { return period; }
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Configure module                                                 ********************************************** */   
bool RPCControllerModule::configure(ResourceFinder &rf){
    using std::string;
    using yarp::os::Network;

    cout << dbgTag << "Starting. \n";

    /* ****** Configure the Module                            ****** */
    // Get resource finder and extract properties
    moduleName = rf.check("name", Value("RPCController"), "The module name.").asString().c_str();
    period = rf.check("period", 1.0).asDouble();
    robotName = rf.check("robot", Value("icub"), "The robot name.").asString().c_str();
    string portNameRoot = "/" + moduleName + "/";


    /* ******* Ports.                                       ******* */
    rpcModule.open((portNameRoot + "cmd:io").c_str());
    portExperimentStepsOut.open((portNameRoot + "experiment/status:o").c_str());
    
    
    /* ******* Read RPC port parameters.                    ******* */
    Bottle parGroup = rf.findGroup("rpc");
    if (!parGroup.isNull()) {
        if (parGroup.check("rpcServer")) {
            string rpcServer = parGroup.find("rpcServer").asString().c_str();
            // Connect to RPC server
            if (!Network::connect(rpcModule.getName(), rpcServer)) {
                cerr << dbgTag << "Could not connect to the specified RPC server: " << rpcServer << ". \n";
                return false;
            }
        } else {
            cerr << dbgTag << "Could not find the rpcServer name in the specified configuration file. \n";
            return false;
        }
    } else {
        cerr << dbgTag << "Could not find the [rpc] parameter group in the specified configuration file. \n";
        return false;
    }

    /* ******* Read experiment parameters.                  ******* */
    parGroup = rf.findGroup("experiment");
    if (!parGroup.isNull()) {
        // Number of columns in array
        if (parGroup.check("nCols")) {
            int nCols = parGroup.find("nCols").asInt();

            // Experiment parameters
            Bottle *confExpParams = parGroup.find("cmdT").asList();
            if (confExpParams != NULL) {
                for (int i = 0; i < confExpParams->size(); ++i) {
                    ExperimentParams tmpPar;
                    tmpPar.cmd = confExpParams->get(i).asString().c_str();
                    tmpPar.time = confExpParams->get(i + 1).asDouble();
                    expParams.push_back(tmpPar);

                    i++;
                }
            } else {
                cerr << dbgTag << "Could not find the experiment parameter list in the supplied configuration file. \n";
                return false;
            }
        } else {
            cerr << dbgTag << "Could not find nCols parameter in supplied configuration file. \n";
            return false;
        }
    } else {
        cerr << dbgTag << "Could not find the experiment parameter group [experiment] in the supplied configuration file. \n";
        return false;
    }

#ifndef NODEBUG
    cout << "DEBUG: " << dbgTag << "Experiment parameters are: \n";
    for (size_t i = 0; i < expParams.size(); ++i) {
        cout << "DEBUG " << dbgTag << expParams[i].cmd << " " << expParams[i].time << "\n";
        i++;
    }
#endif


    
    cout << dbgTag << "Started correctly. \n";

    return true;
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Update    module                                                 ********************************************** */   
bool RPCControllerModule::updateModule() {
    using yarp::os::Time;

    if (stepCounter == 0) {
        cout << dbgTag << "Running experiment. \n\n";

        // Connect datadumper
        connectDataDumper();
    } 
    
    if (stepCounter < expParams.size()) {
        // Run all experiment steps
        cout << dbgTag << "Running experiment step n. " << stepCounter << "\n";
        cout << dbgTag << "\t Sending command: " << expParams[stepCounter].cmd << ".\n";

        // Send command
        Bottle cmd, reply;
        cmd.addString(expParams[stepCounter].cmd);
        rpcModule.write(cmd, reply);
        cout << dbgTag << "\t\t" << reply.toString() << "\n";
        // Delay for position to be reached
        if (expParams[stepCounter].cmd == "grasp") {
            Time::delay(2);
        }

        // Wait for time
        double timeStart = Time::now();
        Time::delay(expParams[stepCounter].time);
        double timeEnd = Time::now();

        cout << dbgTag << "\t Done. Elapsed time: " << (timeEnd - timeStart)  << "\n";

        // Write out experiment status
        writeExperimentStatus(timeStart, timeEnd);
        
        // Increment step counter
        stepCounter++;
    } else {
        cout << "\n";
        cout << dbgTag << "Experiment completed. \n";

        // Disconnect datadumper
        disconnectDataDumper();

        return false;
    }

    return true; 
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Interrupt module                                                 ********************************************** */   
bool RPCControllerModule::interruptModule() {
    cout << dbgTag << "Interrupting. \n";
    
    // Interrupt ports
    rpcModule.interrupt();
    portExperimentStepsOut.interrupt();

    cout << dbgTag << "Interrupted correctly. \n";

    return true;
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Close module                                                     ********************************************** */   
bool RPCControllerModule::close() {
    cout << dbgTag << "Closing. \n";
    
    // Close ports
    rpcModule.close();
    portExperimentStepsOut.close();

    cout << dbgTag << "Closed. \n";
    
    return true;
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Respond to rpc calls                                             ********************************************** */   
bool RPCControllerModule::respond(const Bottle &command, Bottle &reply) {

    return true;
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Write the experiment status to the port. ********************************************** */
bool RPCControllerModule::writeExperimentStatus(const double &i_timeStart, const double &i_timeEnd) {
    using yarp::sig::Vector;

    Vector &out = portExperimentStepsOut.prepare();
    portStamp.update();

    // Write step counter, tStart ant tEnd
    out.clear();
    out.push_back(stepCounter);
    out.push_back(i_timeStart);
    out.push_back(i_timeEnd);

    // Write data
    portExperimentStepsOut.setEnvelope(portStamp);
    portExperimentStepsOut.write();

    return true;
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Connect the data dumper.                                         ********************************************** */
bool RPCControllerModule::connectDataDumper(void) {
    using yarp::os::Network;

    bool ok = true;
    
    // Connect data dumper
#ifndef NODEBUG
    cout << "DEBUG: " << dbgTag << "Connecting datadumpers. \n";
#endif

    ok &= Network::connect("/" + robotName + "/right_arm/state:o", "/dump_right_pos");
    ok &= Network::connect("/NIDAQmxReader/data/real:o", "/dump_right_nano17");
    ok &= Network::connect(portExperimentStepsOut.getName(), "/dump_right_exp");
    ok &= Network::connect("/" + robotName + "/skin/right_hand", "/dump_right_skin_raw");
    ok &= Network::connect("/" + robotName + "/skin/right_hand_comp", "/dump_right_skin_comp");

    return ok;
}
/* *********************************************************************************************************************** */


/* *********************************************************************************************************************** */
/* ******* Disconnect the data dumper.                                      ********************************************** */
bool RPCControllerModule::disconnectDataDumper(void) {
    using yarp::os::Network;
    
    bool ok = true;

    // Disconnect data dumper
#ifndef NODEBUG
    cout << "DEBUG: " << dbgTag << "Disconnecting datadumpers. \n";
#endif

    ok &= Network::disconnect("/" + robotName + "/right_arm/state:o", "/dump_right_pos");
    ok &= Network::disconnect("/NIDAQmxReader/data/real:o", "/dump_right_nano17");
    ok &= Network::disconnect(portExperimentStepsOut.getName(), "/dump_right_exp");
    ok &= Network::disconnect("/" + robotName + "/skin/right_hand", "/dump_right_skin_raw");
    ok &= Network::disconnect("/" + robotName + "/skin/right_hand_comp", "/dump_right_skin_comp");

    return ok;
}
/* *********************************************************************************************************************** */

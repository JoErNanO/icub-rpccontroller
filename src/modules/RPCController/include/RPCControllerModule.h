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



/**
 * @ingroup icub_module
 * \defgroup icub_MyModule RPCControllerModule
 * 
 * The RPCControllerModule is a
 * 
 * \section intro_sec Description
 * Description here...
 * 
 * 
 * \section lib_sec Libraries
 * YARP
 * 
 * 
 * \section parameters_sec Parameters
 * <b>Command-line Parameters</b> 
 * <b>Configuration File Parameters </b>
 *  
 * 
 * \section portsa_sec Ports Accessed
 * 
 * 
 * \section portsc_sec Ports Created
 * <b>Output ports </b>
 * <b>Input ports</b>
 * 
 * 
 * \section in_files_sec Input Data Files
 * 
 * 
 * \section out_data_sec Output Data Files
 * 
 *  
 * \section conf_file_sec Configuration Files
 * 
 * 
 * \section tested_os_sec Tested OS
 * 
 * 
 * \section example_sec Example Instantiation of the Module
 * 
 * 
 * \author Francesco Giovannini (francesco.giovannini@iit.it)
 * 
 * Copyright (C) 2014 Francesco Giovannini, iCub Facility - Istituto Italiano di Tecnologia
 * 
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 * 
 * This file can be edited at .... .
 */

#ifndef __RPCCONTROLLER_MODULE_H__
#define __RPCCONTROLLER_MODULE_H__

#include <string>
#include <vector>

#include <yarp/os/RFModule.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Stamp.h>
#include <yarp/sig/Vector.h>

namespace iCub {
    namespace RPCController {
        struct ExperimentParams {
            std::string cmd;
            double time;
        };

        class RPCControllerModule : public yarp::os::RFModule {
            private:
                /* ****** Module attributes                             ****** */
                /** The module period. */
                double period;

                std::string moduleName;
                std::string robotName;

                /** The experiment parameters. */
                std::vector<iCub::RPCController::ExperimentParams> expParams;

                /** The step counter. */
                int stepCounter;
         
                /* ****** Ports                                         ****** */
                /** The port timestamp. */
                yarp::os::Stamp portStamp;
                yarp::os::RpcClient rpcModule;
                yarp::os::BufferedPort<yarp::sig::Vector> portExperimentStepsOut;

         
                /* ****** Debug attributes                              ****** */
                std::string dbgTag;

            public:
                /**
                 * Default constructor.
                 */
                RPCControllerModule();
                virtual ~RPCControllerModule();
                virtual double getPeriod();
                virtual bool configure(yarp::os::ResourceFinder &rf);
                virtual bool updateModule();
                virtual bool interruptModule();
                virtual bool close();
                virtual bool respond(const yarp::os::Bottle &command, yarp::os::Bottle &reply);

            private:
                /**
                 * Output the experiment status on the dedicated port.
                 *
                 * \param \i_timeStart The start time of the current experiment step
                 * \param \i_timeEnd The end time of the current experiment step
                 *
                 * \return True upon success
                 */
                bool writeExperimentStatus(const double &i_timeStart, const double &i_timeEnd);

                bool connectDataDumper(void);
                bool disconnectDataDumper(void);
        };
    }
}

#endif


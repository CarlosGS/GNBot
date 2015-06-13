/** \file
  Copyright (c) CSIRO ICT Robotics Centre
  \brief  View Video Stream using SDL - faster for Colour video 
  \author Cedric.Pradalier@csiro.au
  \warning This program requires SDL
 */

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <image_transport/image_transport.h>

#include <dynamic_reconfigure/server.h>
#include <canon_vbc50i/CanonParamsConfig.h>
#include <canon_vbc50i/PTZ.h>


#include <string>
#include <vector>
#include "canon_vbc50i/libCanon/JpegReader.h"
#include "canon_vbc50i/libCanon/CanonDriver.h"
#include "canon_vbc50i/libCanon/XString.h"
// #include "canon_vbc50i/libCanon/CanonProjection.h"
using namespace std;

#if 0
#warning Saving path specific to eddie-ph
#define RECORDING_DEST "/scratch/stream"
#else
#define RECORDING_DEST "/tmp"
#endif

canon_vbc50i::CanonParamsConfig currentConfig;
boost::recursive_mutex param_mutex;
CanonDriver * driver = NULL;

void received_frame_callback(void * arg, const JpegReader * jpeg, unsigned int frame)
{

#if 1
    image_transport::Publisher *pub = (image_transport::Publisher*)arg;
    sensor_msgs::Image output;
	assert (jpeg->getOutputColorSpace() == JpegReader::cmRGB);
	frame += 1;
    output.width = jpeg->width;
    output.height = jpeg->height;
    output.header.stamp = ros::Time::now();
    output.header.seq = frame;
    output.header.frame_id = currentConfig.frame_id;
    output.encoding = "rgb8";
    output.is_bigendian = 0;
    output.step = output.width*3;
    output.data.resize(3*jpeg->width*jpeg->height);
    std::copy(jpeg->buffer,jpeg->buffer+3*jpeg->width*jpeg->height,output.data.begin());
    pub->publish(output);
#endif
}

void callback(canon_vbc50i::CanonParamsConfig &config, uint32_t level)
{
    if (config.infrared != currentConfig.infrared) {
		driver->setInfraRed(config.infrared);
    }
    if (config.night_mode != currentConfig.night_mode) {
		driver->setNightMode(config.night_mode);
    }
    if (config.fps != currentConfig.fps) {
		driver->setFrameRate(config.fps);
    }
    bool pos_change = false;
    if (config.pan_ang != currentConfig.pan_ang) {
        pos_change = true;
    }
    if (config.tilt_ang != currentConfig.tilt_ang) {
        pos_change = true;
    }
    if (config.zoom_ang != currentConfig.zoom_ang) {
        pos_change = true;
    }
	if (config.pan_speed != currentConfig.pan_speed) 
		driver->setPanSpeed(config.pan_speed);
	if (config.tilt_speed != currentConfig.tilt_speed) 
		driver->setTiltSpeed(config.tilt_speed);
	if (config.zoom_speed != currentConfig.zoom_speed) 
		driver->setZoomSpeed(config.zoom_speed);

    if (pos_change) {
        driver->moveTo(config.pan_ang,config.tilt_ang,config.zoom_ang);
    }
#if 0

	// if ((local.width != buffer.width) || 
	// 		(local.height != buffer.height))
	// 	driver->setImageSize(local.width,local.height);

	if (config.focus_mode != currentConfig.focus_mode) {
        switch(config.focus_mode) {
            case 0:
                driver->setFocusMode(CanonDriver::FMAuto, CanonDriver::FMUndef);
                break;
            case 1:
                driver->setFocusMode(CanonDriver::FMManual, CanonDriver::FMManualFar);
                break;
            case 2:
                driver->setFocusMode(CanonDriver::FMManual, CanonDriver::FMManualNear);
                break;
            case 3:
                driver->setFocusMode(CanonDriver::FMAutoDomes, CanonDriver::FMUndef);
                break;
            case 4:
                driver->setFocusMode(CanonDriver::FMInfinity, CanonDriver::FMUndef);
                break;
        }
	}

    if (config.autoexp != currentConfig.autoexp) {
        driver->setAutoExposure(CanonDriver::AXPmode1);
        printf("Switched to auto exposure\n");
	} else if ((config.aperture != currentConfig.aperture) ||
				(config.inv_shutter != currentConfig.inv_shutter) ||
				(config.gain != currentConfig.gain)) {
        driver->setExposureParameters(config.aperture,
                config.inv_shutter,config.gain);
        printf("Changed exposure parameters\n");
	}

	if (config.digital_zoom != currentConfig.digital_zoom) {
		driver->setDigitalZoom(config.digital_zoom);
	}
	if (config.pause != currentConfig.pause) {
		driver->setVideoReceptionPauseStatus(config.pause);
	}
	if (config.record != currentConfig.record) {
        driver->setRecordingDestination(config.record_dir.c_str());
		driver->setVideoRecordingMode(config.record);
		if (config.record) {
			fprintf(stderr,"Warning: video recording started\n");
		}
	}
    currentConfig = config;
#endif
}





int main(int argc, char* argv[]) 
{
	if (argc < 2) {
		return 1;
	}

    currentConfig.hostname = argv[1];
    currentConfig.subsampling = 0;
    currentConfig.fps = 10;

    ros::init(argc, argv, "canon_vbc50i");
    ros::NodeHandle nh("~");
    ros::Publisher ptzpub = nh.advertise<canon_vbc50i::PTZ>("ptz", 1);
    image_transport::ImageTransport it(nh);
    image_transport::Publisher image_pub = it.advertise("image",1);


	char* loginf = NULL;
	if (argc >= 3) {
		loginf = argv[2];
	}
	try {
        double p,t,z;
        unsigned int width = 768, height = 576;
		/**** Initialising Canon Driver *****/
		CanonDriver canon(loginf,currentConfig.hostname.c_str());
        driver = &canon;
		if (!canon.connect()) {
			return 1;
		}

		canon.setVideoOutputColorSpace(JpegReader::cmRGB);
        width >>= currentConfig.subsampling;
        height >>= currentConfig.subsampling;
		// canon.setImageSize(currentConfig.width,currentConfig.height);
		canon.setFrameRate(currentConfig.fps);
		canon.startVideoReception(received_frame_callback,&image_pub);
        canon.getCurrentPos(&p,&t,&z);
        currentConfig.pan_ang = p;
        currentConfig.tilt_ang = t;
        currentConfig.zoom_ang = z;
        ros::Rate loop_rate(10);

        dynamic_reconfigure::Server<canon_vbc50i::CanonParamsConfig> srv(param_mutex,nh);
        dynamic_reconfigure::Server<canon_vbc50i::CanonParamsConfig>::CallbackType f = boost::bind(&callback, _1, _2);
        srv.setCallback(f);
        srv.updateConfig(currentConfig);


        while (ros::ok()) {
            canon_vbc50i::PTZ ptz;
            canon.getCurrentPos(&p,&t,&z);
            ptz.pan = p;
            ptz.tilt = t;
            ptz.zoom = z;
            ptzpub.publish(ptz);

            ros::spinOnce();
            loop_rate.sleep();
        }

		canon.disconnect();
        driver = NULL;

	} catch (const std::exception & e) {
		fprintf(stderr,"Failed to connect to '%s'\n",argv[1]);
		fprintf(stderr,"\t%s\n",e.what());
	}

	printf("\n");
	return 0;
}

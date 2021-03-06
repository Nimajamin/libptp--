/**
 * @file libptp++.hpp
 * 
 * @brief A conversion of pyptp2 and all that comes with it to C++
 * 
 * libptp2 is nice, but appears to be tightly bound to ptpcam.  There
 * are a few other CHDK-specific programs to communicate with a camera
 * through PTP, but all contain source code that is tightly integrated
 * and difficult to read.
 * 
 * While this library should be able to communicate with any PTP camera
 * through the \c PTPCamera interface, it's primary purpose is to allow
 * easy communication with cameras running CHDK through \c CHDKCamera.
 * 
 * This library has two goals:
 *  -# Provide all functionality of pyptp2 through a C++ interface.
 *  -# Be easy to use, and well-documented.
 *  
 * @author Bobby Graese <bobby.graese@gmail.com>
 * 
 * @see http://code.google.com/p/pyptp2/
 * @see http://libptp.sourceforge.net/
 * 
 * @version 0.1
 */
 
/**
 * \mainpage libptp++ API Reference
 * 
 * \section intro Introduction
 *
 * libptp++ is an open-source C++ library for communicating with PTP devices in
 * the easiest way possible.  It is a port and extension of the pyptp2 API to
 * C++.  This API was chosen to avoid designing an API from the ground up, and
 * because it seems to be fairly stable and useful.  However, the pyptp2 library
 * was able to use some Python conveniences that aren't available in C++, so
 * some additions have been made to this API.
 *
 * This library does not assume that the developer knows anyting about PTP, or
 * how it handles its transactions over USB.  Instead, all these functions are
 * abstracted out to library functions, and these library functions attempt to
 * hide the underlying USB interface as much as possible.  In some cases, it is
 * simply not feasibly to hide this interface, so it is exposed to the
 * developer.
 *
 * \section gettingstarted Getting Started
 *
 * If you learn best by reading documentation, head over to the "Classes" page,
 * and you can read an overview of what classes are available, as well as what
 * functionality each of their methods provide.
 *
 * However, most likely, the "Examples" page will be more useful.  This library
 * is designed so that a lot can be accomplished in as few calls as possible, so
 * the examples should help get you a quick start to using libptp++.
 */
 
/**
 * \page examples Examples
 *
 * Note: this page is mostly incomplete. More examples coming in the future.
 *
 * \section simple A Simple Example
 *
 * This example simply finds the first PTP camera available, connects to it,
 * and asks CHDK to put the camera in "record" mode.
\code
libusb_init(NULL);  // Make sure to initialize libusb first!

libusb_device * dev = CHDKCamera::find_first_camera();

CHDKCamera cam; 
cam.open(dev);

// Execute a lua script to switch the camera to "Record" mode.
//  Second parameter, error_code, is NULL, because we don't care if an error
//  occurs, and we aren't blocking to wait for one.
cam.execute_lua("switch_mode_usb(1)", NULL);

// The camera is closed automatically when the cam object is destroyed

// Be sure to exit libusb
libusb_exit(NULL);
\endcode
 *
 */
 
#ifndef LIBPTP_PP_H_
#define LIBPTP_PP_H_

#include <libusb-1.0/libusb.h>

// This serves as a global "include" file -- include this to grab all the other
//  headers, too
#include "CameraBase.hpp"
#include "CHDKCamera.hpp"
#include "LVData.hpp"
#include "PTPCamera.hpp"
#include "PTPContainer.hpp"

namespace PTP {

// Force these definitions into the PTP namespace
#include "chdk/live_view.h"
#include "chdk/ptp.h"

    enum LIBPTP_PP_ERRORS {
        ERR_NONE = 0,
        ERR_CANNOT_CONNECT,
        ERR_NO_DEVICE,
        ERR_ALREADY_OPEN,
        ERR_NOT_OPEN,
        ERR_CANNOT_RECV,
        ERR_TIMEOUT,
        ERR_INVALID_RESPONSE,
        ERR_NOT_IMPLEMENTED,
        
        ERR_PTPCONTAINER_NO_PAYLOAD,
        ERR_PTPCONTAINER_INVALID_PARAM,
        
        ERR_LVDATA_NOT_ENOUGH_DATA
    };
    
    // Picked out of CHDK source in a header we don't want to include
    enum CHDK_PTP_RESP {
        CHDK_PTP_RC_OK = 0x2001,
        CHDK_PTP_RC_GeneralError = 0x2002,
        CHDK_PTP_RC_ParameterNotSupported = 0x2006,
        CHDK_PTP_RC_InvalidParameter = 0x201D
    };

}

#endif /* LIBPTP_PP_H_ */

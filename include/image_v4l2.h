/*
 * Copyright (C) 2017 Emmanuel Durand
 *
 * This file is part of Splash.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Splash is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Splash.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @image.h
 * The Image_V4L2 class
 */

#ifndef SPLASH_IMAGE_V4L2_H
#define SPLASH_IMAGE_V4L2_H

#include "config.h"
#include "image.h"

#include <deque>
#include <linux/videodev2.h>

namespace Splash
{

class Image_V4L2 : public Image
{
  public:
    /**
     * \brief Constructor
     * \param root Root object
     */
    Image_V4L2(std::weak_ptr<RootObject> root);

    /**
     * \brief Destructor
     */
    ~Image_V4L2();

    /**
     * No copy constructor, but a copy operator
     */
    Image_V4L2(const Image_V4L2&) = delete;
    Image_V4L2& operator=(const Image_V4L2&) = delete;
    Image_V4L2& operator=(Image_V4L2&&) = default;

  private:
    std::string _devicePath{"/dev/video0"};
    std::string _controlDevicePath{"/dev/video63"};

    // Parameters to send to the shader
    std::unordered_map<std::string, Values> _shaderUniforms;

    // File descriptors
    int _controlFd{-1};
    int _deviceFd{-1};

    // V4L2 stuff
    bool _capabilitiesEnumerated{false}; // Only enumerate capabilities once for each device
    bool _hasStreamingIO{false};
    struct v4l2_capability _v4l2Capability; //!< The video4linux capabilities structure

    int _v4l2InputCount{0};
    std::vector<struct v4l2_input> _v4l2Inputs{};

    int _v4l2StandardCount{0};
    std::vector<struct v4l2_standard> _v4l2Standards{};

    int _v4l2FormatCount{0};
    std::vector<struct v4l2_fmtdesc> _v4l2Formats{};

    struct v4l2_format _v4l2Format;
    struct v4l2_format _v4l2SourceFormat;
    struct v4l2_streamparm _v4l2StreamParams;

    // Datapath specific variables
    bool _isDatapath{false};
    bool _autosetResolution{true};

    // Capture parameters
    int _v4l2Index{0};
    int _outputWidth{1920};
    int _outputHeight{1080};
    uint32_t _outputPixelFormat{V4L2_PIX_FMT_RGB24};
    std::string _sourceFormatAsString{""};

    struct v4l2_requestbuffers _v4l2RequestBuffers;
    int _bufferCount{3};
    std::deque<std::unique_ptr<ImageBuffer>> _imageBuffers{};

    bool _capturing{false};        //!< True if currently capturing frames
    bool _captureThreadRun{false}; //!< Set to false to stop the capture thread
    bool _startCapturing{false};
    bool _stopCapturing{false};

    ImageBufferSpec _spec{};

    std::thread _captureThread{};

    /**
     * Capture thread function
     */
    void captureThreadFunc();

    /**
     * \brief As the name suggests
     */
    void init();

    /**
     * Initialize V4L2 userptr capture mode
     * \return Return true if all went well
     */
    bool initializeUserPtrCapture();

    /**
     * Initialize the capture
     * \return Return true if everything is OK
     */
    bool initializeCapture();

#if HAVE_DATAPATH
    /**
     * Open the control device
     * \return Return true if everything is OK
     */
    bool openControlDevice();

    /**
     * Close the control device
     */
    void closeControlDevice();
#endif

    /**
     * Open the capture device
     * \param devicePath Path to the device
     * \return Return true if the device has been successfully opened
     */
    bool openCaptureDevice(const std::string& devicePath);

    /**
     * Close the capture device
     */
    void closeCaptureDevice();

    /**
     * Enumerate capture device inputs
     * \return Return true if everything went OK
     */
    bool enumerateCaptureDeviceInputs();

    /**
     * Enumerate capture formats
     * \return Return true if everything went OK
     */
    bool enumerateCaptureFormats();

    /**
     * Enumerate video standards
     * \return Return true if everything went OK
     */
    bool enumerateVideoStandards();

    /**
     * Do capture from the device
     * \return Return true if the capture has been launched successfully
     */
    bool doCapture();

    /**
     * Stop capture from the device
     */
    void stopCapture();

    /**
     * \brief Register new functors to modify attributes
     */
    void registerAttributes();
};

} // end of namespace

#endif // SPLASH_IMAGE_V4L2_H

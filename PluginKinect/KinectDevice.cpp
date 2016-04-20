/***********************************************************************
* Module:  KinectDevice.cpp
* Author:  Tine
* Purpose: Implementation of the class KinectDevice
***********************************************************************/

#include <string>
#include "KinectDevice.h"
#include <Windows.h>

// Constructor, creates musicianBodies, calls initSensor() and creates thread for run()
ttmm::KinectDevice::KinectDevice(std::vector<KinectMusician> &musicians)
    : musicians(musicians)
{
    for (auto &body : musicianBodies)
    {
        body = nullptr;
    }

    // try to start sensor; success if deviceHandle is not nullptr
    (void)initSensor();

    if (deviceHandle != nullptr)
    {
        deviceThread = std::thread(&KinectDevice::run, this);
        if (!deviceThread.joinable())
        {
            ttmm::logger.write("Failed to start the KinectThread");
        } 
		else 
		{
			open = true;
		}
    }

	// execute visualisation plugin
	//std::string executable("Bunnystomp.exe");
	//LPWSTR ws = new wchar_t[executable.size() + 1]; 
	//std::copy(executable.begin(), executable.end(), ws);
	//ws[executable.size()] = 0; 
	//startup(ws);
}

// helper function which starts an external application (.exe) located at given path
void ttmm::KinectDevice::startup(LPCTSTR applicationPath)
{
	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	CreateProcess(applicationPath,   // the path of app to be started 
		NULL,           // Command line args
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi            // Pointer to PROCESS_INFORMATION structure
		);
	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

// Destructor, stops kinect sensor & update thread, deletes musicianBodies
ttmm::KinectDevice::~KinectDevice()
{
    stopDevice = true;
    deviceThread.join();

    if (deviceThread.joinable())
    {
        ttmm::logger.write(
            "Kinect thread still running but it shouldn't. That's weird.");
    }

    // close reader & mapper
    safeRelease(bodyReader);
    safeRelease(coordinateMapper);

    // close the Kinect sensor
    if (deviceHandle)
    {
        deviceHandle->Close();
    }
    safeRelease(deviceHandle);
    ttmm::logger.write("Stopping Kinect.. Goodbye!");

    // delete all bodies
    for (Body *b : musicianBodies)
    {
        delete b;
    }
}

// will be called by constructor in an own thread,
// periodically calls update() while device is open
void ttmm::KinectDevice::run()
{
    while (!stopDevice)
    {
		// read latest frame from camera, process body and floor, map coordinates
        update();

		// push all musicians and floordata to musicians vector
        for (size_t i = 0; i < musicianBodies.size(); i++)
        {
            if (musicians.size() <= i)									//<check if there is already a musician for this body, if there is none...
            {
                musicians.push_back(KinectMusician(musicianBodies[i])); //<...create new kinectmusician for body
				logger.write("A new KinectMusician was created at slot" + std::to_string(i));
			}
            else if (!musicians.at(i).hasBody())						//<else if there is already a musician and he has a body..
            {
                musicians.at(i).setBody(musicianBodies[i]);				//<...change his bodydata
				//logger.write("BodyData for KinectMusician at slot " + std::to_string(i) + " was changed");
            }
            musicians.at(i).pushEvent(floor2D);
        }
    }
	open = false;
}

// opens kinect sensor, bodyreader and coordinatemapper
HRESULT ttmm::KinectDevice::initSensor()
{
    ttmm::logger.write("starting Kinect...");
    HRESULT result;

    // get Kinect sensor
    result = GetDefaultKinectSensor(&deviceHandle);
    if (FAILED(result))
    {
        deviceHandle = nullptr;
        ttmm::logger.write("GetDefaultKinectSensor FAILED, Result: " +
                           std::to_string(result));
        return result;
    }

    if (deviceHandle)
    {
        // initialize Kinect and get coordinateMapper and bodyReader
        IBodyFrameSource *bodyFrameSource = NULL;

        result = deviceHandle->Open();

        if (SUCCEEDED(result))
        {
            ttmm::logger.write("kinectSensor Open SUCCEEDED, Result: " +
                               std::to_string(result));
            result = deviceHandle->get_CoordinateMapper(&coordinateMapper);
        }

        if (SUCCEEDED(result))
        {
            ttmm::logger.write("get_CoordinateMapper SUCCEEDED, Result: " +
                               std::to_string(result));
            result = deviceHandle->get_BodyFrameSource(&bodyFrameSource);
        }

        if (SUCCEEDED(result))
        {
            ttmm::logger.write("get_BodyFrameSource SUCCEEDED, Result: " +
                               std::to_string(result));
            result = bodyFrameSource->OpenReader(&bodyReader);
        }
		if (SUCCEEDED(result))
		{
			ttmm::logger.write("get_BodyReader SUCCEEDED, Result: " +
				std::to_string(result));
		}
        safeRelease(bodyFrameSource);
    }

    if (!deviceHandle || FAILED(result))
    {
        ttmm::logger.write("No ready Kinect found!");
        deviceHandle = nullptr;
        return E_FAIL;
    }

    return result;
}

// gets current body-frame from camera and calls processBody() function, is
// called periodically by run()
void ttmm::KinectDevice::update()
{
    if (!bodyReader)
    {
        return;
    }
    IBodyFrame *frame = NULL;
    if (SUCCEEDED(bodyReader->AcquireLatestFrame(&frame)))
    {
		open = true;
        IBody *bodies[BODY_COUNT] = {0};
        if (SUCCEEDED(frame->GetAndRefreshBodyData(_countof(bodies), bodies)))
        {
            processBody(BODY_COUNT, bodies);
        }
        frame->get_FloorClipPlane(&floor3D);
        mapFloorTo2D();

        for (int i = 0; i < _countof(bodies); ++i)
        {
            safeRelease(bodies[i]);
        }
	}
    safeRelease(frame);
}

// iterate through all bodies (generated by kinect sensor)
// and put their joint positions into KinectMusicians' bodies
void ttmm::KinectDevice::processBody(int bodyCount, IBody **bodies)
{

    if (coordinateMapper)
    {
        for (int i = 0; i < bodyCount && i < MAX_BODIES; ++i)
        {
            IBody *currentBody = bodies[i];
            if (currentBody)
            {
                BOOLEAN tracked = false;
                HRESULT result = currentBody->get_IsTracked(&tracked);
                if (SUCCEEDED(result) && tracked)
                {

                    // set detectedBodyIndex to first body we found
                    // after this body ignore all other bodies
                    //if (detectedBodyIndex != -1)
                    //{
                    //    if (i != detectedBodyIndex)
                    //    {
                    //        continue;
                    //    }
                    //}
                    //detectedBodyIndex = i;

                    // check if musician has a body
                    // if he has none, create new body
                    Body *b = musicianBodies[i];
                    if (b == nullptr)
                    {
                        musicianBodies[i] = new Body();
                        b = musicianBodies[i];
                    }

                    // get joints from current body and store their positions in musicians
                    // body (if joint type is supported)
                    Joint joints[JointType_Count];
                    if (SUCCEEDED(currentBody->GetJoints(_countof(joints), joints)))
                    {
                        for (int j = 0; j < _countof(joints); ++j)
                        {
                            if (Body::isSupported(joints[j].JointType))
                            {
                                D2D1_POINT_2F jointPoint = mapPoint(joints[j]);
                                b->setJoint(jointPoint.x, jointPoint.y, joints[j].JointType);
                            }
                            // check foot position for floor calculation
                            if (joints[j].JointType == JointType::JointType_SpineMid)
                            {
                                footX = joints[j].Position.X;
                                footZ = joints[j].Position.Z;
                            }
                        }
                    }
                }
            }
        }
    }
}

// maps a joint from CameraSpace to DepthSpace and then to 2D / ScreenPosition
D2D1_POINT_2F ttmm::KinectDevice::mapPoint(Joint joint)
{
    DepthSpacePoint depthPoint = {0};
    coordinateMapper->MapCameraPointToDepthSpace(joint.Position, &depthPoint);

    int width = 768;
    int height = 689;
    float screenPointX = static_cast<float>(depthPoint.X * width) / 512;
    float screenPointY = static_cast<float>(depthPoint.Y * height) / 424;

    return D2D1::Point2F(screenPointX, screenPointY);
}

// maps floor coordinates to 2D coordinates by using the general plan equation
void ttmm::KinectDevice::mapFloorTo2D()
{
    DepthSpacePoint depthPoint = {0};

    // The general plane equation for floor is:
    // Ax + By + Cz + D = 0   where:
    //			A = vFloorClipPlane.x
    //			B = vFloorClipPlane.y
    //			C = vFloorClipPlane.z
    //			D = vFloorClipPlane.w
    float y =
        (-(floor3D.x * footX) - (floor3D.z * footZ) - floor3D.w) / floor3D.y;
    CameraSpacePoint floorPoint{footX, y, footZ};
    coordinateMapper->MapCameraPointToDepthSpace(floorPoint, &depthPoint);

	// Depth space is the term used to describe a 2D location on the depth image. 
	// Think of this as a row / column location of a pixel where x is the column and y is the row.
	// So x = 0, y = 0 corresponds to the top left corner of the image and x = 511, y = 423 
	// (width - 1, height - 1) is the bottom right corner of the image.
    int width = 768;
    int height = 689;
    float screenPointX = static_cast<float>(depthPoint.X * width) / 512;
    float screenPointY = static_cast<float>(depthPoint.Y * height) / 424;

    floor2D = D2D1::Point2F(screenPointX, screenPointY);
}
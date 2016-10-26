// ConsoleApplication2.cpp : 定義主控台應用程式的進入點。
//

#include <iostream>
#include <opencv2\opencv.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video\background_segm.hpp>
#include <Kinect.h>
using namespace std;
using namespace cv;

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease) {
	if (pInterfaceToRelease != NULL) {
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

int main()
{
	//Sensor
	IKinectSensor* pSensor;
	HRESULT hResult = S_OK;
	hResult = GetDefaultKinectSensor(&pSensor);
	if (FAILED(hResult)) {
		cerr << "Error: GetDefaultKinectsensor" << endl;
		return -1;
	}
	hResult = pSensor->Open();
	if (FAILED(hResult)) {
		cerr << "Error: IKinectSensor::Open()" << endl;
		return -1;
	}

	//Depth Source
	IDepthFrameSource* pDepthSource;
	hResult = pSensor->get_DepthFrameSource(&pDepthSource);
	if (FAILED(hResult)) {
		cerr << "Error: IKinectSensor::get_DepthFrameSource()" << endl;
		return -1;
	}

	//Depth Reader
	IDepthFrameReader* pDepthReader;
	hResult = pDepthSource->OpenReader(&pDepthReader);
	if (FAILED(hResult)) {
		cerr << "Error: IDepthFrameSource::OpenReader()" << endl;
		return -1;
	}

	//Body Source
	IBodyFrameSource* pBodySource;
	hResult = pSensor->get_BodyFrameSource(&pBodySource);
	if (FAILED(hResult)) {
		cerr << "Error: IKinectSensor::get_BodyFrameSource()" << endl;
		return -1;
	}

	//Body Reader
	IBodyFrameReader* pBodyReader;
	hResult = pBodySource->OpenReader(&pBodyReader);
	if (FAILED(hResult)) {
		cerr << "Error: IBodyFrameSource::OpenReader()" << endl;
		return -1;
	}

	//Color Source
	IColorFrameSource* pColorSource;
	hResult = pSensor->get_ColorFrameSource(&pColorSource);
	if (FAILED(hResult)) {
		cerr << "Error: IKinectSensor::get_ColorFrameSource()" << endl;
		return -1;
	}

	//Color Reader
	IColorFrameReader* pColorReader;
	hResult = pColorSource->OpenReader(&pColorReader);
	if (FAILED(hResult)) {
		cerr << "Error: IColorFrameSource::OpenReader()" << endl;
		return -1;
	}

	//Coordinate Mapper
	ICoordinateMapper* pCoordinateMapper;
	hResult = pSensor->get_CoordinateMapper(&pCoordinateMapper);
	if (FAILED(hResult)) {
		cerr << "Error: IKinectSensor::get_CoordinateMapper()" << endl;
		return -1;
	}

	//Frame description
	int width = 1920;
	int height = 1080;
	int depthWidth = 512;
	int depthHeight = 424;
	IFrameDescription* pFrameDescription = nullptr;
	
	/*
	//pColorSource->get_FrameDescription(&pFrameDescription);
	pDepthSource->get_FrameDescription(&pFrameDescription);
	pFrameDescription->get_Width(&width);
	pFrameDescription->get_Height(&height);
	pFrameDescription->Release();
	pFrameDescription = nullptr;
	cout << "width,height: " << width << "," << height << endl;
	UINT16 uDepthMin = 0, uDepthMax = 0;
	pDepthSource->get_DepthMinReliableDistance(&uDepthMin);
	pDepthSource->get_DepthMaxReliableDistance(&uDepthMax);
	cout << "Reliable Distance: " << uDepthMin << "-" << uDepthMax << endl;
	*/

	//Color Table
	cv::Vec3b color[BODY_COUNT];
	color[0] = cv::Vec3b(255, 0, 0);
	color[1] = cv::Vec3b(0, 255, 0);
	color[2] = cv::Vec3b(0, 0, 255);
	color[3] = cv::Vec3b(255, 255, 0);
	color[4] = cv::Vec3b(255, 0, 255);
	color[5] = cv::Vec3b(0, 255, 255);


	//opencv
	unsigned int bufferSize = depthWidth * depthHeight * sizeof(unsigned short);
	unsigned int bufferColorSize = width * height * 4 * sizeof(unsigned char);
	cv::Mat bufferMat(depthHeight, depthWidth, CV_16UC1);
	cv::Mat depthMat(depthHeight, depthWidth, CV_8UC3);
	cv::Mat colorfulMat(depthHeight, depthWidth, CV_8UC3);
	cv::Mat colorBufferMat(height, width, CV_8UC4);
	cv::Mat bodyMat(height / 2, width / 2, CV_8UC4);

	int depthColorCheck = 0;

	//cv::namedWindow("Depth && Body");
	while (1) {
		//Depth Frame
		
		IDepthFrame* pDepthFrame = nullptr;
		hResult = pDepthReader->AcquireLatestFrame(&pDepthFrame);
		if (SUCCEEDED(hResult)) {
			hResult = pDepthFrame->AccessUnderlyingBuffer(&bufferSize, reinterpret_cast<UINT16**>(&bufferMat.data));
			if (SUCCEEDED(hResult)) {
				//bufferMat.convertTo(depthMat, CV_8U, -255.0f/8000.0f, 255.0f);
				//bufferMat.convertTo(depthMat, CV_8U, 0.005);//有點黑
				//bufferMat.convertTo(depthMat, CV_8U, 0.05);//變亮了
				//bufferMat.convertTo(depthMat, CV_8U, 0.5);//變超白
				bufferMat.convertTo(depthMat, CV_8U, 0.1);//白背景灰身體
				//bufferMat.convertTo(depthMat, CV_8U, 0.2);//白背景淡灰身體
				//bufferMat.convertTo(depthMat, CV_8U, 0.3);//白背景白身體

				//convert color
				if(depthColorCheck == 0){
					for (int y = 0;y < depthHeight;y++) {
						for (int x = 0;x < depthWidth;x++) {
							depthMat.at<uchar>(y,x) = 255-depthMat.at<uchar>(y,x);
						}
					}
				}
				#define Color_B(depthMat, y, x) depthMat.at<Vec3b>(y,x)[0]
				#define Color_G(depthMat, y, x) depthMat.at<Vec3b>(y,x)[1]
				#define Color_R(depthMat, y, x) depthMat.at<Vec3b>(y,x)[2]
				uchar tmp=0;
				// 51,102,153,204
				for (int y = 0;y < depthHeight;y++) {
					for (int x = 0;x < depthWidth;x++) {

						tmp=depthMat.at<uchar>(y,x);
						if (tmp <= 51) {
							Color_B(colorfulMat,y,x) = 255;
							Color_G(colorfulMat, y, x) = tmp*5;
							Color_R(colorfulMat, y, x) = 30;
						}else if (tmp <= 102) {
							tmp-=51;
							Color_B(colorfulMat, y, x) = 255-tmp*5;
							Color_G(colorfulMat, y, x) = 255;
							Color_R(colorfulMat, y, x) = 30;
						}else if (tmp <= 153) {
							tmp-=102;
							Color_B(colorfulMat, y, x) = 30;
							Color_G(colorfulMat, y, x) = 255;
							Color_R(colorfulMat, y, x) = tmp*5;
						}else if (tmp <= 204) {
							tmp-=153;
							Color_B(colorfulMat, y, x) = 30;
							Color_G(colorfulMat, y, x) = 255-uchar(128.0*tmp/51.0+0.5);
							Color_R(colorfulMat, y, x) = 255;
						}else{
							tmp-=204;
							Color_B(colorfulMat, y, x) = 30;
							Color_G(colorfulMat, y, x) = 127-uchar(127.0*tmp/51.0+0.5);
							Color_R(colorfulMat, y, x) = 255;
						}

					}
				}
			}
		}
		SafeRelease(pDepthFrame);
		

		//Color Frame
		IColorFrame* pColorFrame = nullptr;
		hResult = pColorReader->AcquireLatestFrame(&pColorFrame);
		if (SUCCEEDED(hResult)) {
			hResult = pColorFrame->CopyConvertedFrameDataToArray(bufferColorSize, reinterpret_cast<BYTE*>(colorBufferMat.data), ColorImageFormat_Bgra);
			if (SUCCEEDED(hResult)) {
				cv::resize(colorBufferMat, bodyMat, cv::Size(), 0.5, 0.5);
			}
		}

		//Body Frame
		IBodyFrame* pBodyFrame = nullptr;
		hResult = pBodyReader->AcquireLatestFrame(&pBodyFrame);
		if (SUCCEEDED(hResult)) {
			IBody* pBody[BODY_COUNT] = {0};
			hResult = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, pBody);
			if (SUCCEEDED(hResult)) {
				for (int count = 0;count < BODY_COUNT;count++) {
					BOOLEAN bTracked = false;
					hResult = pBody[count]->get_IsTracked(&bTracked);
					if (SUCCEEDED(hResult) && bTracked) {
						Joint joint[JointType::JointType_Count];
						hResult = pBody[count]->GetJoints(JointType::JointType_Count, joint);
						if (SUCCEEDED(hResult)) {
						
							//Left Hand state							
							HandState leftHandState = HandState::HandState_Unknown;
							hResult = pBody[count]->get_HandLeftState(&leftHandState);
							if (SUCCEEDED(hResult)) {
								ColorSpacePoint colorSpacePoint = {0};
								hResult = pCoordinateMapper->MapCameraPointToColorSpace(joint[JointType::JointType_HandLeft].Position, &colorSpacePoint);
								if (SUCCEEDED(hResult)) {
									int x = static_cast<int>(colorSpacePoint.X);
									int y = static_cast<int>(colorSpacePoint.Y);
									if ((x >= 0) && (x < width) && (y >= 0) && (y < height)) {
										if (leftHandState == HandState::HandState_Open) {
											cv::circle(colorBufferMat, cv::Point(x,y),75,cv::Scalar(0,128,0), 5, CV_AA);
										}
										else if (leftHandState == HandState::HandState_Closed) {
											cv::circle(colorBufferMat, cv::Point(x, y), 75, cv::Scalar(0, 0, 128), 5, CV_AA);
											depthColorCheck = 0;
										}
										else if (leftHandState == HandState::HandState_Lasso) {
											cv::circle(colorBufferMat, cv::Point(x, y), 75, cv::Scalar(128, 128, 0), 5, CV_AA);
											if (depthColorCheck == 0) {
												std::cout << "color turn" << endl;
												for (int y = 0;y < depthHeight;y++) {
													for (int x = 0;x < depthWidth;x++) {
														depthMat.at<uchar>(y, x) = 255 - depthMat.at<uchar>(y, x);
													}
												}
												
												depthColorCheck = 1;
											}
											
										}
									}
								}
							}
							
							//Right Hand state							
							HandState rightHandState = HandState::HandState_Unknown;
							hResult = pBody[count]->get_HandRightState(&rightHandState);
							if (SUCCEEDED(hResult)) {
								ColorSpacePoint colorSpacePoint = { 0 };
								hResult = pCoordinateMapper->MapCameraPointToColorSpace(joint[JointType::JointType_HandRight].Position, &colorSpacePoint);
								if (SUCCEEDED(hResult)) {
									int x = static_cast<int>(colorSpacePoint.X);
									int y = static_cast<int>(colorSpacePoint.Y);
									if ((x >= 0) && (x < width) && (y >= 0) && (y < height)) {
										if (rightHandState == HandState::HandState_Open) {
											cv::circle(colorBufferMat, cv::Point(x, y), 75, cv::Scalar(0, 128, 0), 5, CV_AA);
										}
										else if (rightHandState == HandState::HandState_Closed) {
											cv::circle(colorBufferMat, cv::Point(x, y), 75, cv::Scalar(0, 0, 128), 5, CV_AA);
											depthColorCheck = 0;
										}
										else if (rightHandState == HandState::HandState_Lasso) {
											cv::circle(colorBufferMat, cv::Point(x, y), 75, cv::Scalar(128, 128, 0), 5, CV_AA);
											if (depthColorCheck == 0) {
												std::cout << "color turn" << endl;
												for (int y = 0;y < depthHeight;y++) {
													for (int x = 0;x < depthWidth;x++) {
														depthMat.at<uchar>(y, x) = 255 - depthMat.at<uchar>(y, x);
													}
												}

												depthColorCheck = 1;
											}

										}
									}
								}
							}


							//Joint
							for (int type = 0;type < JointType::JointType_Count;type++) {
								ColorSpacePoint colorSpacePoint = { 0 };
								pCoordinateMapper->MapCameraPointToColorSpace(joint[type].Position, &colorSpacePoint);
								int x = static_cast<int>(colorSpacePoint.X);
								int y = static_cast<int>(colorSpacePoint.Y);
								if ((x >= 0) && (x < width) && (y >= 0) && (y < height)) {
									cv::circle(colorBufferMat, cv::Point(x, y), 5, static_cast<cv::Scalar>(color[count]), -1, CV_AA);
								}
							}


						}
					}
				}
				cv::resize(colorBufferMat, bodyMat, cv::Size(), 0.5, 0.5);
			}
			for (int count = 0;count < BODY_COUNT;count++) {
				SafeRelease(pBody[count]);
			}
		}


		SafeRelease(pBodyFrame);
		SafeRelease(pColorFrame);

		//Show Window
		cv::imshow("Depth",colorfulMat);
		cv::imshow("Body", bodyMat);
		if (cv::waitKey(30) == VK_ESCAPE) {
			break;
		}
	}

	//release frame source
	std::cout << "release source" << endl;
	SafeRelease(pDepthSource);
	SafeRelease(pColorSource);
	SafeRelease(pBodySource);

	//release frame reader
	std::cout << "release reader" << endl;
	SafeRelease(pDepthReader);
	SafeRelease(pColorReader);
	SafeRelease(pBodyReader);

	//close Sensor
	std::cout << "Release sensor" << endl;
	SafeRelease(pSensor);

	return 0;
}


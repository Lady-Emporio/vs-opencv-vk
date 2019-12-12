#include "stdafx.h"

#ifdef __linux__
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#endif
#include <thread>
#include <wchar.h>
#include <string>
#include "AddInNative.h"
#include <Windows.h>
#include <fstream>
#include <string>
#include <mutex>
std::mutex g_num_mutex;
std::mutex lastThreadWork;
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#include <chrono>
void myLog(std::string text) {
	/*
	std::ofstream outfile;
	outfile.open("D:/log.txt", std::ios_base::app);

	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y.%m.%d %H:%M:%S", &tstruct);

	std::string String_now = buf;
	outfile << String_now <<" | "<< text << std::endl;
	outfile.close();
	*/
}

static const wchar_t g_kClassNames[] = L"CAddInNative"; //|OtherClass1|OtherClass2";

//---------------------------------------------------------------------------//
long GetClassObject(const wchar_t* wsName, IComponentBase** pInterface)
{
	myLog("GetClassObject");
	if (!*pInterface)
	{
		*pInterface = new CAddInNative();
		return (long)*pInterface;
	}
	myLog("GetClassObject bad end");
	return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
	myLog("DestroyObject");
	if (!*pIntf)
		return -1;

	delete* pIntf;
	*pIntf = 0;
	return 0;
}
//---------------------------------------------------------------------------//
const WCHAR_T* GetClassNames()
{
	myLog("GetClassNames");
	//static WCHAR_T* names = 0;
	/*if (!names)
		::convToShortWchar(&names, g_kClassNames);*/
	WCHAR_T* lnames = L"CAddInNative";
	return lnames;

}

void MySecondFunc(CAddInNative* main) {
	VideoCapture cap;
	if (!cap.open(0)) {
		MessageBox(NULL, "Not open camera. Maybe not found.", "Camera error. Not open.", MB_OK);
		return;
	}
	int counter = 0;
	lastThreadWork.lock();
	while(true)
	{
		Mat frame;
		//cap >> frame;
		if (!cap.isOpened())  // if not success, exit program
		{
			MessageBox(NULL, "Camera is closed. Camera not is opened.", "Camera error.", MB_OK);
			break;
		}
		bool bSuccess = cap.read(frame); // read a new frame from video
		if (!bSuccess) //if not success, break loop
		{
			MessageBox(NULL, "Cannot read a frame from video file. Maybe camera not connecter to computer.", "Camera error.", MB_OK);
			break;
		}
		if (frame.empty()) {
			MessageBox(NULL, "Frame is empty.", "Frame error.", MB_OK);
			break;
		}
		//imshow("this is you, smile! :)", frame);
		//if (waitKey(10) == 27) {
		//	std::cout << "Break. Press ESC" << std::endl;
		//	break; // stop capturing by pressing ESC 
		//}
		counter++;
		try {
			std::string path = "C:/imgs/img" + std::to_string(counter) + ".jpg";
			bool result_save = imwrite(path, frame);
			if (!result_save) {
				MessageBox(NULL, "Cannot save image in ""C:/imgs/img/imgN.jpg"".", "Image save error.", MB_OK);
				break;
			}
			myLog("save image");

			wchar_t* who = L"ComponentNative", * what = L"Timer";

			wchar_t buffer[1024];
			std::wstring wstr(path.begin(), path.end());
			std::size_t length = wstr.copy(buffer, 1023, 0);
			buffer[length] = '\0';

			g_num_mutex.lock();
			int test = *main->cloneToEnd;
			g_num_mutex.unlock();
			if (9 != test) {
				//MessageBox(NULL, "Camera stop.", "End thread.", MB_OK);
				myLog("Close thread. Stop thread.");
				break;
			}
			if (8 == test) {
				//MessageBox(NULL, "Camera stop.", "End thread.", MB_OK);
				myLog("Close thread. Need close no new.");
				break;
			}
			g_num_mutex.lock();
			bool res = (main->workThread != std::this_thread::get_id());
			g_num_mutex.unlock();
			if (res) {
				myLog("Close thread. Not this work thread. Thread id not work thread id.");
				//MessageBox(NULL, "Not this thread", "End thread.", MB_OK);
				break;
			}
			
			main->pAsyncEvent->ExternalEvent(who, what, buffer);
		}
		catch (std::runtime_error & ex) {
			MessageBox(NULL, "Exception converting image to PNG format. Something wrong with frame in save time.", "Its runtime_error.", MB_OK);
			break;
		}

		if (counter >= 100) {
			counter = 0;
		}
	}
	myLog("This thread function close.");
	lastThreadWork.unlock();
}

void CAddInNative::beginGivesMePhoto()
{
	myLog("beginGivesMePhoto begin");
	g_num_mutex.lock();
	*cloneToEnd = 8;
	g_num_mutex.unlock();

	lastThreadWork.lock();
	myLog("last thread lock check before start next.");
	lastThreadWork.unlock();

	g_num_mutex.lock();
	*cloneToEnd = 9;
	g_num_mutex.unlock();

	std::thread first(MySecondFunc, this);
	workThread = first.get_id();
	first.detach();
	myLog("beginGivesMePhoto end");
}

void CAddInNative::_stopGetPhoto()
{
	myLog("stopGetPhoto begin");
	g_num_mutex.lock();
	*cloneToEnd = 1;
	g_num_mutex.unlock();
	myLog("stopGetPhoto end");
}



//---------------------------------------------------------------------------//
//CAddInNative
CAddInNative::CAddInNative()
{
	myLog("CAddInNative");
	cloneToEnd = new int(9);
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
	myLog("~CAddInNative");
	*cloneToEnd =2;
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{
	myLog("Init");
	IAddInDefBase_point = (IDispatch*)pConnection;
	pAsyncEvent = (IAddInDefBase*)pConnection;
	*cloneToEnd = 9;
	return true;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetInfo()
{
	myLog("GetInfo");
	return 2000;
}
//---------------------------------------------------------------------------//
void CAddInNative::Done()
{
	myLog("Done");
	//cloneToEnd = new int(2);
}
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsLanguageExt)
{
	myLog("RegisterExtensionAs");
	wchar_t* wsExtension = L"VkForCameraThis";
	int iActualSize = ::wcslen(wsExtension) + 1;
	WCHAR_T* dest = 0;
	if (this->memManager)
	{
		auto res = this->memManager->AllocMemory((void**)wsLanguageExt, iActualSize * sizeof(WCHAR_T));
		if (res)
			::convToShortWchar(wsLanguageExt, wsExtension, iActualSize);
		myLog("RegisterExtensionAs return 'VkForCameraThis'");
		return true;
	}
	myLog("RegisterExtensionAs bad end");
	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNProps()
{
	myLog("GetNProps");
	return eLastProp;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindProp(const WCHAR_T* wsPropName)
{
	myLog("FindProp");
	return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetPropName(long lPropNum, long lPropAlias)
{
	myLog("GetPropName");
	return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetPropVal(const long lPropNum, tVariant* pvarPropVal)
{
	myLog("GetPropVal");
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::SetPropVal(const long lPropNum, tVariant* varPropVal)
{
	myLog("SetPropVal");
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropReadable(const long lPropNum)
{
	myLog("IsPropReadable");
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::IsPropWritable(const long lPropNum)
{
	myLog("IsPropWritable");
	return false;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNMethods()
{
	myLog("GetNMethods");
	return eLastMethod;
}
//---------------------------------------------------------------------------//
long CAddInNative::FindMethod(const WCHAR_T* wsMethodName)
{
	myLog("FindMethod");
	wchar_t* tempString = 0;
	convFromShortWchar(&tempString, wsMethodName);
	std::wstring wstring(tempString);
	std::string methodMame(wstring.begin(), wstring.end());
	myLog("FindMethod get:" + methodMame);
	if ("beginGiveMePhoto" == methodMame) {
		int result = CAddInNative::Methods::beginGiveMePhoto;
		myLog("FindMethod return:" + std::to_string(result));
		return result;
	}
	else if ("stopGetPhoto" == methodMame) {
		int result = CAddInNative::Methods::stopGetPhoto;
		myLog("FindMethod return:" + std::to_string(result));
		return result;
	}
	return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum,
	const long lMethodAlias)
{
	myLog("GetMethodName");
	return 0;
}
//---------------------------------------------------------------------------//
long CAddInNative::GetNParams(const long lMethodNum)
{
	myLog("GetNParams:" + std::to_string(lMethodNum));
	switch (lMethodNum)
	{
	case CAddInNative::Methods::beginGiveMePhoto:
		return 0;
	case CAddInNative::Methods::stopGetPhoto:
		return 0;
	}
	return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
	tVariant* pvarParamDefValue)
{
	myLog("GetParamDefValue");
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{
	myLog("HasRetVal:" + std::to_string(lMethodNum));
	switch (lMethodNum)
	{
	case CAddInNative::CAddInNative::beginGiveMePhoto:
		return false;
	case CAddInNative::CAddInNative::stopGetPhoto:
		return false;
	}
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
	tVariant* paParams, const long lSizeArray)
{
	myLog("CallAsProc get:" + std::to_string(lMethodNum));
	switch (lMethodNum)
	{
	case CAddInNative::Methods::beginGiveMePhoto:
		myLog("CallAsProc need call 'beginGiveMePhoto'.");
		this->beginGivesMePhoto();
		return true;
	case CAddInNative::Methods::stopGetPhoto:
		myLog("CallAsProc need call 'stopGetPhoto'.");
		this->_stopGetPhoto();
		return true;
	}
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsFunc(const long lMethodNum,
	tVariant* pvarRetValue, tVariant* paParams, const long lSizeArray)
{
	myLog("CallAsFunc");
	return false;
}
//---------------------------------------------------------------------------//
void CAddInNative::SetLocale(const WCHAR_T* loc)
{
	myLog("SetLocale");
#ifndef __linux__
	_wsetlocale(LC_ALL, loc);
#else
	int size = 0;
	char* mbstr = 0;
	wchar_t* tmpLoc = 0;
	convFromShortWchar(&tmpLoc, loc);
	size = wcstombs(0, tmpLoc, 0) + 1;
	mbstr = new char[size];

	if (!mbstr)
	{
		delete[] tmpLoc;
		return;
	}

	memset(mbstr, 0, size);
	size = wcstombs(mbstr, tmpLoc, wcslen(tmpLoc));
	setlocale(LC_ALL, mbstr);
	delete[] tmpLoc;
	delete[] mbstr;
#endif
}
//---------------------------------------------------------------------------//
bool CAddInNative::setMemManager(void* mem)
{
	myLog("setMemManager");
	memManager = (IMemoryManager*)mem;
	return true;
}
//---------------------------------------------------------------------------//

uint32_t convToShortWchar(WCHAR_T** Dest, const wchar_t* Source, uint32_t len)
{
	if (!len)
		len = ::wcslen(Source) + 1;

	if (!*Dest)
		*Dest = new WCHAR_T[len];

	WCHAR_T* tmpShort = *Dest;
	wchar_t* tmpWChar = (wchar_t*)Source;
	uint32_t res = 0;

	::memset(*Dest, 0, len * sizeof(WCHAR_T));
	do
	{
		*tmpShort++ = (WCHAR_T)*tmpWChar++;
		++res;
	} while (len-- && *tmpWChar);

	return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
	if (!len)
		len = getLenShortWcharStr(Source) + 1;

	if (!*Dest)
		*Dest = new wchar_t[len];

	wchar_t* tmpWChar = *Dest;
	WCHAR_T* tmpShort = (WCHAR_T*)Source;
	uint32_t res = 0;

	::memset(*Dest, 0, len * sizeof(wchar_t));
	do
	{
		*tmpWChar++ = (wchar_t)*tmpShort++;
		++res;
	} while (len-- && *tmpShort);

	return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
	uint32_t res = 0;
	WCHAR_T* tmpShort = (WCHAR_T*)Source;

	while (*tmpShort++)
		++res;

	return res;
}
//---------------------------------------------------------------------------//
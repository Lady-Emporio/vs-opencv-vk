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

std::mutex saveImageMutex;
std::mutex StopMutex;

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

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
		MessageBox(NULL, "Not open camera. Maybe not found. Need restart.", "Camera error. Not open. Need restart.", MB_OK);
		return;
	}
	while (true)
	{
		bool needBreak = false;
		StopMutex.lock();
		if (false==main->isWork) {
			needBreak = true;
		} 
		StopMutex.unlock();
		if (needBreak) {
			break;
		}

		Mat frame;
		if (!cap.isOpened())
		{
			MessageBox(NULL, "Camera is closed. Camera not is opened.", "Camera error. Need restart.", MB_OK);
			break;
		}

		bool bSuccess = cap.read(frame);
		if (!bSuccess)
		{
			MessageBox(NULL, "Cannot read a frame from video file. Maybe camera not connecter to computer.", "Camera error. Need restart.", MB_OK);
			break;
		}

		if (frame.empty()) {
			MessageBox(NULL, "Frame is empty. Need restart.", "Frame error.", MB_OK);
			break;
		}
		try {
			myLog("save image");
			wchar_t* who = L"ComponentNative", *what = L"Timer";

			//wchar_t buffer[1024];
			//std::wstring wstr(path.begin(), path.end());
			//std::size_t length = wstr.copy(buffer, 1023, 0);
			//buffer[length] = '\0';

			std::vector<uchar> buff;
			std::vector<int> param(2);
			param[0] = cv::IMWRITE_JPEG_QUALITY;
			param[1] = 100;//default(95) 0-100
			cv::imencode(".jpg", frame, buff, param);

			saveImageMutex.lock();
			main->M_buff.clear();
			for (char i : buff) {
				main->M_buff.push_back(i);
			}
			saveImageMutex.unlock();
			/*
			std::ofstream outfile;
			outfile.open("D:/log2.jpg", std::ios_base::binary | std::ios_base::out);
			for (char i : buff) {
				outfile << i;
			}
			outfile.close();
			*/

			std::wstring wstr = L"next";
			main->pAsyncEvent->ExternalEvent(who, what, wstr.data());

			
			
		}
		catch (std::runtime_error & ex) {
			MessageBox(NULL, "Exception converting image to PNG format. Something wrong with frame in save time.", "Its runtime_error.", MB_OK);
		}
	}
	myLog("This thread function close.");
}


void CAddInNative::_giveNextImage(tVariant * val) {

	/*
	//“ак можно вернуть истину или ложь
	TV_VT(val) = VTYPE_BOOL; // действующий тип данных
	TV_BOOL(val) = false; // устанавливаем само значение
   */

   /*
   //“ак можно вернуть строку
  std::string str = "w1234r";
  char* t1 = str.data();
  TV_VT(val) = VTYPE_PSTR;
  memManager->AllocMemory((void**)&t1, (str.length() + 1) * sizeof(WCHAR_T));
  memcpy(t1, str.c_str(), (str.length() + 1) * sizeof(WCHAR_T));
  val->pstrVal = t1;
  val->strLen = str.length();
  */

	
	
	std::string str = "";

	bool zero = false;

	saveImageMutex.lock();
	for (uchar i : M_buff) {
		str.push_back(i);
	}
	if (M_buff.size() == 0) {
		zero = true;
	}
	saveImageMutex.unlock();

	if (zero) {
		return;
	}
	
	char* t1 = str.data();
	//memManager->AllocMemory((void**)&t1, (str.length() + 1) * sizeof(WCHAR_T));
	memManager->AllocMemory((void**)&t1, (str.length() + 1) * sizeof(char));
	//memcpy(t1, str.c_str(), (str.length() + 1) * sizeof(WCHAR_T));
	memcpy(t1, str.c_str(), (str.length() + 1) * sizeof(char));

	TV_VT(val) = VTYPE_BLOB;
	val->pstrVal = t1;
	//val->strLen = str.length();
	val->strLen = (str.length() + 1) * sizeof(char);
	
}

void CAddInNative::beginGivesMePhoto()
{
	myLog("beginGivesMePhoto begin");
	if (fut.valid()) {
		fut.wait();
	}
	//_stopGetPhoto();
	//std::future<void> fut = std::async(MySecondFunc, this);
	fut= std::async(MySecondFunc, this);
	myLog("beginGivesMePhoto end");
}

void CAddInNative::_stopGetPhoto()
{
	myLog("stopGetPhoto begin");

	StopMutex.lock();
	this->isWork = false;
	StopMutex.unlock();
	fut.wait();
	myLog("stopGetPhoto end");
}



//---------------------------------------------------------------------------//
//CAddInNative
CAddInNative::CAddInNative()
{
	myLog("CAddInNative");
}
//---------------------------------------------------------------------------//
CAddInNative::~CAddInNative()
{
	myLog("~CAddInNative begin");

	StopMutex.lock();
	this->isWork = false;
	StopMutex.unlock();

	fut.wait();
	myLog("~CAddInNative end");
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{
	myLog("Init");
	IAddInDefBase_point = (IDispatch*)pConnection;
	pAsyncEvent = (IAddInDefBase*)pConnection;
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
	else if ("giveNextImage" == methodMame) {
		int result = CAddInNative::Methods::giveNextImage;
		myLog("FindMethod return:" + std::to_string(result));
		return result;
	}
	return -1;
}
//---------------------------------------------------------------------------//
const WCHAR_T* CAddInNative::GetMethodName(const long lMethodNum,
	const long lMethodAlias)
{
	myLog("GetMethodName | lMethodNum:"+std::to_string(lMethodNum)+" | lMethodAlias:"+std::to_string(lMethodAlias));
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
	case CAddInNative::CAddInNative::giveNextImage:
		return true;
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
	myLog("CallAsFunc get:" + std::to_string(lMethodNum));
	switch (lMethodNum)
	{
	case CAddInNative::Methods::giveNextImage:
		myLog("CallAsFunc need call 'giveNextImage'.");
		this->_giveNextImage(pvarRetValue);
		return true;
	}
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
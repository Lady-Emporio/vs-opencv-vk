
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

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;


void myLog(std::string text) {
	/*std::ofstream outfile;

	QString time_format = "yyyy-MM-dd  HH:mm:ss";
	QDateTime a = QDateTime::currentDateTime();
	QString as = a.toString(time_format);

	outfile.open("F:/log.txt", std::ios_base::app);
	outfile << as.toStdString()<<" | "<< text << std::endl;
	outfile.close();*/
}

static const wchar_t g_kClassNames[] = L"CAddInNative"; //|OtherClass1|OtherClass2";
 
//---------------------------------------------------------------------------//
long GetClassObject(const wchar_t* wsName, IComponentBase** pInterface)
{
	myLog("GetClassObject");
    if(!*pInterface)
    {
        *pInterface= new CAddInNative();
        return (long)*pInterface;
    }
	myLog("GetClassObject bad end");
    return 0;
}
//---------------------------------------------------------------------------//
long DestroyObject(IComponentBase** pIntf)
{
	myLog("DestroyObject");
   if(!*pIntf)
      return -1;

   delete *pIntf;
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

void MySecondFunc(CAddInNative *main) {
	VideoCapture cap;
	if (!cap.open(0)) {
		MessageBox(NULL, "Not open camera. Maybe not found.", "Camera error. Not open.", MB_OK);
		return;
	}
	int counter = 0;
	for (;;)
	{
		Mat frame;
		//cap >> frame;
		if (!cap.isOpened())  // if not success, exit program
		{
			MessageBox(NULL, "Camera is closed. Camera not is opened.", "Camera error.", MB_OK);
			return;
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
				return;
			}
			wchar_t *who = L"ComponentNative", *what = L"Timer";

			wchar_t buffer[1024];
			std::wstring wstr(path.begin(), path.end());
			std::size_t length = wstr.copy(buffer, 1023,0);
			buffer[length] = '\0';


			main->pAsyncEvent->ExternalEvent(who, what, buffer);
		}
		catch (std::runtime_error& ex) {
			MessageBox(NULL, "Exception converting image to PNG format. Something wrong with frame in save time.", "Its runtime_error.", MB_OK);
			return;
		}

		if (counter >= 10000) {
			counter = 0;
		}
	}
}

void CAddInNative::beginGivesMePhoto()
{
	std::thread first(MySecondFunc,this);
	first.detach();
	/*
	VideoCapture cap;
	if (!cap.open(0)) {
		MessageBox(NULL, "Not open camera. Maybe not found.", "Camera error. Not open.", MB_OK);
		return;
	}
	int counter = 0;
	for (;;)
	{
		Mat frame;
		//cap >> frame;
		if (!cap.isOpened())  // if not success, exit program
		{
			MessageBox(NULL, "Camera is closed. Camera not is opened.", "Camera error.", MB_OK);
			return;
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
				return;
			}
			wchar_t *who = L"ComponentNative", *what = L"Timer";

			wchar_t buffer[1024];
			std::wstring str(L"Test string...");
			std::size_t length = str.copy(buffer, 1023, 5);
			buffer[length] = '\0';


			pAsyncEvent->ExternalEvent(who, what, buffer);
		}
		catch (std::runtime_error& ex) {
			MessageBox(NULL, "Exception converting image to PNG format. Something wrong with frame in save time.", "Its runtime_error.", MB_OK);
			return;
		}

		if (counter >= 100) {
			counter = 0;
		}
	}
	*/
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
	myLog("~CAddInNative");
}
//---------------------------------------------------------------------------//
bool CAddInNative::Init(void* pConnection)
{ 
	myLog("Init");
	IAddInDefBase_point = (IDispatch *)pConnection;
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
}
//---------------------------------------------------------------------------//
bool CAddInNative::RegisterExtensionAs(WCHAR_T** wsLanguageExt)
{ 
	myLog("RegisterExtensionAs");
	wchar_t *wsExtension = L"VkForCameraThis";
	int iActualSize = ::wcslen(wsExtension) + 1;
	WCHAR_T* dest = 0;
	if (this->memManager)
	{
		auto res=this->memManager->AllocMemory((void**)wsLanguageExt, iActualSize * sizeof(WCHAR_T));
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
	myLog("FindMethod get:"+ methodMame);
	if ("beginGiveMePhoto" == methodMame) {
		int result = CAddInNative::Methods::beginGiveMePhoto;
		myLog("FindMethod return:"+ std::to_string(result));
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
	myLog("GetNParams:"+ std::to_string(lMethodNum));
	switch (lMethodNum)
	{
	case CAddInNative::Methods::beginGiveMePhoto:
		return 0;
	}
	return 0;
}
//---------------------------------------------------------------------------//
bool CAddInNative::GetParamDefValue(const long lMethodNum, const long lParamNum,
                          tVariant *pvarParamDefValue)
{ 
	myLog("GetParamDefValue"); 
	return false;
} 
//---------------------------------------------------------------------------//
bool CAddInNative::HasRetVal(const long lMethodNum)
{ 
	myLog("HasRetVal:"+std::to_string(lMethodNum));
	switch (lMethodNum)
	{
	case CAddInNative::CAddInNative::beginGiveMePhoto:
		return false;
	}
	return false;
}
//---------------------------------------------------------------------------//
bool CAddInNative::CallAsProc(const long lMethodNum,
                    tVariant* paParams, const long lSizeArray)
{ 
	myLog("CallAsProc get:"+std::to_string(lMethodNum));
	switch (lMethodNum)
	{
	case CAddInNative::Methods::beginGiveMePhoto:
		myLog("CallAsProc need call 'beginGiveMePhoto'.");
		this->beginGivesMePhoto();
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
    char *mbstr = 0;
    wchar_t *tmpLoc = 0;
    convFromShortWchar(&tmpLoc, loc);
    size = wcstombs(0, tmpLoc, 0)+1;
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
        len = ::wcslen(Source)+1;

    if (!*Dest)
        *Dest = new WCHAR_T[len];

    WCHAR_T* tmpShort = *Dest;
    wchar_t* tmpWChar = (wchar_t*) Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(WCHAR_T));
    do
    {
        *tmpShort++ = (WCHAR_T)*tmpWChar++;
        ++res;
    }
    while (len-- && *tmpWChar);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t convFromShortWchar(wchar_t** Dest, const WCHAR_T* Source, uint32_t len)
{
    if (!len)
        len = getLenShortWcharStr(Source)+1;

    if (!*Dest)
        *Dest = new wchar_t[len];

    wchar_t* tmpWChar = *Dest;
    WCHAR_T* tmpShort = (WCHAR_T*)Source;
    uint32_t res = 0;

    ::memset(*Dest, 0, len*sizeof(wchar_t));
    do
    {
        *tmpWChar++ = (wchar_t)*tmpShort++;
        ++res;
    }
    while (len-- && *tmpShort);

    return res;
}
//---------------------------------------------------------------------------//
uint32_t getLenShortWcharStr(const WCHAR_T* Source)
{
    uint32_t res = 0;
    WCHAR_T *tmpShort = (WCHAR_T*)Source;

    while (*tmpShort++)
        ++res;

    return res;
}
//---------------------------------------------------------------------------//

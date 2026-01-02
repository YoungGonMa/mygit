#include "StdAfx.h"
#include "DestinyOneAgent.h"
#include "TestCodeForAgent.h"

#include <LibDOBase/DOUtilEncode.h>
#include <LibDOBase/DOUtilBase.h>

#ifdef _KSJIN_TEST
//////////////////////////////////////////////////////////////////////////
// 진책임 테스트 코드 구간입니다. [9/4/2018 ksjin]
//////////////////////////////////////////////////////////////////////////
#include "../../common/src/DestinyECMIF/ObjectList.h"


std::string AESEncryptionNEncode2(const char* plainText)
{
	CDoSiteKeyMgr SiteKeyMgr;
	SiteKeyMgr.GetEncryptionKey();

	CDoEncMgr AesEncMgrForBase64(SiteKeyMgr.GetSiteKeyPtr(), SiteKeyMgr.GetSiteKeySize(), ENCMGR_MODE_ENC);

	BYTE* pEnced = new BYTE[lstrlenA(plainText) + 16];
	DWORD dwEnced = 0;
	AesEncMgrForBase64.EncryptBuffersWithFinalize((BYTE*)plainText, lstrlenA(plainText), pEnced, lstrlenA(plainText) + 16, dwEnced);


	std::string aaa;
	aaa.assign((char *)pEnced, dwEnced);

	delete pEnced;

	std::string base64encodedciphertext2;

	CryptoPP::StringSource(aaa, true,
		new CryptoPP::HexEncoder(
			new CryptoPP::StringSink(base64encodedciphertext2)
		) // Base64Encoder
	);


	return base64encodedciphertext2;
}


std::string AESDecryptionNEncode(std::string cipherText)
{
	byte key[AES::MAX_KEYLENGTH];
	byte iv[AES::BLOCKSIZE];


	CDoSiteKeyMgr SiteKeyMgr;
	SiteKeyMgr.GetEncryptionKey();
	SiteKeyMgr.GetKeyAndIV(key, iv);


	std::string decryptedtext;
	std::string base64decryptedciphertext;

	CryptoPP::StringSource(cipherText, true,
		new CryptoPP::HexDecoder(
			new CryptoPP::StringSink(base64decryptedciphertext)
		) // Base64Encoder
	);


	CDoEncMgr AesDecMgrForBase64(SiteKeyMgr.GetSiteKeyPtr(), SiteKeyMgr.GetSiteKeySize(), ENCMGR_MODE_DEC);

	BYTE* pDeced = new BYTE[base64decryptedciphertext.size() + 16];
	DWORD dwDeced = 0;

	AesDecMgrForBase64.DecryptBuffersWithFinalize((BYTE*)base64decryptedciphertext.c_str(),
		base64decryptedciphertext.size(),
		pDeced, base64decryptedciphertext.size() + 16, dwDeced);


	decryptedtext.assign((char*)pDeced, dwDeced);

	delete pDeced;

	return decryptedtext;

	// 
	// 
	// 
	// 	// 	CryptoPP::AES::Decryption aesDecryption(key,
	// 	//        CryptoPP::AES::DEFAULT_KEYLENGTH);
	// 
	// 	CryptoPP::AES::Decryption aesDecryption(key,
	// 		SiteKeyMgr.GetSiteKeySize());
	// 
	// 	CryptoPP::CBC_Mode_ExternalCipher::Decryption
	// 		cbcDecryption(aesDecryption, iv);
	// 
	// 	CryptoPP::StreamTransformationFilter
	// 		stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
	// 	stfDecryptor.Put(reinterpret_cast<const unsigned char*>
	// 		(base64decryptedciphertext.c_str()), base64decryptedciphertext.size());
	// 	stfDecryptor.MessageEnd();
	// 
	// 	return decryptedtext;
}

void FreeObjectInfo(POBJECT_INFO& pObjInfo)
{
	if (pObjInfo)
		delete[](LPBYTE)pObjInfo;

	pObjInfo = nullptr;
}

void StringToLowHigh(LPCWSTR v, OUT DWORD& low, OUT DWORD& high)
{
	try
	{
		ULONGLONG ulv = boost::lexical_cast<ULONGLONG>(v);
		low = LODWORD(ulv);
		high = HIDWORD(ulv);
	}
	catch (const boost::bad_lexical_cast& /*e*/)
	{
		low = 0;
		high = 0;
	}
}


void DriveObjectToObjectInfo(const DRIVEOBJECT_ST& drvObj, OBJECT_INFO& objInfo, DOLogX* pDoLogX = nullptr)
{
	if (drvObj.oID != NULL)
		wcscpy_s(objInfo.wszObjID, drvObj.oID);
	if (drvObj.parentOID != NULL)
		wcscpy_s(objInfo.wszParnetObjID, drvObj.parentOID);

	if (drvObj.versionStatus != NULL)
		wcscpy_s(objInfo.wszVersionStatus, drvObj.versionStatus);
	if (FILE_TYPE == (OBJECT_TYPE)drvObj.objType &&
		NULL != drvObj.versionCode &&
		L'R' == drvObj.versionCode[0] &&
		L'\0' == objInfo.wszVersionStatus[0])
		wcscpy_s(objInfo.wszVersionStatus, L"R"); // 배포 문서의 파일에는 'R' 값이 없기때문에

	if (drvObj.processStatus != NULL)
		wcscpy_s(objInfo.wszProcessStatus, drvObj.processStatus);
	if (drvObj.creatorInfo != NULL)
		wcscpy_s(objInfo.wszObjCreatorInfo, drvObj.creatorInfo);
	if (drvObj.checkOutInfo != NULL)
		wcscpy_s(objInfo.wszObjCheckOutInfo, drvObj.checkOutInfo);

	if (drvObj.tags != NULL)
	{
		if (wcslen(drvObj.tags) < OBJECT_TAG_MAX_LENGTH)
			wcscpy_s(objInfo.wszObjTags, drvObj.tags);
		else
		{
			wcsncpy_s(objInfo.wszObjTags, drvObj.tags, OBJECT_TAG_MAX_LENGTH - 1);
			objInfo.wszObjTags[OBJECT_TAG_MAX_LENGTH - 1] = L'\0';
		}
	}

	if (drvObj.folderType != NULL)
		wcscpy_s(objInfo.wszFolderType, drvObj.folderType);

	CString csFileName(drvObj.objName);
	DOUtilEncode::AssureValideWinFileName(csFileName);
	if (!csFileName.IsEmpty())
		wcscpy_s(objInfo.wszFileName, csFileName.GetLength() + 1, csFileName);
	objInfo.wFileNameLength = static_cast<WORD>(csFileName.GetLength());

	objInfo.dwFileSizeHigh = drvObj.fileSizeHigh;
	objInfo.dwFileSizeLow = drvObj.fileSizeLow;
	objInfo.ObjType = (OBJECT_TYPE)drvObj.objType;
	objInfo.bOpen = drvObj.flagOpen;

	// 	if (!IsParentRecylebin(drvObj))
	// 		// 휴지통 하위 항목에서 flagOpen는 읽기전용으로 열기가 가능한 단일문서라는 의미로 변경됨.
	// 	{
	// 		if (drvObj.drmType == NULL || drvObj.drmType[0] == L'\0')
	// 		{
	// 			if (drvObj.flagOpen)
	// 				wcscpy_s(objInfo.wszDrmFileState, CHK);
	// 		}
	// 		else
	// 		{
	// 			if (drvObj.flagOpen)
	// 				wcscpy_s(objInfo.wszDrmFileState, DHK);
	// 			else
	// 			{
	// 				wcsncpy_s(objInfo.wszDrmFileState, drvObj.drmType, CHK_SIZE - 1);
	// 				objInfo.wszDrmFileState[CHK_SIZE - 1] = L'\0';
	// 			}
	// 		}
	// 	}

	objInfo.bHaveChild = drvObj.flagHaveChild;
	objInfo.bOthersPrivateFolder = drvObj.othersPrivateFolder;

	objInfo.nTotalCount = drvObj.totalCount;
	if (drvObj.totalSize != NULL)
		StringToLowHigh(drvObj.totalSize, objInfo.dwTotalSizeLow, objInfo.dwTotalSizeHigh);

	// LocalFileTime으로 변환하지 않기로함. 2018.01.30
	//FileTimeToLocalFileTime(drvObj.objCreatedAtLow,        drvObj.objCreatedAtHigh,        objInfo.ftCreatedAt);
	//FileTimeToLocalFileTime(drvObj.lastModifiedAtLow,      drvObj.lastModifiedAtHigh,      objInfo.ftLastModifiedAt);
	//FileTimeToLocalFileTime(drvObj.localLastModifiedAtLow, drvObj.localLastModifiedAtHigh, objInfo.ftLocalLastModifiedAt);
	//FileTimeToLocalFileTime(drvObj.checkOutAtLow,          drvObj.checkOutAtHigh,          objInfo.ftCheckOutAt);

	// DriveObject의 filetime 을 FILETIME 구조체로 대입
	auto AssignFileTimeFromDrvObj = [](DWORD dwLow, DWORD dwHigh, FILETIME &ft)
	{
		ft.dwLowDateTime = dwLow;
		ft.dwHighDateTime = dwHigh;
	};
	AssignFileTimeFromDrvObj(drvObj.objCreatedAtLow, drvObj.objCreatedAtHigh, objInfo.ftCreatedAt);
	AssignFileTimeFromDrvObj(drvObj.lastModifiedAtLow, drvObj.lastModifiedAtHigh, objInfo.ftLastModifiedAt);
	AssignFileTimeFromDrvObj(drvObj.localLastModifiedAtLow, drvObj.localLastModifiedAtHigh, objInfo.ftLocalLastModifiedAt);
	AssignFileTimeFromDrvObj(drvObj.checkOutAtLow, drvObj.checkOutAtHigh, objInfo.ftCheckOutAt);


	objInfo.bManagementGroupOID = drvObj.flagGroupRootFolder;
	objInfo.bExistFavorite = drvObj.existFavorite;
	objInfo.bEnableChangeManagerGroup = drvObj.enableChangeManagerGroup;

	objInfo.dwPermissionProPerties = drvObj.permissionProperties;
	objInfo.dwAclProperties = drvObj.aclProperties;

	// log
	if (pDoLogX && DOUtilConfig::UseObjectListDetailLog())
	{
		CString& csCreatedAt = DOUtil::ToString(objInfo.ftCreatedAt, TIME_FORMAT_DATETIME_GMTZ);
		CString& csLastModifiedAt = DOUtil::ToString(objInfo.ftLastModifiedAt, TIME_FORMAT_DATETIME_GMTZ);
		CString& csLocalLastModifiedAt = DOUtil::ToString(objInfo.ftLocalLastModifiedAt, TIME_FORMAT_DATETIME_GMTZ);
		CString& csCheckoutAt = DOUtil::ToString(objInfo.ftCheckOutAt, TIME_FORMAT_DATETIME_GMTZ);
		pDoLogX->Print(DOLOG_FUNCLINE,
			L"- %s : %s : %s <has_child=%s, ExistFavorite=%s TotalCount = %d, EnableChangeManagerGroup=%s ,"
			L" permission=%08X, acl=%08X, folder_type=%s, Tag=%s, created_at=%s, server_modify_at=%s,"
			L" local_modify_at=%s, chkout_at=%s, process_status=%s, flagOpen=%s, otherprivatefolder=%s,"
			L" drmFileState=%s>",
			objInfo.wszFileName == NULL ? L"<null>" : objInfo.wszFileName,
			objInfo.wszObjID == NULL ? L"<null>" : objInfo.wszObjID,
			DOUtilBase::ToString(objInfo.ObjType),
			DOUtilBase::ToString(objInfo.bHaveChild),
			DOUtilBase::ToString(objInfo.bExistFavorite),
			objInfo.nTotalCount,
			DOUtilBase::ToString(objInfo.bEnableChangeManagerGroup),
			objInfo.dwPermissionProPerties,
			objInfo.dwAclProperties,
			objInfo.wszFolderType,
			objInfo.wszObjTags,
			(LPCWSTR)csCreatedAt,
			(LPCWSTR)csLastModifiedAt,
			(LPCWSTR)csLocalLastModifiedAt,
			(LPCWSTR)csCheckoutAt,
			objInfo.wszProcessStatus,
			DOUtilBase::ToString(objInfo.bOpen),
			DOUtilBase::ToString(objInfo.bOthersPrivateFolder),
			objInfo.wszDrmFileState);
	}
}


void DriveObjectToObjectInfo(const DRIVEOBJECT_ST& drvObj, POBJECT_INFO& pObjInfo, DOLogX* pDoLogX = nullptr)
{
	size_t objectLength = sizeof(OBJECT_INFO);
	if (drvObj.objName)
		objectLength += sizeof(wchar_t) * wcslen(drvObj.objName);
	pObjInfo = (POBJECT_INFO)new BYTE[objectLength];
	ZeroMemory(pObjInfo, objectLength);

	DriveObjectToObjectInfo(drvObj, *pObjInfo, pDoLogX);
}

void DriveObjectToObjectInfo2(const DRIVEOBJECT_ST& drvObj, OBJECT_INFO*& pObjInfo, UINT& objInfoSize, DOLogX* pDoLogX = nullptr)
{
	if (!pObjInfo)
	{
		size_t objectLength = sizeof(OBJECT_INFO);
		objectLength += 256 * sizeof(WCHAR);

		pObjInfo = (POBJECT_INFO)new BYTE[objectLength];
		ZeroMemory(pObjInfo, objectLength);

		objInfoSize = (int)objectLength;
	}

	size_t objectLengthNeeded = sizeof(OBJECT_INFO);

	if (drvObj.objName)
		objectLengthNeeded += sizeof(WCHAR) * wcslen(drvObj.objName);

	if (objectLengthNeeded > objInfoSize)
	{
		FreeObjectInfo(pObjInfo);

		pObjInfo = (POBJECT_INFO)new BYTE[objectLengthNeeded];
		ZeroMemory(pObjInfo, objectLengthNeeded);

		objInfoSize = (int)objectLengthNeeded;
	}
	else
	{
		ZeroMemory(pObjInfo, objInfoSize);
	}

	DriveObjectToObjectInfo(drvObj, *pObjInfo, pDoLogX);
}

//////////////////////////////////////////////////////////////////////////
// [21668] <oid = 1MJqSSSmzJL, type = folder / 1, sort = 32, page_index = 1, request_obj_types = 4103, request_mode = 4, dwListCount = 100, filterType = 0, lpcwfilter = >

DWORD _DS_GetObjectList(IN LPCWSTR lpcwObjID,
	IN OBJECT_TYPE ObjectType,
	IN WORD wSortType,
	IN DWORD dwPageIndex,
	OUT PVOID &pObjectBuffer,
	IN OUT DWORD &dwListCount,
	OUT int &nBufferSize,
	IN DWORD dwRequestObjTypes,
	IN DWORD dwRequestMode,
	IN DWORD filterType,
	IN LPCWSTR lpcwfilter,
	OUT int* pnFolderCount = nullptr,
	OUT int* pnDocumentCount = nullptr)
{
	BOOL bToBePrintLog = DOUtilConfig::IsToBePrintLog(PrintLogDetail_ObjectList);
	DOLogX doLogX(bToBePrintLog, nullptr, DOLOG_FUNCLINE);
	doLogX.Print(DOLOG_FUNCLINE, L"<oid=%s, type=%s, sort=%d, page_index=%d, request_obj_types=%d, request_mode=%d>",
		lpcwObjID ? lpcwObjID : L"",
		DOUtilBase::ToString(ObjectType),
		wSortType,
		dwPageIndex,
		dwRequestObjTypes,
		dwRequestMode);

	// 	if (!theApp.IsCompatibleStruct(STRUCT_OBJECT_INFO))
	// 	{
	// 		return doLogX.Result(DOLOG_FUNCLINE, (TYPE_DO_ERROR)RTN_INVALID_PARAMETER, L"IsCompatibleStruct Fail!!!");
	// 	}

	DWORD rtn = RTN_STATUS_FAIL;
	DRIVEOBJECTLIST_ST* DOLST = NULL;
	CObjectList	ObjList;

	POBJECT_INFO pObjInfo = nullptr;
	UINT objectinfoSize = 0;

	POBJECT_INFO pObjInfoChild = nullptr;
	UINT objectinfoChildSize = 0;

	//	DOXCMD_LOGIN();

	if (LINK_FOLDER_TYPE == ObjectType || LINK_DOCUMENT_TYPE == ObjectType)
	{
		OBJECT_TYPE RealObjectType;
		WCHAR wszRealObjID[OBJECT_ID_LENGTH];
		ZeroMemory(wszRealObjID, sizeof(wszRealObjID));
		if (RTN_STATUS_SUCCESS != g_msgpackClient->GetRealObjectIDAndObjTypeAboutLinkObject(doLogX, lpcwObjID, ObjectType, wszRealObjID, RealObjectType))
		{
			rtn = RTN_STATUS_FAIL;
			goto clear;
		}

		DOLST = g_msgpackClient->GetObjectList(doLogX, wszRealObjID, RealObjectType, wSortType, dwPageIndex, dwListCount, dwRequestObjTypes, dwRequestMode, filterType, lpcwfilter);
	}
	else
	{
		DOLST = g_msgpackClient->GetObjectList(doLogX, lpcwObjID, ObjectType, wSortType, dwPageIndex, dwListCount, dwRequestObjTypes, dwRequestMode, filterType, lpcwfilter);
	}

	if (NULL == DOLST)
	{
		rtn = RTN_STATUS_FAIL;
		goto clear;
	}

	if (nullptr != pnFolderCount && nullptr != pnDocumentCount)
	{
		doLogX.Print(DOLOG_FUNCLINE, L"[folderTableCount :%d][tableCount : %d]", DOLST->folderTableCount, DOLST->tableCount);
		*pnFolderCount = DOLST->folderTableCount;
		*pnDocumentCount = DOLST->tableCount;
	}



	for (int i = 0; i < DOLST->driveObjectsCNT; i++)
	{
		const DRIVEOBJECT_ST& DO_ST = DOLST->driveObjects[i];

		doLogX.Print(DOLOG_FUNCLINE, L"[%d]oID=%s,parentOID=%s,sourceOID=%s,storageFileID=%s,versionStatus=%s,docStatus=%s,tags=%s,versionCode=%s,checkOutInfo=%s,creatorInfo=%s,drmType=%s,folderType=%s,objName=%s,processStatus=%s,totalSize=%s,ACL-Property=%d, permission=%d"
			, i, DO_ST.oID, DO_ST.parentOID, DO_ST.sourceOID, DO_ST.storageFileID,
			DO_ST.versionStatus, DO_ST.docStatus, DO_ST.tags, DO_ST.versionCode, DO_ST.checkOutInfo, DO_ST.creatorInfo, DO_ST.drmType, DO_ST.folderType, DO_ST.objName
			, DO_ST.processStatus, DO_ST.totalSize, DO_ST.aclProperties, DO_ST.permissionProperties);

		DriveObjectToObjectInfo2(DO_ST, pObjInfo, objectinfoSize, &doLogX);

		if (DO_ST.childObjectsCNT > 0)
		{
			if (dwRequestMode == REQ_MODE_INCLUDE_DOCUMENT_FILE)
			{
				CObjectList	ChildObjList;

				for (int i = 0; i < DO_ST.childObjectsCNT; i++)
				{
					const DRIVEOBJECT_ST& DO_STChild = DO_ST.childObjects[i];

					DriveObjectToObjectInfo2(DO_STChild, pObjInfoChild, objectinfoChildSize, &doLogX);

					if (FALSE == ChildObjList.AddObject(pObjInfoChild))
					{
						ChildObjList.FreeListBuffer();
						free(pObjInfoChild);
						doLogX.PrintError(DOLOG_FUNCLINE, L"ChildObjList.AddObject Fail!!!");
						break;
					}
				}

				pObjInfo->nChildFileObjListCount = ChildObjList.GetCount();
				pObjInfo->nChildBufferSize = ChildObjList.GetDataSize();
				pObjInfo->pChildFileObjInfo = (POBJECT_INFO)ChildObjList.GetBuffer();

			}
			else
			{
				doLogX.PrintError(DOLOG_FUNCLINE, L"요청 형식이 REQ_MODE_INCLUDE_DOCUMENT_FILE 이 아닙니다.");
			}
		}

		if (FALSE == ObjList.AddObject(pObjInfo))
		{
			//			FreeObjectInfo(pObjInfo);

			doLogX.PrintError(DOLOG_FUNCLINE, L"ObjList.AddObject Fail!!!");
			rtn = RTN_INSUFFICENT_BUFFER;
			goto clear;
		}
	}

	// Object 추가가 전부 성공 할 경우에는 CObjectList의 FreeListBuffer를 호출 하지 말고
	// GetBuffer을 통하여 할당 된 Buffer의 주소와 추가 된 Object List 수, 할당된 Buffer Size
	// 를 전달 한다.
	pObjectBuffer = ObjList.GetBuffer();
	dwListCount = ObjList.GetCount();
	nBufferSize = ObjList.GetDataSize();

	rtn = RTN_STATUS_SUCCESS;

clear:

	FreeObjectInfo(pObjInfo);
	FreeObjectInfo(pObjInfoChild);

	if (DOLST)
		DestroyDRIVEOBJECTLIST_ST(DOLST);

	return doLogX.Result(DOLOG_FUNCLINE, (TYPE_DO_ERROR)rtn, L"");
}


void CMainDlg::TestCodeForFunction()
{
	// 	// 	char* pTest = { "1234567891234567912345679" };
	// 	// 
	// 	// 	std::string strEnc;
	// 	// 	strEnc = AESEncryptionNEncode2(pTest);
	// 	// 
	// 	// 	std::string strDec;
	// 	// 	strDec = AESDecryptionNEncode(strEnc);
	// 	// 암복호화 테스트 [8/17/2018 ksjin]
	// 
	// 	CDoSiteKeyMgr SiteKeyMgr;
	// 	SiteKeyMgr.GetEncryptionKey();
	// 	//	SiteKeyMgr.GetKeyAndIV(key, iv);
	// 
	// 	CDoEncMgr AesDecMgr(SiteKeyMgr.GetSiteKeyPtr(), SiteKeyMgr.GetSiteKeySize(), ENCMGR_MODE_DEC);
	// 
	// 	HANDLE h = ::CreateFile(L"D:\\다운로드\\e0a831ce1d26c1307874c7e1629fa5ad60fca03ee229fea6.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	// 
	// 	HANDLE h2 = ::CreateFile(L"D:\\다운로드\\test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// 
	// 	if (h != INVALID_HANDLE_VALUE)
	// 	{
	// 		DWORD dwSize = ::GetFileSize(h, nullptr);
	// 
	// 		std::vector<BYTE> fileReadBuffer(dwSize);
	// 
	// 		DWORD dwReaded = 0;
	// 		::ReadFile(h, &fileReadBuffer[0], dwSize, &dwReaded, NULL);
	// 
	// 
	// 		DWORD dwDecSize = 0;
	// 
	// 		::WriteFile(h2, &fileReadBuffer[0], 11808, &dwDecSize, NULL);
	// 
	// 		CloseHandle(h2);
	// 
	// 		// 		try
	// 		// 		{
	// 		// 			std::vector<BYTE> decBuffer((dwSize/16)*16 + 16);
	// 		// 
	// 		// 			
	// 		// //			AesDecMgr.DecryptBuffersWithFinalize(&fileReadBuffer[0], dwReaded, &decBuffer[0], (dwSize / 16) * 16 + 16, dwDecSize);
	// 		// 
	// 		// 			AesDecMgr.DecryptBuffers(&fileReadBuffer[0], dwReaded, &decBuffer[0], (dwSize / 16) * 16 + 16, dwDecSize);
	// 		// 
	// 		// 		}
	// 		// 		catch (const Exception& e)
	// 		// 		{
	// 		// 			int i = e.GetErrorType();
	// 		// 
	// 		// 			OutputDebugStringW(L"11");
	// 		// 		}
	// 
	// 		CloseHandle(h);
	// 	}
	// 
	// 
	// 	//	DOAMessageBoxApi::DoShowMessageBoxModelessByMsgAgent(NULL,  (LPWSTR)(LPCWSTR)L"	                                원본 문서가 삭제되어서 조회할 수 없습니다.");
	// 	//	DOAMessageBoxApi::DoShowMessageBoxModelessByMsgAgent(NULL,  (LPWSTR)(LPCWSTR)L"	                                원본 문서가 삭제되어서 조회할 수 없습니다.	                                원본 문서가 삭제되어서 조회할 수 없습니다.	                                원본 문서가 삭제되어서 조회할 수 없습니다.	                                원본 문서가 삭제되어서 조회할 수 없습니다.");
	// 
	// 
	// 	// 	DOAMessageBoxApi::DoShowMessageBoxModelessByMsgAgent(NULL,  
	// 	// 		(LPWSTR)DOResMsg::GetMsg(COM_ECMAGENT, L"EACannotAddFileToServer2"), 
	// 	// 		CTRL_MESSAGE_MODE_ERROR, SHOWMSG_SINGLE_RANGE_DOWNLOAD);
	// 	// 
	// 	// 	CString str = L"E:\\Temp\\view\\1MHkMTKg5jB\\\\입금전용계좌사용현황(2018.06월)_1.xlsx";
	// 	// 
	// 	// 	BOOL bTest = DOUtilFile::IsExistedFilePath(str);
	// 	// 
	// 	// 	std::wstring metaFilePath;
	// 	// 	metaFilePath = L"1231221";
	// 	// 	CString csFilePath = L"21312";
	// 	// 
	// 	// 	doLogX.PrintError(DOLOG_FUNCLINE,
	// 	// 		L"metaFile.MoveTmFileToWm Failed!!! <metapath=%s, filepath=%s>",
	// 	// 		metaFilePath.c_str(), csFilePath);
	// 
	// 	CString csUpdateIP, csUpdateContextPath, csRpcIP, csRpcContextPath, csStorageIP, csStorageContextPath, strUseSSL, csTenantID;
	// 	int nUpdatePort = 443, nRpcPort = 555, nStoragePort = 164;
	// 
	// 	csUpdateIP = L"192.168.1.1";
	// 	csUpdateContextPath = L"111111111111111111111";
	// 	csRpcIP = L"192.168.1.1";
	// 	csRpcContextPath = L"2222222222222222222";
	// 	csStorageIP = L"192.168.1.1";
	// 	csStorageContextPath = L"444444444444444444";
	// 	strUseSSL = L"true";
	// 	csTenantID = L"ksjin";
	// 
	// // 	DOUtilConfig::SaveServerInfoToFile(csUpdateIP, nUpdatePort, csUpdateContextPath,
	// // 		csRpcIP, nRpcPort, csRpcContextPath,
	// // 		csStorageIP, nStoragePort, csStorageContextPath,
	// // 		strUseSSL, csTenantID);
	// // 
	// // 	DOUtilConfig::LoadServerInfoFromFile(csUpdateIP, nUpdatePort, csUpdateContextPath,
	// // 		csRpcIP, nRpcPort, csRpcContextPath,
	// // 		csStorageIP, nStoragePort, csStorageContextPath,
	// // 		strUseSSL, csTenantID);


	// 	DWORD _DS_GetObjectList(IN LPCWSTR lpcwObjID,
	// 		IN OBJECT_TYPE ObjectType,
	// 		IN WORD wSortType,
	// 		IN DWORD dwPageIndex,
	// 		OUT PVOID &pObjectBuffer,
	// 		IN OUT DWORD &dwListCount,
	// 		OUT int &nBufferSize,
	// 		IN DWORD dwRequestObjTypes,
	// 		IN DWORD dwRequestMode,
	// 		IN DWORD filterType,
	// 		IN LPCWSTR lpcwfilter,
	// 		OUT int* pnFolderCount = nullptr,
	// 		OUT int* pnDocumentCount = nullptr)

	//////////////////////////////////////////////////////////////////////////
	// [21668] <oid = 1MJqSSSmzJL, type = folder / 1, sort = 32, page_index = 1, request_obj_types = 4103, request_mode = 4, dwListCount = 100, filterType = 0, lpcwfilter = >
	// 1MJgTgJbASi

	CString strOid = L"1MJgTgJbASi";

	INT nBufferSize = 0;
	DWORD dwListCount = 100;
	PVOID pObjectBuffer = NULL;
	_DS_GetObjectList(strOid,
		(OBJECT_TYPE)1,
		32, 1,
		pObjectBuffer,
		dwListCount,
		nBufferSize,
		4103,
		4,
		0,
		L"");

}

void TestCodeForFunction()
{
	// 	char* pTest = { "1234567891234567912345679" };
	// 
	// 	std::string strEnc;
	// 	strEnc = AESEncryptionNEncode2(pTest);
	// 
	// 	std::string strDec;
	// 	strDec = AESDecryptionNEncode(strEnc);
	// 암복호화 테스트 [8/17/2018 ksjin]

	CDoSiteKeyMgr SiteKeyMgr;
	SiteKeyMgr.GetEncryptionKey();
	//	SiteKeyMgr.GetKeyAndIV(key, iv);

	CDoEncMgr AesDecMgr(SiteKeyMgr.GetSiteKeyPtr(), SiteKeyMgr.GetSiteKeySize(), ENCMGR_MODE_DEC);

	HANDLE h = ::CreateFile(L"D:\\다운로드\\e0a831ce1d26c1307874c7e1629fa5ad60fca03ee229fea6.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	HANDLE h2 = ::CreateFile(L"D:\\다운로드\\test.dat", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (h != INVALID_HANDLE_VALUE)
	{
		DWORD dwSize = ::GetFileSize(h, nullptr);

		std::vector<BYTE> fileReadBuffer(dwSize);

		DWORD dwReaded = 0;
		::ReadFile(h, &fileReadBuffer[0], dwSize, &dwReaded, NULL);


		DWORD dwDecSize = 0;

		::WriteFile(h2, &fileReadBuffer[0], 11808, &dwDecSize, NULL);

		CloseHandle(h2);

		// 		try
		// 		{
		// 			std::vector<BYTE> decBuffer((dwSize/16)*16 + 16);
		// 
		// 			
		// //			AesDecMgr.DecryptBuffersWithFinalize(&fileReadBuffer[0], dwReaded, &decBuffer[0], (dwSize / 16) * 16 + 16, dwDecSize);
		// 
		// 			AesDecMgr.DecryptBuffers(&fileReadBuffer[0], dwReaded, &decBuffer[0], (dwSize / 16) * 16 + 16, dwDecSize);
		// 
		// 		}
		// 		catch (const Exception& e)
		// 		{
		// 			int i = e.GetErrorType();
		// 
		// 			OutputDebugStringW(L"11");
		// 		}

		CloseHandle(h);
	}


	//	DOAMessageBoxApi::DoShowMessageBoxModelessByMsgAgent(NULL,  (LPWSTR)(LPCWSTR)L"	                                원본 문서가 삭제되어서 조회할 수 없습니다.");
	//	DOAMessageBoxApi::DoShowMessageBoxModelessByMsgAgent(NULL,  (LPWSTR)(LPCWSTR)L"	                                원본 문서가 삭제되어서 조회할 수 없습니다.	                                원본 문서가 삭제되어서 조회할 수 없습니다.	                                원본 문서가 삭제되어서 조회할 수 없습니다.	                                원본 문서가 삭제되어서 조회할 수 없습니다.");


	// 	DOAMessageBoxApi::DoShowMessageBoxModelessByMsgAgent(NULL,  
	// 		(LPWSTR)DOResMsg::GetMsg(COM_ECMAGENT, L"EACannotAddFileToServer2"), 
	// 		CTRL_MESSAGE_MODE_ERROR, SHOWMSG_SINGLE_RANGE_DOWNLOAD);
	// 
	// 	CString str = L"E:\\Temp\\view\\1MHkMTKg5jB\\\\입금전용계좌사용현황(2018.06월)_1.xlsx";
	// 
	// 	BOOL bTest = DOUtilFile::IsExistedFilePath(str);
	// 
	// 	std::wstring metaFilePath;
	// 	metaFilePath = L"1231221";
	// 	CString csFilePath = L"21312";
	// 
	// 	doLogX.PrintError(DOLOG_FUNCLINE,
	// 		L"metaFile.MoveTmFileToWm Failed!!! <metapath=%s, filepath=%s>",
	// 		metaFilePath.c_str(), csFilePath);

	CString csUpdateIP, csUpdateContextPath, csRpcIP, csRpcContextPath, csStorageIP, csStorageContextPath, strUseSSL, csTenantID;
	int nUpdatePort = 443, nRpcPort = 555, nStoragePort = 164;

	csUpdateIP = L"192.168.1.1";
	csUpdateContextPath = L"111111111111111111111";
	csRpcIP = L"192.168.1.1";
	csRpcContextPath = L"2222222222222222222";
	csStorageIP = L"192.168.1.1";
	csStorageContextPath = L"444444444444444444";
	strUseSSL = L"true";
	csTenantID = L"ksjin";

	// 	DOUtilConfig::SaveServerInfoToFile(csUpdateIP, nUpdatePort, csUpdateContextPath,
	// 		csRpcIP, nRpcPort, csRpcContextPath,
	// 		csStorageIP, nStoragePort, csStorageContextPath,
	// 		strUseSSL, csTenantID);
	// 
	// 	DOUtilConfig::LoadServerInfoFromFile(csUpdateIP, nUpdatePort, csUpdateContextPath,
	// 		csRpcIP, nRpcPort, csRpcContextPath,
	// 		csStorageIP, nStoragePort, csStorageContextPath,
	// 		strUseSSL, csTenantID);
}
#endif
// 진책임 테스트 구간 끝 [9/4/2018 ksjin]
//////////////////////////////////////////////////////////////////////////
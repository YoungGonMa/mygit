#include "StdAfx.h"
#include "DestinyOneAgent.h"
#include "TestAgent.h"

#include "windows.h"

#include <libcmn/str.h>
#include <libcmn/proc.h>
#include <boost/progress.hpp>

#include <DestinyMsgpackRpc/auto_generated_files/ClientDataType.h>


#pragma region HiDPI Test
//#	include <LibDOCtrl/DOCtrlDlg.h>
//#	include <LibDOCtrl/DOCtrlDlgAsk.h>
//#	include <LibDOCtrl/DOCtrlDlgFileEdit.h>
//#	include <LibDOCtrl/DOCtrlDlgMessage.h>
//#	include <LibDOCtrl/DOCtrlDlgSelOpenMode.h>
//#	include <LibDOCtrl/DOCtrlDlgUploadMulti.h>
//#	include <LibDOCtrl/DOCtrlDlgUploadSingle.h>
//#	include <LibDOCtrl/DOCtrlDlgWarning.h>
#	include <LibDOCtrl/DOCtrlEditReadOnly.h>
//#	include <LibDOCtrl/DOCtrlDlgFileError.h>
//#	include "ChangePWDlg.h"
//#	include "OverwriteDlg.h"
//#	include "ReFileNameDlg.h"
//#	include "Transfer/DlgFileTransfer.h"
//#	include "LibDOCtrl/AvoidDuplicationDlg.h"
//#	include "LibDOCtrl/MultiUploadOptDlg.h"
#	include <utility>
//#	include "IDCheckDlg.h"
#pragma endregion HiDPI Test

#pragma region SessionChecker Test
#include <ppl.h>
#include "SessionChecker.h"
#include "AppStatus.h"
#include "DOWorkerOA.h"
#pragma endregion SessionChecker Test

#include "Dialog/DoNotSavedDocListDlg.H"

#pragma region Shutdown Application Test
#include <LibDOCore/DOUtilReg.h>
#include <LibDOInterface/DOPipeClientString.h>
#include <ShareCtrl/DOMessageBox.h>
#pragma endregion Shutdown Application Test

#include <libcmn/file.h>
#include <libcmn/cnv.h>
#include <libcmn/cmn.h>
#include <libcmn/shell.h>

#pragma region Download Decision Test
#include <DownloadDecision/DownloadDecision.h>
#include <LibDOTransfer/DOTDownloadingList.h>
#pragma endregion Download Decision Test

#pragma region Multiple Execute At SameTime Test
#include <DestinyOneAgent.h>
#include <LibDOBase/DOUtilEncode.h>
#include <LibDOBase/DOLocalMetaFile.h>

#include "TrayIcon.h"
#include "LibDOInterface/DestinyDMException.h"

extern CDestinyOneAgentApp theApp;
#pragma endregion Multiple Execute At SameTime Test

#pragma region RunCommand Test
#include "LibDOBase/DOUtilProcess.h"
#pragma endregion RunCommand Test

#pragma region WebDialog Test
#	include <ONEConnector.h>
#	include "../../common/share/Dialog/DoPCTakeOutHtmlDlg.h"
#	include "../../common/share/Dialog/DoExternalExportHtmlDlg.h"
#	include <SelectFolderDlg.h>
#	include <SaveDocumentToServerDlg.h>
#pragma endregion WebDialog Test

#pragma region PCFileBackup Test
#	include "PCFileBackup.h"
#	include "PCFileCollect.h"
#pragma endregion PCFileBackup Test

#pragma region WebView2 Test
#	include "ViewPropertiesDlg_WebView2.h"
#pragma endregion WebView2 Test

#include "NotificationCenterDlg.h"

#include <../../common/share/Dialog/DOAMessageBox.h>

using namespace std;

namespace
{

	CString GetValueFromConfig(const YAML::Node& testCfg, char *propertyName)
	{
		string stdStr = "";
		if (testCfg[propertyName])
		{
			if (testCfg[propertyName].IsNull())
				return L"";
			stdStr = testCfg[propertyName].as<string>();
		}
		return CString(stdStr.c_str());

	}

};

TestAgent::TestAgent(const fs::path& cfgFile)
{
	testCfg_ = YAML::LoadFile(cfgFile.string());
}

TestAgent::~TestAgent()
{
}

static void
logLines(const std::string& name, const int locToWrite)
{
	std::wostringstream oss;
	oss << L"---------------------------------------------------" << std::endl;
	for (int j = 0; j < locToWrite; j++)
	{
		oss << CA2W(name.c_str()) << L" .... line count:" << std::dec << (j + 1) << std::endl;
	}
	oss << L"--------------------------------------------------------------" << std::endl;
	DOLog::PrintLog(DOLOG_FUNCLINE, oss.str().c_str());
}

static void
createResultHeader(const fs::path& resultFilePath)
{
	std::string header(
		     "start_time,"
		     "log_method,"
		  "loc_to_Write1,"
		   "job1_time(s),"
		  "loc_to_Write2,"
		   "job2_time(s),"
		  "loc_to_Write3,"
		"elapsed_time(s),"
		"delayed_job_time_by_log(s)\n"
	);

	cmn::str::saveToTxtFile(resultFilePath, header);
}

static void
testJob(const YAML::Node& job, const fs::path& testDir, const std::string& logMethod)
{
	std::string startTime = cmn::str::now_iso8601_with_msecs();
	cmn::str::replaceAll(startTime, "T", "_");

	int job1TimeSec = job["job1_time(s)"].as<int>();
	int job2TimeSec = job["job2_time(s)"].as<int>();
	int locToWrite1 = job["loc_to_write1"].as<int>();
	int locToWrite2 = job["loc_to_write2"].as<int>();
	int locToWrite3 = job["loc_to_write3"].as<int>();

	std::ostringstream os;
	{
		boost::progress_timer t(os);
		{
			logLines("Write1", locToWrite1);
			::Sleep(job1TimeSec * 1000);
			logLines("Write2", locToWrite2);
			::Sleep(job2TimeSec * 1000);
			logLines("Write3", locToWrite3);
		}
	}

	std::string elapsedTime = os.str();
	cmn::str::replaceAll(elapsedTime, " s", "");
	cmn::str::trimRight(elapsedTime);
	float fElapsedTime = cmn::str::toFloat(elapsedTime);
	float fNetLogTime = fElapsedTime - float(job1TimeSec + job2TimeSec);

	std::string resultRecord = fmt::format("{},{},{},{},{},{},{},{},{}\n",
		startTime, logMethod,
		locToWrite1,
		job1TimeSec,
		locToWrite2,
		job2TimeSec,
		locToWrite3,
		elapsedTime, fNetLogTime);
	fs::path resultPath = testDir.wstring() + L"/result.csv";
	if (!DOUtilFile::IsExistedFilePath(resultPath.wstring().c_str()))
		createResultHeader(resultPath);

	cmn::str::saveToTxtFile(resultPath, resultRecord, true);
}

IMPLEMENT_TESTAGENTFUNC(hidpi)
{
	try
	{
        const YAML::Node& cfg = testCfg["config"];
	    YAML::Node& job = const_cast<YAML::Node& >(cfg["job"]);

        YAML::Node& targetNode = job["DOCtrlDlgWarning"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             DOCtrlDlgWarning warn;
// 	        warn.DoModal();
//         }

//         targetNode = job["DOCtrlDlgWarningSmall"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             DOCtrlDlgWarning warn(DOResMsg::GetMsg(COM_ECMAGENT, L"EAWritingTransferFileList"), true);
// 	        warn.DoModal();
//         }
		
//         targetNode = job["DOCtrlDlgSelOpenMode"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             DOCtrlDlgSelOpenMode doCtrlDlgSelOpenMode;
// 	        doCtrlDlgSelOpenMode.DoModal();
//         }

//         targetNode = job["DOCtrlDlgFileEdit"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             DOCtrlDlgFileEdit doCtrlDlgFileEdit;
// 	        doCtrlDlgFileEdit.DoModal();
//         }

//         targetNode = job["DOCtrlDlgUploadSingle"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             DOCtrlDlgUploadSingle doCtrlDlgUploadSingle( FALSE, L"test", FALSE, NULL);
// 	        doCtrlDlgUploadSingle.DoModal();
//         }

//         targetNode = job["DOCtrlDlgAsk"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             /*DOCtrlDlgAsk doCtrlDlgAsk(DOT_ANSWER_OK, L"test", DOT_ANSWER_OK, NULL, FALSE);
// 	        doCtrlDlgAsk.DoModal();*/
// 
//             CString csMessage;
//             csMessage.Format(DOResMsg::GetMsg(COM_ECMAGENT, L"EAAlreadyExistFolder"), L"testfile.xyz");
//             // TYPE_DOT_ANSWER typeAnswers = (TYPE_DOT_ANSWER)(DOT_ANSWER_SKIP | DOT_ANSWER_OVERWRITE | DOT_ANSWER_CANCEL);
//             TYPE_DOT_ANSWER typeAnswers = (TYPE_DOT_ANSWER)(DOT_ANSWER_RENAME | DOT_ANSWER_CANCEL);
//             DOCtrlDlgAsk doCtrlDlgAsk(typeAnswers, csMessage, DOT_ANSWER_OVERWRITE);
//             doCtrlDlgAsk.DoModal();
//         }

//        targetNode = job["DOCtrlDlgFileError"];
//        if (targetNode.IsDefined() && targetNode.as<bool>())
//        {
//             DOCtrlDlgFileError::TypeFileJobMode typeFileJobMode = DOCtrlDlgFileError::FileJobModeAttach;
// 	        DOCtrlDlgFileError::TypeFileUploadError typeFileUploadError = DOCtrlDlgFileError::FileUploadErrorCheckOutByOtherUser;				
// 	        DOCtrlDlgFileError doCtrlDlgFileError(typeFileJobMode, typeFileUploadError, NULL );
// 	        doCtrlDlgFileError.DoModal();
//         }

//         targetNode = job["DOCtrlDlgMessage"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             S_CTRL_MESSAGE_INFO* pMsgParam = new S_CTRL_MESSAGE_INFO;;
// 	        TYPE_SHOWMSG_SINGLE_RANGE typeShowSingleRange = SHOWMSG_SINGLE_RANGE_NONE;
// 	        TYPE_CTRL_MESSAGE_MODE typeCtrlMessage = CTRL_MESSAGE_MODE_INFO;
// 	        CString strNoDatFileMsg = L"설치가 완료되었습니다."
//                                       L"\r\n시스템을 정상적으로 사용하기 위해서는 컴퓨터를 재시작해야 합니다."
//                                       L"\r\n\r\n작업 중인 모든 내용을 저장한 후 확인 버튼을 누르십시오";
// 	        pMsgParam->csMsg = strNoDatFileMsg;
// 	        pMsgParam->dwButtonFlags = cfg["mb_type"].as<int>();
// 	        pMsgParam->dwRet = IDCANCEL;
// 	        pMsgParam->hWndDlg = NULL;
// 	        pMsgParam->typeShowMsgSingleRange = typeShowSingleRange;
//             pMsgParam->fHideSysMenu = TRUE;
// 	        DOCtrlDlgMessage doCtrlDlgMessage(*pMsgParam, NULL);
// 	        doCtrlDlgMessage.DoModal();
//             delete pMsgParam;
//         }
// 
//         targetNode = job["DOCtrlDlgMessageS"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             S_CTRL_MESSAGE_INFO* pMsgParam = new S_CTRL_MESSAGE_INFO;;
// 	        TYPE_SHOWMSG_SINGLE_RANGE typeShowSingleRange = SHOWMSG_SINGLE_RANGE_NONE;
// 	        TYPE_CTRL_MESSAGE_MODE typeCtrlMessage = CTRL_MESSAGE_MODE_INFO;
// 	        CString strNoDatFileMsg = L"설치가선택한 항목을 삭제 하시겠습니까?";
// 	        pMsgParam->csMsg = strNoDatFileMsg;
// 	        pMsgParam->dwButtonFlags = cfg["mb_type"].as<int>();
// 	        pMsgParam->dwRet = IDCANCEL;
// 	        pMsgParam->hWndDlg = NULL;
// 	        pMsgParam->typeShowMsgSingleRange = typeShowSingleRange;
//             pMsgParam->fHideSysMenu = FALSE;
// 	        DOCtrlDlgMessage doCtrlDlgMessage(*pMsgParam, NULL);
// 	        doCtrlDlgMessage.DoModal();
//             delete pMsgParam;
//         }

//         targetNode = job["DOCtrlDlgMessageL"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             S_CTRL_MESSAGE_INFO* pMsgParam = new S_CTRL_MESSAGE_INFO;;
// 	        TYPE_SHOWMSG_SINGLE_RANGE typeShowSingleRange = SHOWMSG_SINGLE_RANGE_NONE;
// 	        TYPE_CTRL_MESSAGE_MODE typeCtrlMessage = CTRL_MESSAGE_MODE_INFO;
// 	        CString strNoDatFileMsg = L"설치가선택한 항목을 삭제 하시겠습니까?"
//                                       L"\n\n01 테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구"
//                                       L"\n02 테스트문구테스트문구테스트문구테스트문구테스트문구"
//                                       L"\n03 테스트문구테스트문구테스트문구테스트문구테스트문구"
//                                       L"\n04 테스트문구테스트문구테스트문구테스트문구테스트문구"
//                                       L"\n05 테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구테스트문구";
// 	        pMsgParam->csMsg = strNoDatFileMsg;
// 	        pMsgParam->dwButtonFlags = cfg["mb_type"].as<int>();
// 	        pMsgParam->dwRet = IDCANCEL;
// 	        pMsgParam->hWndDlg = NULL;
// 	        pMsgParam->typeShowMsgSingleRange = typeShowSingleRange;
//             pMsgParam->fHideSysMenu = FALSE;
// 	        DOCtrlDlgMessage doCtrlDlgMessage(*pMsgParam, NULL);
// 	        doCtrlDlgMessage.DoModal();
//             delete pMsgParam;
//         }

      /*  targetNode = job["DOCtrlDlgSelSaveMode"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
            SelSaveModeDlg doCtrlDlgSelSaveMode;
	        doCtrlDlgSelSaveMode.DoModal();
        }*/

//         targetNode = job["DOCtrlDlgUploadMulti"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             CString s(L"aaa");
//             DODelimiterStrUnpack doDelStrUnpack(s);
// 	        DOCtrlDlgUploadMulti doCtrlDlgUploadMulti(&doDelStrUnpack);
// 	        doCtrlDlgUploadMulti.DoModal();
//         }
// 
//         targetNode = job["CChangePWDlg"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             CChangePWDlg changePWDlg;
//             changePWDlg.m_bUseCancel = TRUE;
//             changePWDlg.DoModal();
//         }
// 
//         targetNode = job["CChangePWDlg_NoCancelButton"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             CChangePWDlg changePWDlg;
//             changePWDlg.m_bUseCancel = FALSE;
//             changePWDlg.DoModal();
//         }

        targetNode = job["CDocListDlg"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
            CDoNotSavedDocListDlg::ShowDocListDlg(false);
            // CDocListDlg docListDlg;
            // docListDlg.DoModal();
        }

//         targetNode = job["CIDCheckDlg"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             CIDCheckDlg idCheckDlg;
//             idCheckDlg.DoModal();
//         }

        targetNode = job["CExternalExportDlg"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
            g_mainDlg->PostMessage(WM_COMMAND, ID_MESSAGE_EXTERNALEXPORTAPPROVE, 0);
        }

        targetNode = job["CMessageDlg"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
            g_mainDlg->PostMessage(WM_COMMAND, ID_MESSAGE, 0);
        }

//         targetNode = job["COverwriteDlg"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             COverwriteDlg overwriteDlg;
//             overwriteDlg.m_csDlgText = L"동일한 파일 이름이 해당 문서에서 발견되었습니다.";
//             overwriteDlg.DoModal();
//         }

        targetNode = job["CPCTakeOutDlg"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
            // CPCTakeOutDlg
            g_mainDlg->PostMessage(WM_COMMAND, ID_MESSAGE_PCTAKEOUTREQUEST,0);
        }

//         targetNode = job["ReFileNameDlg"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             std::map<CString,CString> m;
//             m.insert(std::make_pair<CString, CString>(L"test1", L"testA"));
//             m.insert(std::make_pair<CString, CString>(L"test2", L"testB"));
//             ReFileNameDlg reFileNameDlg(m);
//             reFileNameDlg.DoModal();
//         }
        // 

//         targetNode = job["CAvoidDuplicationDlg"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             AVOID_DUPLICATION ad;
//             ad.csNewName = "x";
//             ad.csObjectName = "y";
//             ad.csObjectPath ="xyz";
//             ad.isFile = true;
//             CAvoidDuplicationDlg avoidDuplicationDlg(ad);
//             avoidDuplicationDlg.DoModal();
//         }
// 
//         targetNode = job["MultiUploadOptDlg"];
//         if (targetNode.IsDefined() && targetNode.as<bool>())
//         {
//             MultiUploadOptDlg multiUploadOptDlg(FILE_TYPE , SINGLEDOC_TYPE);
//             multiUploadOptDlg.DoModal();
//         }

        targetNode = job["CDlgFileTransfer_Up"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
//             CDlgFileTransfer dlgFileTransfer1(CDlgFileTransfer::DOTRANSE_DLG_UP_MOVE, NULL);
//             dlgFileTransfer1.DoModal();
        }

        targetNode = job["CDlgFileTransfer_Down"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
//             CDlgFileTransfer dlgFileTransfer2(CDlgFileTransfer::DOTRANSE_DLG_DOWN_MOVE, NULL);
//             dlgFileTransfer2.DoModal();
        }

        targetNode = job["CDlgFileTransfer_Execute"];
        if (targetNode.IsDefined() && targetNode.as<bool>())
        {
//             CDlgFileTransfer dlgFileTransfer3(CDlgFileTransfer::DOTRANSE_DLG_EXCUTE, NULL);
//             dlgFileTransfer3.DisableCancel(FALSE);
//             dlgFileTransfer3.DoModal();
        }
    }
    catch (YAML::Exception& ex)
    {
        OutputDebugStringA(ex.what());
    }
}

IMPLEMENT_TESTAGENTFUNC(session_check)
{
	try
    {
        const YAML::Node& cfg = testCfg["config"];
	    YAML::Node& job = const_cast<YAML::Node& >(cfg["job"]);

        YAML::Node& targetNodeSession = job["session"];
        YAML::Node& targetNodeNetwork = job["network"];
        YAML::Node& targetNodeSesCont = job["sessionContinuos"];

        if (targetNodeSession.IsDefined() && targetNodeSession.as<bool>())
        {
#pragma region session check
            std::wstring skey(g_activeAccount->GetSessionKey());
            LPCWSTR key = skey.c_str();
            SessionChecker sc;
            auto check = [&sc, &key]()
            {
                sc.Check(key);
            };

            auto checkOther = [&sc]()
            {
                sc.Check(L"149008102173966629");
            };

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }

            ::Sleep(1000);

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }

            {
	            boost::thread t1(checkOther);
                boost::thread t2(checkOther);
                boost::thread t3(checkOther);
                boost::thread t4(checkOther);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(checkOther);
                boost::thread t2(checkOther);
                t1.join();
                t2.join();
            }
#pragma endregion session check
        }

        if (targetNodeNetwork.IsDefined() && targetNodeNetwork.as<bool>())
        {
#pragma region network check
            auto check = []()
            {
                AppStatus::GetNetworkStatus();
            };

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }

            ::Sleep(10 * 1000);

            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }
            ::Sleep(1000);
            {
	            boost::thread t1(check);
                boost::thread t2(check);
                boost::thread t3(check);
                boost::thread t4(check);
                t1.join();
                t2.join();
                t3.join();
                t4.join();
            }
            {
                boost::thread t1(check);
                boost::thread t2(check);
                t1.join();
                t2.join();
            }
#pragma endregion network check
        }

        if (targetNodeSesCont.IsDefined() && targetNodeSesCont.as<bool>())
        {
#pragma region session check continuosly
            std::wstring skey(g_activeAccount->GetSessionKey());
            LPCWSTR key = skey.c_str();
            SessionChecker sc;

            Concurrency::parallel_for(0, 0x7FFFFFFF, 1, [&sc, &key](int /*v*/)
            {
                sc.Check(key);
            });         
#pragma endregion session check continuosly
        }
    }
    catch (YAML::Exception& ex)
    {
        OutputDebugStringA(ex.what());
    }    
}

IMPLEMENT_TESTAGENTFUNC(refresh_not_saved_doclist_dlg)
{
	string testId = testCfg["test_id"].as<string>();
	fs::path testDir = "c:/Cyberdigm/DestinyECMAgent/test/" + testId;
	if (!DOUtilFile::IsExistedFilePath(testDir.wstring().c_str()))
		fs::create_directories(testDir);

	const YAML::Node cfg = testCfg["config"];
	
	auto pMainDlg = theApp.GetMainDlg();
	if (pMainDlg->m_pNotSavedDocListDlg)
		::PostMessage(pMainDlg->m_pNotSavedDocListDlg->m_hWnd, WM_DOCLIST_REFRESH, NULL, NULL);
	else
	{
		::AfxMessageBox(L"pMainDlg->m_pNotSavedDocListDlg is NULL !!!", MB_OK);
	}
}

IMPLEMENT_TESTAGENTFUNC(shutdown_applications_for_update)
{
#pragma region Shutdown Application Test
	// VDiskManager가 사라지기를 기다린다. 5분
	static const DWORD kSleepVDiskManager = 1000;
	static const DWORD kMaxCountsUntilDisappearVDiskManager = 60 * 5;
	DWORD countsUntilDisappearVDiskManager = 0;

	//////////////////////////////////////////////////////////////////////////
	// For Test environment
	auto GetDriveECMIFPipeName = []() -> CString {
		CString csECMIFPAth = L"Software\\AppDataLow\\Software\\DestinyECM\\DestinyECMAgent\\NotifyIF\\PantaAgent.exe";
		int nProcessID = DOUtilReg::RegGetIntValue(csECMIFPAth, L"PID", 0);
		if(1 < nProcessID)
		{
			CString csTmpName;
			csTmpName.Format(L"%s%d", PIPENAME_DestinyBridgePipeForECMA_Prefix, nProcessID);
			return csTmpName;
		}

		return L"";
	};
	typedef boost::char_separator<wchar_t> WCharSeparator;
	typedef boost::tokenizer<boost::char_separator<wchar_t>,
		boost::wstring_ref::const_iterator, std::wstring> TokenizerWStringRef;
	//////////////////////////////////////////////////////////////////////////

#define PIPENAME_ECMIF_X L"\\\\.\\Pipe\\X_DestinyECMIF"
	while(TRUE)
	{
		//CString csDriveECMIFPipeName = PIPENAME_ECMIF_X; //GetDriveECMIFPipeName();
		CString csDriveECMIFPipeName = GetDriveECMIFPipeName();
		if(csDriveECMIFPipeName.IsEmpty())
			return;

		DOPipeClientString doPipeClient(csDriveECMIFPipeName);
		CString csRunningStatus = doPipeClient.DoPipeCommand(L"CHECK_RUNNING_APP");
		if(csRunningStatus.IsEmpty())
			return;

		TRACE(L"%s's result : %s", (LPCTSTR)csDriveECMIFPipeName, (LPCTSTR)csRunningStatus);
		CString strLog;
		strLog.Format(L"Pipe: %s\nResult: %s", csDriveECMIFPipeName, csRunningStatus);
		MessageBox(0, strLog, L"debug", MB_OK);

		int nPos = csRunningStatus.Find(L"APP_LIST|");
		// csRunningStatus의 형식
		//    APP_LIST|RUNNING_APP_1?RUNNING_APP_2?RUNNING_APP_3
		CString csAppList;
		if(nPos != -1)
		{
			nPos += 9;
			csAppList = csRunningStatus.Tokenize(L"|", nPos);
		}

		if( (!csAppList.IsEmpty()) && (0 != csAppList.CompareNoCase(L"EMPTY")))
		{
			TRACE(L"열린 목록 %s", csAppList);

#pragma region Wait To Disappear VDiskManager
			// #52332
			// 이 시점에서 VDiskManager가 포함되어 있는 경우
			// 대부분 잠시 기다리면 VDiskManager는 없어진다.
			auto findVDiskManager = [&csAppList]() -> bool {
				static const WCharSeparator kSeparatorQuestion(L"?");
				boost::wstring_ref appListRef(csAppList.GetString());
				TokenizerWStringRef tokenizer(appListRef, kSeparatorQuestion);

				BOOST_FOREACH (auto& s, tokenizer) {
					if (s == L"VDiskManager")
						return true;
				}
				return false;
			};
			if (++countsUntilDisappearVDiskManager <= kMaxCountsUntilDisappearVDiskManager &&
				findVDiskManager())
			{
				::Sleep(kSleepVDiskManager);
				continue;
			}
			countsUntilDisappearVDiskManager = 0;
#pragma endregion

			csAppList.Replace( L"?" ,L",\r\n  ");

			CString csMessage;
			//csMessage.Format(L"업데이트를 위해 대상 응용프로그램 종료가 필요합니다.\r\n\r\n%s\r\n\r\n작업 중인 프로그램들을 모두 닫고 다시 시도 하시기 바랍니다.", csAppList);
			csMessage.Format(DOResMsg::GetMsg(COM_ECMAGENT, L"EAForUpdateAppTerminate"), csAppList);
			DOMessageBox::DoShowMessageBoxBlocking(csMessage, NULL, CTRL_MESSAGE_MODE_INFO, MB_RETRY);
			
			continue;
		}

		// PantaAgent를 셧다운 할 이유가 없는데, 무조건 셧다운 하고 있다.
		//doPipeClient.DoPipeCommand(L"SHUTDOWN");
		return;
	}
#pragma endregion Shutdown Application Test
}

IMPLEMENT_TESTAGENTFUNC(performance_test)
{
	//////////////////////////////////////////////////////////////////////////
	// NewDoc 메타파일 처리 코드 성능 테스트
	/*
	DECL_DURATION_TESTER(metafile);

	for (int i=0; i<4000; i++)
	{
		BEGIN_DURATION_TEST(metafile);

		LOCALMETAFILE meta;
		meta.sourceFilePath_ = "D:\\working-documents\\1T\\Test\\sourceFile.ext";
		meta.folderOID_ = "12345678";
		meta.documentOID_ = "abcdefgh";
		meta.fileOID_ = "ABCDEFGH";
		meta.storageFileOID_ = "!@#$%^&*";

		DOLocalMetaFile metaFile;
	
		wstring metaFilePath = DOLocalMetaFile::FindMetaFileInWM(meta.sourceFilePath_);
		if (metaFilePath.empty())
		{
			metaFilePath = DOLocalMetaFile::CreateBeforeNewDoc(
				meta,
				meta.sourceFilePath_,
				L"aabbccdd"
				);
			metaFile.SetMetaFilePath(metaFilePath);
			metaFile.Update(meta, LMFS__BEGIN_NEWDOC);
		}
		else
		{
			metaFile.Update(meta, LMFS__END_NEWDOC);
			metaFile.Read(meta);
		}

		END_DURATION_TEST(metafile);
	}
	*/

	//////////////////////////////////////////////////////////////////////////
	// 파이프 구조체 크기
	/*
	S_DOPipeParam_NewDocByOneTransaction_Req reqParam = {};
	S_DOPipeParam_NewDocByOneTransaction_Res resParam = {};
	ZeroMemory(&reqParam, sizeof(reqParam));
	ZeroMemory(&resParam, sizeof(resParam));

	CString str;
	str.Format(L"Size of NewDoc Structure, req: %d, res: %d", sizeof(reqParam), sizeof(resParam));
	MessageBox(0, str, L"debug", MB_OK);
	*/

	//////////////////////////////////////////////////////////////////////////
	// 파일 생성, 수정일자 획득 시간 측정
	/*
	DECL_DURATION_TESTER(filetime);

	for (int i=0; i<4000; i++) 
	{
		BEGIN_DURATION_TEST(filetime);
		CString strFileName = L"D:\\working-documents\\1T\\Test\\sourceFile.ext";
		FILETIME now = cmn::file::systemTimeUtc();
		auto filetime = [&strFileName, now] (const int i) -> const CString
		{
			FILETIME ft;
			try
			{
				if(i == 1)
					ft = cmn::file::creationTime(LPCWSTR(strFileName));
				else if(i == 2)
					ft = cmn::file::lastWriteTime(LPCWSTR(strFileName));
			}
			catch(const std::exception& e)
			{
				DOLogX doLogX(DOLOG_FUNCLINE, L"%s", CA2W(e.what()));
				ft = now;
			}
			return S2W(cmn::cnv::toStr(ft)).c_str();
		};

		filetime(1);
		filetime(2);
		END_DURATION_TEST(filetime);
	}

	CString str;
	str.Format(L"%d회 / %f초", s_dur_filetime_cnt, s_dur_filetime);
	MessageBox(0, str, L"debug", MB_OK);
	*/

	//////////////////////////////////////////////////////////////////////////
	// 로그인 호출 처리시간 측정
	/*
	DECL_DURATION_TESTER(login);

	for (int i=0; i<10; i++) 
	{
		BEGIN_DURATION_TEST(login);

		try
		{
			if (NULL == g_activeAccount)
				continue;

			g_loginMgr.DoLogIn(*g_activeAccount);
		}
		catch (DOException& ex)
		{
			(ex);
			continue;
		}

		END_DURATION_TEST(login);

		Sleep(3000);
	}

	CString str;
	str.Format(L"%d회 / %f초", s_dur_login_cnt, s_dur_login);
	MessageBox(0, str, L"debug", MB_OK);
	*/

	//////////////////////////////////////////////////////////////////////////
	// chrono 와 TickCount 비교; 천만번 호출
	/*
	DECL_DURATION_TESTER(timer1);
	DECL_DURATION_TESTER(timer2);

	for (int i=0; i<10000000; i++) 
	{
		BEGIN_DURATION_TEST(timer1);

		boost::chrono::system_clock::now();

		END_DURATION_TEST(timer1);
	}

	for (int i=0; i<10000000; i++) 
	{
		BEGIN_DURATION_TEST(timer2);

		GetTickCount();

		END_DURATION_TEST(timer2);
	}

	CString str;
	str.Format(L"%d회 / chrono %f초 / tickcount %f초", s_dur_timer1_cnt, s_dur_timer1, s_dur_timer2);
	MessageBox(0, str, L"debug", MB_OK);
	*/

	//////////////////////////////////////////////////////////////////////////
	// 로그파일 1000라인 기록 시간 측정
	/*
	DECL_DURATION_TESTER(writelog);

	for (int i=0; i<1000; i++) 
	{
		BEGIN_DURATION_TEST(writelog);

		DOLog::PrintLogDebug(DOLOG_FUNCLINE, L"%04d", i);
		boost::chrono::system_clock::now();

		END_DURATION_TEST(writelog);
	}

	CString str;
	str.Format(L"%d회 / %f초", s_dur_writelog_cnt, s_dur_writelog);
	MessageBox(0, str, L"debug", MB_OK);

	s_dur_writelog_cnt = 0; 
	s_dur_writelog = boost::chrono::system_clock::now() - boost::chrono::system_clock::now(); // 0
	*/
}

// 41817
IMPLEMENT_TESTAGENTFUNC(ecmif_getfilestatus)
{
	//const YAML::Node cfg = testCfg["config"];

	string fileoid = "";
	string filepath = "";

	if (testCfg["fileoid"])
		fileoid = testCfg["fileoid"].as<string>();
	if (testCfg["filepath"])
		filepath = testCfg["filepath"].as<string>();

	DOLogX log(DOLOG_FUNCLINE);
	auto r2 = g_ecmIfxCaller->DoXCmd_GetFileDownloadPath(
		log,
		CString(fileoid.c_str())
	);
		
	auto r = g_ecmIfxCaller->DoXCmd_GetFileStatus(
		log,
		CString(fileoid.c_str()),
		CString(filepath.c_str())
	);

	CString str;
	str.Format(
		L"== Input ==\n"
		L"파일OID: %s\n"
		L"파일경로: %s\n"
		L"(파일다운로드경로: %s (획득: %d))\n\n"
		L"== Output ==\n"
		L"bExist: %s\n"
		L"wFileStatus: 0x%08x\n"
		L"wServerWorkStatus: 0x%08x\n"
		L"wProcessType: 0x%08x\n"
		L"dwProcessID: 0x%08x\n"
		L"wNewRunProcessType: 0x%08x\n\n"
		L"== Result ==\n"
		L"로컬파일존재 / Exist: %s\n"
		L"파일상태 (창소유: %d) (핸들잠김: %d) (확인불가: %d)\n"
		L"서버반영중 (작업: %s)\n"
		L"프로세스타입 (대상App: %d) (창소유: %d) (ROP: %d) (임시대상App: %d)",
		r.wszOID,
		r.wszPath,
		r2.wszDownloadPath,
		r2.bSuccessDownloadPath ? 1 : 0,
		//////////////////////////////////////////////////////////////////////////
		r.bExist ? L"true" : L"false",
		r.wFileStatus,
		r.wServerWorkStatus,
		r.wProcessType,
		r.dwProcessID,
		r.wNewRunProcessType,
		//////////////////////////////////////////////////////////////////////////
		r.bExist ? L"true" : L"false",
		((r.wFileStatus & 0x01) == 0x01) ? 1 : 0,
		((r.wFileStatus & 0x02) == 0x02) ? 1 : 0,
		((r.wFileStatus & 0x03) == 0x03) ? 1 : 0,
		(r.wServerWorkStatus == 2) ? L"Attach" : (r.wServerWorkStatus == 3 ? L"Update" : (r.wServerWorkStatus == 1 ? L"NewDoc" : L"Unknown")),
		((r.wProcessType & 0x01) == 0x01) ? 1 : 0,
		((r.wProcessType & 0x02) == 0x02) ? 1 : 0,
		((r.wProcessType & 0x04) == 0x04) ? 1 : 0,
		((r.wProcessType & 0x08) == 0x08) ? 1 : 0
	);
	MessageBox(0, str, L"debug", MB_OK);
}

// 41817
IMPLEMENT_TESTAGENTFUNC(ecmif_getfiledownloadpath)
{
	string fileoid = testCfg["fileoid"].as<string>();
	DOLogX log(DOLOG_FUNCLINE);
	auto r = g_ecmIfxCaller->DoXCmd_GetFileDownloadPath(log, CString(fileoid.c_str()));

	CString str;
	str.Format(
		L"== Input ==\n"
		L"파일OID: %s\n"
		L"== Output ==\n"
		L"파일다운로드경로: %s (획득성공: %d)\n",
		r.wszOID,
		r.bSuccessDownloadPath ? r.wszDownloadPath : L"[failed]",
		r.bSuccessDownloadPath ? 1 : 0
	);
	MessageBox(0, str, L"debug", MB_OK);
}

// 41817
IMPLEMENT_TESTAGENTFUNC(dk_is_downloading)
{
	string fileoid = testCfg["fileoid"].as<string>();

	CString fileOID = CString(fileoid.c_str());
	auto downloadingListItem = DOTransfer::FindInDownloadingList(fileOID);
	CString str;
	str.Format(
		L"== Input ==\n"
		L"파일OID: %s\n"
		L"== Output ==\n"
		L"다운로드중? %d\n",
		fileOID, downloadingListItem ? 1 : 0
	);
	MessageBox(0, str, L"debug", MB_OK);
}

// 41817
IMPLEMENT_TESTAGENTFUNC(dk_is_lfro)
{
	bool isReadOnly = false;
	string filepath = "";
	if (testCfg["filepath"])
		filepath = testCfg["filepath"].as<string>();
	CString filePath = CString(filepath.c_str());

	DOUtilFile::IsExistedFilePathAndReadOnly(
		filePath,
		isReadOnly,
		true);

	CString str;
	str.Format(
		L"== Input ==\n"
		L"파일경로: %s\n"
		L"== Output ==\n"
		L"Readonly? %d\n",
		filePath, isReadOnly ? 1 : 0
	);
	MessageBox(0, str, L"debug", MB_OK);
}

IMPLEMENT_TESTAGENTFUNC(decisionkeys)
{
#if defined(_DEBUG)
	// 디버그 빌드에서만 사용하도록 함.
	string fileoid = "";
	string filepath = "";
	if (testCfg["fileoid"]) fileoid = testCfg["fileoid"].as<string>();
	if (testCfg["filepath"]) filepath = testCfg["filepath"].as<string>();
	CString fileOID = CString(fileoid.c_str());
	CString filePath = CString(filepath.c_str());

	DownloadDecision decision;
	decision.Initialize(fileOID, filePath, READONLY, DownloadDecisionFrom::EXPLORER, false, false);
	decision.CheckAllDeicisionKeys();
	// -> MessageBox를 띄움
#endif
}

IMPLEMENT_TESTAGENTFUNC(multiple_request_at_sametime)
{
#if defined(_DEBUG)
	// 가상 Request 로 execute 를 시도한다.
	static std::thread *mainThread = nullptr;
	if (mainThread != nullptr)
		mainThread->join();

	auto GetConfigValue = [&testCfg](string propertyName)
	{
		string value = "";
		if (testCfg[propertyName] && !testCfg[propertyName].IsNull())
			value = testCfg[propertyName].as<string>();

		CString strValue(value.c_str());
		return strValue;
	};

	auto GetConfigValueAsString = [&testCfg, &GetConfigValue](string propertyName)
	{
		return CS2S(GetConfigValue(propertyName));
	};

	bool canReturn = false;
	mainThread = new std::thread([&]()
	{
		DOLog::PrintLog(DOLOG_FUNCLINE, L"Starting test main thread");

		DOWebServerForAgent::http_request r;
		auto &p = r.params_;

		std::vector<string> propertyNames = {
			"sessionKey", "docID", "fileID_", "fileID", "fileName",
			"fileDate", "execute", "checkOut", "typeExecuteFileMode",
			"clientType", "version", "isDeleted", "externalMode",
			"convertFileType", "callback"
		};

		for (auto propertyName : propertyNames)
		{
			p[propertyName] = GetConfigValueAsString(propertyName);
		}

		auto fileOID = GetConfigValue("fileID_");

		// 여기까지 진행한 후에 이 함수를 리턴해야 읽기 위반이 아님.
		canReturn = true;

		// fileName은 특이하게 wstring 으로 넣어줘야 한다.
		// config 파일에서는 string으로 읽은후 wstring 으로 변환한 값을 assign 한다.
		wstring wstr(
			CS2W(
				S2CS(
					p["fileName"]
				)
			).c_str()
		);
		p["fileName"].assign((char*)wstr.c_str(), (wstr.size() + 1) * 2);

		// 이렇게 하면 실행된다.
		//theApp.GetWebServer()->execute(&r);

		// 동시에 여러개 호출해보자.
		HANDLE eventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
		auto WaitAndExecute = [&](int id)
		{
			DOLog::PrintLog(DOLOG_FUNCLINE, L"[Thread%d] Waiting event...", id);
			auto waitResult = WaitForSingleObject(eventHandle, INFINITE);

			//if (id == 2)
			//	Sleep(100);
			DOLog::PrintLog(DOLOG_FUNCLINE, L"[Thread%d] Calling execute...", id);
			theApp.GetWebServer()->execute(&r);

			DOLog::PrintLog(DOLOG_FUNCLINE, L"[Thread%d] execute finished...", id);
		};

#define threadCount 30
		std::thread *t[threadCount];
		for (int i = 0; i < threadCount; i++)
			t[i] = new std::thread(WaitAndExecute, i);

		for (int i = 0; i < 3; i++)
		{
			DOLog::PrintLog(DOLOG_FUNCLINE, L"Set event after %d seconds ...", 3 - i);
			Sleep(1000);
		}

		SetEvent(eventHandle);
		CloseHandle(eventHandle);

		for (int i = 0; i < threadCount; i++)
		{
			t[i]->join();
			delete t[i];
		}
#undef threadCount

		DOLog::PrintLog(DOLOG_FUNCLINE, L"execute threads finished");

		CString metaPath = DOLocalMetaFile::FindMetaFileInWMByFileOid(fileOID).c_str();

		CString message;
		message.Format(L"MetaPath: %ls, exist? %d\n", metaPath, DOUtilFile::IsExistedFilePath(metaPath));
		MessageBox(0, message, 0, MB_OK);

		DOLog::PrintLog(DOLOG_FUNCLINE, L"Test main thread finished");
	});

	while (canReturn == false)
		Sleep(1);
#endif
}

IMPLEMENT_TESTAGENTFUNC(tasklist)
{
	const YAML::Node& cfg = testCfg["config"];

	const string processName = cfg["process_name"].as<string>();
	string cmdLine = "tasklist /v /fo csv /nh /fi \"IMAGENAME eq " + processName + "\"";
	CString outString = DOUtilProcess::RunCommand(CA2T(cmdLine.c_str()), L"C:\\WINDOWS\\system32");
	AfxMessageBox(outString);
}

IMPLEMENT_TESTAGENTFUNC(wait_existing_pki_sso_login)
{
//	g_mainDlg->TrySSOLogin();
}

IMPLEMENT_TESTAGENTFUNC(ecmif_log_pidlist_using_filehandle)
{
#if defined(_DEBUG)
	DOLogX log(DOLOG_FUNCLINE);
	string fullpath = "H:\\U\\사이버다임\\워드.docx";

	// EUC-KR 로 저장해야 한글을 잘 읽어오는 듯함.
	if (testCfg["fullpath"])
		fullpath = testCfg["fullpath"].as<string>();
	g_ecmIfxCaller->DoXCmd_LogPIDListUsingFileHandle(log, CString(fullpath.c_str()));
#endif
}

IMPLEMENT_TESTAGENTFUNC(rpc_timeout)
{
	DOLogX log(DOLOG_FUNCLINE);
	CString returnString;
	auto result = g_msgpackClient->GetConfiguration(
		log,
		L"agent.AdminInfo",
		L"",
		returnString);

	{
		auto begin = log.GetStartTick();
		auto end = GetTickCount();

		CString msg;
		msg.Format(L"Elapsed: %d ms", end - begin);
		MessageBox(0, msg, L"Result", MB_OK);
	}
}

IMPLEMENT_TESTAGENTFUNC(webdialog)
{
	const YAML::Node& cfg = testCfg["config"];

	// https://snow.cyberdigm.co.kr/redmine/projects/project_ecm/wiki/WebDialogList
	
	/*
	CDHtmlDialog
	DOSimpleHtmlDlg
		CDoExternalExportHtmlDlg
		CDoMsgViewHtmlDlg - 쪽지 x
		CdoPCTakeOutHtmlDlg
		CDoPersonalInfoHtmlDlg - 개인정보 *
	CSaveAsHTMLDlg
		WebJobUnknown
		WebJobEADocumentCreate
		WebJobEASelectFile *
		WebJobEASelectDocument *
		WebJobEASelectFolder *
		WebJobEAModifyFile
		WebJobEAReflectFile
		WebJobEASaveAsNewVersion
		WebJobEAViewProperties1
		WebJobEASaveDocumentToServer
		WebJobEADocumentCreatingStatus
		WebJobEASelectUser - 사용자선택 *
		WebJobEASelectGroup - 부서선택 *
		WebJobEAFavoriteAdd - 즐겨찾기 추가 *
		WebJobEAOutsideCheckout
		WebJobEAPersonalInformation - 개인정보 *
		WebJobEAHelp
		WebJobEAOutsideCheckoutRequest
		WebJobEAApproveOutsideCheckout
		WebJobEANotice
		WebJobEARequestView
		WebJobEAAppPort
	DOHtmlDlg *
		CustomLoginDlg
		SaveDocumentToServerDlg
		SelectFolderDlg
		ShowCartListDlg *
		ViewPropertiesDlg - 속성창
	CWebDlg ?
	CDOHtmlWithModelessDlg ?
		CDoSaveDocToServerWithModelessDlg
	*/

	auto mode = GetValueFromConfig(testCfg, "mode");
	if (mode.CompareNoCase(L"saveasdlg") == 0)
	{
		auto type = GetValueFromConfig(testCfg, "type");
		auto title = DOResMsg::GetMsg(COM_ECMAGENT, GetValueFromConfig(testCfg, "title"));

		CONEConnector con(L"", L"", L"", L"", nullptr, g_mainDlg->m_userConfMgr.get());
		CString url;
		url.Format(L"%s%s", con.GetPageUrl(type), AppStatus::GetActiveSessionKey());

		auto hs = new HookSaveAsInfo;
		hs->hWndParent = nullptr;
		hs->csUID = AppStatus::GetActiveUserID();
		hs->csURL = url;
		hs->m_endEvent.ResetEvent();

		// #CHECK 버그인지 모르겠으나, SaveAsHTMLDlg 내부의 switch 는 title에 따라 크기와 위치를 조절하는데
		// title을 못찾으면 default 로 동작, 이때 내부 dialog 의 좌표가 0,0이 아닌 10,10 으로 되어 밀린것처럼 표시됨.
		// 밀린 상태의 화면을 사용하는 UI 가 있는지 확인이 되지 않아 이 코드를 테스트하려면 SaveAsHTMLDlg 도 수정하고 테스트해야
		// 정상적으로 표시됨.
		CSaveAsDlg regDlg(nullptr, hs, hs->csURL, title, L"", L"", L"", L"", hs->hWndParent);
		regDlg.DoModal();

		delete hs;
	}
	else if (mode.CompareNoCase(L"dohtmldlg") == 0)
	{
		// dlg 를 띄울수는 있는데 닫으면 crash 된다. 시간관계상 원인파악 못함

		//CDoPCTakeOutHtmlDlg dlg;

		//CDoExternalExportHtmlDlg dlg;


		//dlg.DoModal();
	}
	else if (mode.CompareNoCase(L"simplehtmldlg") == 0)
	{
		//SelectFolderDlg dlg;

		//SaveDocumentToServerDlg dlg;

		//dlg.DoModal();
	}
}

IMPLEMENT_TESTAGENTFUNC(ecmif_req_running_app_for_loginstatus)
{
	DOLogX log(DOLOG_FUNCLINE);
	RUNNING_APP_LIST_FOR_LOGINSTATUS status;
	status.nLoginStatus = LOGIN_STATUS::LOGOUT;
	ZeroMemory(status.wszAppList, sizeof(WCHAR) * MAX_PATH);

	g_ecmIfxCaller->DoXCmd_GetAppListForLoginStatus(log, status);

	log.PrintDebug(DOLOG_FUNCLINE, L"For status: %s, ProcessList: %s",
		status.nLoginStatus == LOGIN_STATUS::LOGIN ? L"LOGIN" : L"LOGOUT",
		status.wszAppList);
}

IMPLEMENT_TESTAGENTFUNC(ecmif_open_ecm_folder)
{
	DOLogX log(DOLOG_FUNCLINE);

	auto csOID = GetValueFromConfig(testCfg, "folderoid");
	if (csOID.IsEmpty())
		csOID = L"1MglmD5L09o";

	wchar_t oid[22];
	wcscpy_s(oid, OBJECT_ID_LENGTH, csOID);

	// 대상 폴더 여는 것만 테스트함
	OBJECT_TYPE type = FOLDER_TYPE;

	{
		// 경로 확인이 가능한 폴더인지 확인한다.
		// TODO: 팬타랩이 이미 확인하고 있기때문에 NotifyCallback이 결과를 줘야 한다.
		CString errorCode, errMsg;
		auto pFullPathItem = g_msgpackClient->GetFullPathItem(
			log,
			oid,
			1, // requestMode
			(OBJECT_TYPE)type,
			errorCode, errMsg);

		if (pFullPathItem == nullptr || !errorCode.IsEmpty())
		{
			log.PrintError(DOLOG_FUNCLINE, L"Result=ERROR");
			return;
		}
	}
	CString errorString;
	auto result = g_ecmIfxCaller->DoXCmd_OpenECMFolder(log, oid, type, OpenEcmFolderTarget, true, errorString);
	log.Print(DOLOG_FUNCLINE, L"Result=%d", result);
}

IMPLEMENT_TESTAGENTFUNC(copy_folder_to_folder)
{
	DOLogX log(DOLOG_FUNCLINE);

	auto csFrom = GetValueFromConfig(testCfg, "from");
	if (csFrom.IsEmpty())
		csFrom = L"1MglmD5L09o";

	auto csTo = GetValueFromConfig(testCfg, "to");
	if (csTo.IsEmpty())
		csTo = L"1MglmD5L0TC";

	wchar_t from[22];
	wcscpy_s(from, OBJECT_ID_LENGTH, csFrom);

	wchar_t to[22];
	wcscpy_s(to, OBJECT_ID_LENGTH, csTo);

	CString option = L"";
	g_msgpackClient->EcmRPC_copyFolder(from, to, option);
}

IMPLEMENT_TESTAGENTFUNC(check_update_time)
{
	//DOLogX log(DOLOG_FUNCLINE);

	//auto oid = GetValueFromConfig(testCfg, "folderoid");
	//wchar_t oid2[22];
	//wcscpy_s(oid2, OBJECT_ID_LENGTH, oid);

	//wchar_t updateTime[128] = { 0, };
	//auto rvo = g_msgpackClient->GetLastUpdateTime(log, oid2, FOLDER_TYPE, updateTime);

	//log.Print(DOLOG_FUNCLINE, L"===================================================");
	//log.Print(DOLOG_FUNCLINE, L"GetLastUpdateTime(%s): %s", oid2, rvo->returnValue.c_str());
	//log.Print(DOLOG_FUNCLINE, L"===================================================");
}

IMPLEMENT_TESTAGENTFUNC(ecmif_remove_ecmfile_from_db)
{
	DOLogX log(DOLOG_FUNCLINE);
	
	auto oid = GetValueFromConfig(testCfg, "fileoid");
	auto path = GetValueFromConfig(testCfg, "filepath");
	wchar_t oid2[22], path2[MAX_PATH * 2];
	wcscpy_s(oid2, OBJECT_ID_LENGTH, oid);
	wcscpy_s(path2, MAX_PATH * 2, path);

	auto ret = g_ecmIfxCaller->DoXCmd_RemoveECMFileFromMemoryAndDB(log, oid2, path2);

	log.Print(DOLOG_FUNCLINE, L"===================================================");
	log.Print(DOLOG_FUNCLINE, L"DoXCmd_RemoveECMFileFromMemoryAndDB(%s, %s): %d", 
		oid2, path2, ret);
	log.Print(DOLOG_FUNCLINE, L"===================================================");
}

IMPLEMENT_TESTAGENTFUNC(gdi_handle_leak)
{
	DOLogX log(DOLOG_FUNCLINE);

	auto countStr = GetValueFromConfig(testCfg, "count");
	auto count = _wtoi(countStr);

	for (int i = 0; i < count; i++)
	{
		CreatePen(PS_SOLID, 1, 0);
	}
}

IMPLEMENT_TESTAGENTFUNC(ecmif_getecmifinfo)
{
	DOLogX log(DOLOG_FUNCLINE);

	if (g_activeAccount)
	{
		if (g_ecmIfxCaller->IsServerAlive())
		{
			S_DOPipeParam_GetEcmIfInfo_Res_Real sGetEcmIfInfoRes;
			if (g_ecmIfxCaller->DoXCmd_GetEcmIfInfo(log, sGetEcmIfInfoRes, ECM_IF_INFO_TYPE::ALL_CHECK))
			{
				;
			}
		}
	}
}

IMPLEMENT_TESTAGENTFUNC(ecmif_createfolder_forreturndriveobject)
{
	auto parentoid = GetValueFromConfig(testCfg, "parent_folderoid");
	auto foldername = GetValueFromConfig(testCfg, "foldername");
	
	DOLogX doLogX(DOLOG_FUNCLINE);

	CString errCode, errMsg;
	auto drvObj = g_msgpackClient->CreateFolderForReturnDriveObject(doLogX, parentoid, foldername, errCode, errMsg);
	if (nullptr == drvObj)
	{
		if (errCode == ServerDMException::FolderNameIsDuplicated) // DM2005
		{
			// 이름 중복으로 폴더를 생성하지 못한 경우

			// [parentFolder 의 fullPath + 만들려던 folderName] 에 해당하는 DriveObject를 얻어서 리턴한다.
			CString fullPath;
			CString errCode2, errMsg2;
			auto fullpathItem = g_msgpackClient->GetFullPathItem(doLogX, parentoid, 0, FOLDER_TYPE, errCode2, errMsg2);
			if (fullpathItem != nullptr && errMsg2.IsEmpty() && fullpathItem->driveObjectsCNT > 0)
			{
				//fullPath.Format(L"%c:\\", theApp.GetMainDlg()->m_sPolicyInfo.cEcmDDVolume);
				fullPath = L"\\";
				
				for (int i=0; i<fullpathItem->driveObjectsCNT; ++i)
				{
					auto obj = fullpathItem->driveObjects[i];
					if (obj->oid == L"S_ROOT") continue;

					fullPath.AppendFormat(L"%s\\", obj->objName.c_str());
				}
				
				fullPath += foldername;

				auto createdFolder = g_msgpackClient->GetObjectByFullPathName(doLogX, fullPath);
				if (createdFolder != nullptr && !createdFolder->objName.empty())
				{
					CString msg;
					msg.Format(L"Early-created folder object obtained: 0x%08x", createdFolder.get());
					MessageBox(0, msg, L"Special case", MB_OK);
					return;
				}
			}
		}

		MessageBox(0, L"Failed to get folder object!!!", L"Error", MB_OK);
		return;
	}

	MessageBox(0, L"Successfully obtained created folder", L"Normal case", MB_OK);
}

IMPLEMENT_TESTAGENTFUNC(filesystem_corrupt)
{
	DOLogX doLogX(DOLOG_FUNCLINE);

#if 0 // 직접호출
	FileSystemCorrupt::Occurred();
#else // 전역 이름있는 이벤트 핸들을 통한 호출
	auto filePath = GetValueFromConfig(testCfg, "filePath");
	if (filePath.IsEmpty())
		filePath = L"S:\\사이버다임\\OhMyGod.txt";
	
	SetLastError(ERROR_FILE_CORRUPT);
	cmn::shell::FileSystemCorruptCheck(nullptr, filePath, __FUNCTION__, __LINE__);
#endif

}

IMPLEMENT_TESTAGENTFUNC(ecmif_vdisk_req_open_file)
{
	DOLogX log(DOLOG_FUNCLINE);

	if (g_activeAccount && g_ecmIfxCaller->IsServerAlive())
	{
		OPEN_MODE openMode;
		auto fileOID = GetValueFromConfig(testCfg, "fileOID");
		auto fileName = GetValueFromConfig(testCfg, "fileName");
		auto openModeStr = GetValueFromConfig(testCfg, "openMode");
		if (!openModeStr.IsEmpty() && openModeStr[0] == L'W')
			openMode = WRITABLE;
		else
			openMode = READONLY;
		
		if (g_ecmIfxCaller->DoXCmd_VDisk_OpenFile(log, fileOID, fileName, openMode))
		{
			CString msg;
			msg.Format(L"파일 열기 요청 성공");
			MessageBox(0, msg, L"TestAgent", MB_OK);
		}
	}
}

IMPLEMENT_TESTAGENTFUNC(ecmif_vdisk_get_working_filelist)
{
	DOLogX log(DOLOG_FUNCLINE);

	if (g_activeAccount && g_ecmIfxCaller->IsServerAlive())
	{
		auto workingFileList = g_ecmIfxCaller->DoXCmd_VDisk_GetWorkingFileList(log);
		if (workingFileList)
		{
			CString msg;
			msg.Format(L"workingFileList = 0x%08x", workingFileList);
			MessageBox(0, msg, L"TestAgent", MB_OK);

			workingFileList->Destroy();
		}
	}
}

// DOXPipeEAHandlers::DoXCmd_NotifyCollectFilesCompleted 코드를 그대로 복사해놓았음.
// #TODO 가짜 XPipePacket를 생성해서 파이프 서버로 던지자...
IMPLEMENT_TESTAGENTFUNC(utilityif_file_collect_completed)
{
	DOLogX log(DOLOG_FUNCLINE);

	CollectFilesCompletedReq reqParam;
	reqParam.collectType = (FileCollectType)_wtoi(GetValueFromConfig(testCfg, "collectType"));
	reqParam.succeededFiles = _wtoi(GetValueFromConfig(testCfg, "succeededFiles"));
	reqParam.failedFiles = _wtoi(GetValueFromConfig(testCfg, "failedFiles"));

	{
		auto GetTargetPathName = [](FileCollectType type)
		{
			switch (type)
			{
			case FileCollectType::PCCentral: return RESMSG(PTEEntryNameToBeUpdateRoot);
			case FileCollectType::PCExternal: return RESMSG(PTEEntryNameToBeUpPCExport);
			case FileCollectType::Periodic: return RESMSG(PTEEntryNameToBeTempFileBox);
			}
			return L"Unknown";
		};
		switch (reqParam.collectType)
		{
		case FileCollectType::PCCentral:
		case FileCollectType::PCExternal:
			{
				// PTEEntryNameToBeUpdateRoot=초기 문서 중앙화
				// PTEEntryNameToBeUpPCExport=서버등록 대기문서
				// PTEEntryNameToBeTempFileBox=임시파일함
				if (reqParam.succeededFiles != 0 && reqParam.failedFiles == 0)
				{
					// 모든파일 성공: 메시지 출력 후 종료
					//------------------------------------
					// PC의 파일 수집을 완료했습니다. 수집된 파일은 {이동한 파일 수}건입니다.  
					// {이동한 폴더명}에서 이동한 파일을 확인해 주십시오.(1500-LFCOL-0001)
					CString msg;
					msg.Format(
						DOResMsg::GetMsg(COM_ECMAGENT, L"EACompletedToCollectLocalFilesWithoutFailure", FALSE, L"EACompletedToCollectLocalFilesWithoutFailure-%d"),
						reqParam.succeededFiles, GetTargetPathName(reqParam.collectType));
					DOMessageBox::DoShowMessageBoxBlocking(msg, nullptr, CTRL_MESSAGE_MODE_INFO, 0, FALSE, nullptr, FALSE, FALSE, TRUE);
				}
				else if (reqParam.succeededFiles != 0 || reqParam.failedFiles != 0)
				{
					// 모두 실패했거나, 성공/실패 섞여있는 경우: 선택가능 메세지박스 출력
					//--------------------------------------------------------------------
					// PC의 파일 수집을 완료했습니다.
					// 수집된 파일은 {이동한 파일 수} 건입니다. {실패 파일 수} 건 은 사용 중이거나 다른 이유로 이동에 실패하였습니다.
					// {이동한 폴더명}에서 이동한 파일을 확인하거나 PC를 재부팅하여 수집을 완료하여 주십시오.(1500-LFCOL-0002)
					// [PC 재부팅] / [나중에 하기]
					CString msg;
					msg.Format(
						DOResMsg::GetMsg(COM_ECMAGENT, L"EACompletedToCollectLocalFilesWithFailure", FALSE, L"EACompletedToCollectLocalFilesWithFailure-%d,%d,%s"),
						reqParam.succeededFiles, reqParam.failedFiles, GetTargetPathName(reqParam.collectType));

					MessageBoxParams param;
					param.buttonAdd = BUTTON_ADD_OK | BUTTON_ADD_CANCEL;
					param.contents = msg;
					param.title = RESMSG(EASystemNotice);
					param.uType = MB_YESNO | MB_ICONERROR;
					param.okBtnTxt = RESMSG(EARebootPC);
					param.cancelBtnText = RESMSG(EADoLater);
					auto ret = DOAMessageBoxWithCheck(param);
					log.Print(DOLOG_FUNCLINE, L"User selected '%s', %s",
						ret == IDOK ? L"IDOK" : L"IDCANCEL",
						ret == IDOK ? L"Reboot now" : L"Skip rebooting");
					if (ret == IDOK)
					{
						// 재부팅하려면 관리자권한이 필요함.
						UINT uFlags = EWX_REBOOT;
						BOOL bWinNT = FALSE;

						HANDLE hToken;
						TOKEN_PRIVILEGES tkp;
						OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
						LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
						tkp.PrivilegeCount = 1;
						tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
						AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

						uFlags |= EWX_FORCE;
						if (ExitWindowsEx(uFlags, 0))
						{
							log.Print(DOLOG_FUNCLINE, L"Succeed to ExitWindowsEx");
						}
						else
						{
							auto err = GetLastError();
							log.PrintError(DOLOG_FUNCLINE, L"Failed to ExitWindowsEx %s", LOGARG(GetSystemErrorMsg(&err)));
						}
					}
				}
			}
			break;
		case FileCollectType::Periodic:
			//#106606 수집된 파일이 없을 경우 티어오프 메시지 미발생
			if ((reqParam.succeededFiles != 0) || (reqParam.failedFiles != 0))
			{
				CString msg;
				msg.Format(DOResMsg::GetMsg(COM_ECMAGENT, L"EACompletedToCollectLocalFiles", 0, L"EACompletedToCollectLocalFiles-%d,%d,%s"),
					reqParam.succeededFiles,
					reqParam.failedFiles,
					DOResMsg::GetMsg(COM_ECMAGENT, L"PTEEntryNameToBeTempFileBox"));
				TearOff::Request(TRAY_TEAROFF_NOTICE, msg.GetString(), 1);
			}
			break;
		}
	}
}

IMPLEMENT_TESTAGENTFUNC(maindlg_cmd)
{
	DOLogX log(DOLOG_FUNCLINE);

	auto cmdStr = GetValueFromConfig(testCfg, "cmd");
	int cmd = _wtoi(cmdStr);

	::PostMessage(g_mainDlg->GetSafeHwnd(), WM_COMMAND, cmd, 0);
}

IMPLEMENT_TESTAGENTFUNC(pcfilebackup_test)
{
	DOLogX log(DOLOG_FUNCLINE);

	auto worker = PCFileBackup::GetInstance();
	std::thread t([&]()
		{
			worker->Test();
		});
	t.detach();
}

IMPLEMENT_TESTAGENTFUNC(pcfilebackup_realtime_policy)
{
	DOLogX log(DOLOG_FUNCLINE);

	PCFileCollect::pcFileBackupPolicy_.reset();
	auto policy = PCFileCollect::pcFileBackupPolicy_ = std::make_shared<DestinyMsgpack::XLocalCollectPolicy>();

#define SET_POLICY_MEMBER(_member)		policy->_member = \
	GetValueFromConfig(testCfg, (char*)(std::string("policy.") + #_member).c_str())
#define SET_POLICY_MEMBERBOOL(_member)	policy->_member = \
	GetValueFromConfig(testCfg, (char*)(std::string("policy.") + #_member).c_str()) == L"true" ? true : false
#define SET_POLICY_MEMBERINT(_member)	policy->_member = \
	_wtoi(GetValueFromConfig(testCfg, (char*)(std::string("policy.") + #_member).c_str()))
#define SET_POLICY_MEMBERINT64(_member)	policy->_member = \
	_wtoi64(GetValueFromConfig(testCfg, (char*)(std::string("policy.") + #_member).c_str()))

	SET_POLICY_MEMBER(oid);
	SET_POLICY_MEMBER(name);
	SET_POLICY_MEMBER(collectType);
	SET_POLICY_MEMBERBOOL(flagBackground);
	SET_POLICY_MEMBERBOOL(flagDefault);
	SET_POLICY_MEMBERINT(localTarget);
	SET_POLICY_MEMBER(creationType);
	SET_POLICY_MEMBER(duplicatedNameRule);
	SET_POLICY_MEMBER(targetFileExtType);
	SET_POLICY_MEMBER(targetFileExt);
	SET_POLICY_MEMBERINT(excludeFileSizeMB);
	SET_POLICY_MEMBER(serverFolderSubPath);
	SET_POLICY_MEMBERINT(fileTransferLimitKB);
	SET_POLICY_MEMBER(execType);
	SET_POLICY_MEMBER(execPeriodType);
	SET_POLICY_MEMBERINT(execPeriod);
	SET_POLICY_MEMBER(execStartTime);
	SET_POLICY_MEMBER(localPath);
	SET_POLICY_MEMBER(excludePath);
	SET_POLICY_MEMBERINT(startedAtLow);
	SET_POLICY_MEMBERINT(startedAtHigh);
	SET_POLICY_MEMBER(status);

#undef SET_POLICY_MEMBER
#undef SET_POLICY_MEMBERBOOL
#undef SET_POLICY_MEMBERINT
#undef SET_POLICY_MEMBERINT64

	PCFileBackup::OnReceivePolicy();
}

IMPLEMENT_TESTAGENTFUNC(webview2_test)
{
	DOLogX log(DOLOG_FUNCLINE);

	ViewPropertiesDlg_WebView2 webView2Dialog;
	webView2Dialog.DoModal();
}

IMPLEMENT_TESTAGENTFUNC(zone_unblock)
{
	DOLogX log(DOLOG_FUNCLINE);

	auto targetFile = GetValueFromConfig(testCfg, "testfile_path");
	auto result = DOUtilFile::UnblockFile(&log, targetFile);

	log.Print(DOLOG_FUNCLINE, L"<result=%d>", result);
}

IMPLEMENT_TESTAGENTFUNC(notification_center_test)
{
	DOLogX log(DOLOG_FUNCLINE);

	g_mainDlg->GetNotificationCenter()->PostMessage(WM_KEYDOWN, VK_F3, 0);
}

IMPLEMENT_TESTAGENTFUNC(notification_center_trayicon)
{
	DOLogX log(DOLOG_FUNCLINE);

	auto showStr = GetValueFromConfig(testCfg, "show");
	bool show = showStr == L"true";
	
	TrayIcon& ti = TrayIcon::GetInstance();
	ti.UpdateModeData_for_newNotification(show);
	if (show)
		ti.Update(TrayIcon::DM_NewNotification);
	else
		ti.Update(TrayIcon::DM_ToBeDetermine);
}

IMPLEMENT_TESTAGENTFUNC(rpc_get_object_by_fullpathname)
{
	DOLogX log(DOLOG_FUNCLINE);
	
	auto path = GetValueFromConfig(testCfg, "path");
	auto driveObject = g_msgpackClient->GetObjectByFullPathName(log, path);

	CString result;
	result.Format(
		L"driveObject != nullptr : %d\r\n"
		L"driveObject->objName.empty() : %d\r\n"
		L"driveObject->objName : %s\r\n", 
		driveObject != nullptr,
		driveObject->objName.empty(),
		driveObject->objName.c_str()
	);
	
	MessageBox(0, result, L"Result", MB_OK);
}

IMPLEMENT_TESTAGENTFUNC(rpc_get_parentsLastUpdateTime)
{
	std::thread t([&]
	{
		DOLogX log(DOLOG_FUNCLINE);
		for (int i = 0; i < 1000; ++i)
		{
			wchar_t updateTimeBuff[MAX_PATH * 100] = { 0, };
			auto returnVOArray = g_msgpackClient->GetParentsLastUpdateTime(log, L"S_ROOT", FOLDER_FILESYSTEM_TYPE, updateTimeBuff);
		}
	});
	t.detach();
}

IMPLEMENT_TESTAGENTFUNC(rpc_get_RsaPublicKey)
{
	DOLogX log(DOLOG_FUNCLINE);

	CString rsaPublicKey = L"";
	CString nance = L"";

	// NonceAndPublicKeyPtrArray nonceAndPublicKey;
	// auto nonceAndPublicKey = g_msgpackClient->GetNonceAndPublicKey(log);

	// DWORD rtn = g_msgpackClient->GetNonceAndPublicKey(log, rsaPublicKey, nance);

	CString csFullPath = L"Software\\AppDataLow\\Software\\DestinyECM\\DestinyECMAgent\\Accounts";
	CString csValue = DOUtilReg::RegGetStrValue((LPCTSTR)csFullPath, INI_KEY_ACCOUNT_AUTOLOGINTOKEN, L"");
	

	DestinyMsgpack::NonceAndPublicKeyPtr nonceAndPublicKey = g_msgpackClient->GetNonceAndPublicKey(log);

	CString rsaEncryptedPassword = CString(nonceAndPublicKey->nonceToken.c_str()) + _T("@") + csValue;

	// 공개키 문자열 길이 + 1 (NULL 종료)
	size_t publicKeyLen = nonceAndPublicKey->publicKey.length();
	wchar_t* publicKey = new wchar_t[publicKeyLen + 1];
	wmemset(publicKey, 0, publicKeyLen + 1);

	wcsncpy_s(publicKey, publicKeyLen + 1, nonceAndPublicKey->publicKey.c_str(), _TRUNCATE);

	rsaEncryptedPassword = DOUtil::RSAEncrypt(rsaEncryptedPassword, publicKey);


	DestinyMsgpack::XmlRpcLoginContextPtr rpcLoginContext = nullptr;
	rpcLoginContext = g_msgpackClient->TryLoginByAutoLoginToken(rsaEncryptedPassword);


}

IMPLEMENT_TESTAGENTFUNC(websocket_ping)
{
	g_mainDlg->SendPingMessageToWebSocketServer();
}

void TestAgent::run()
{
	string id = testCfg_["test_id"].as<string>();
//  TRACE(L"test_id: %s\n", S2WC(id));

	DECLARE_TESTAGENTFUNC(hidpi)
	DECLARE_TESTAGENTFUNC(session_check)
	DECLARE_TESTAGENTFUNC(refresh_not_saved_doclist_dlg)
	DECLARE_TESTAGENTFUNC(shutdown_applications_for_update)
	DECLARE_TESTAGENTFUNC(performance_test)

	DECLARE_TESTAGENTFUNC(ecmif_getfilestatus)
	DECLARE_TESTAGENTFUNC(ecmif_getfiledownloadpath)
	DECLARE_TESTAGENTFUNC(ecmif_log_pidlist_using_filehandle)
	DECLARE_TESTAGENTFUNC(ecmif_req_running_app_for_loginstatus)
	DECLARE_TESTAGENTFUNC(ecmif_open_ecm_folder)
	DECLARE_TESTAGENTFUNC(ecmif_remove_ecmfile_from_db)
	DECLARE_TESTAGENTFUNC(ecmif_getecmifinfo)
	DECLARE_TESTAGENTFUNC(ecmif_createfolder_forreturndriveobject)
	DECLARE_TESTAGENTFUNC(ecmif_vdisk_req_open_file)
	DECLARE_TESTAGENTFUNC(ecmif_vdisk_get_working_filelist)
	
	DECLARE_TESTAGENTFUNC(dk_is_downloading)
	DECLARE_TESTAGENTFUNC(dk_is_lfro)
	DECLARE_TESTAGENTFUNC(decisionkeys)

	DECLARE_TESTAGENTFUNC(multiple_request_at_sametime)
	DECLARE_TESTAGENTFUNC(tasklist)
	DECLARE_TESTAGENTFUNC(wait_existing_pki_sso_login)
	DECLARE_TESTAGENTFUNC(rpc_timeout)
	DECLARE_TESTAGENTFUNC(webdialog)
	DECLARE_TESTAGENTFUNC(copy_folder_to_folder)
	DECLARE_TESTAGENTFUNC(check_update_time)
	DECLARE_TESTAGENTFUNC(gdi_handle_leak)
	DECLARE_TESTAGENTFUNC(filesystem_corrupt)
	DECLARE_TESTAGENTFUNC(utilityif_file_collect_completed)
	DECLARE_TESTAGENTFUNC(maindlg_cmd)
	DECLARE_TESTAGENTFUNC(pcfilebackup_test)
	DECLARE_TESTAGENTFUNC(pcfilebackup_realtime_policy)
	DECLARE_TESTAGENTFUNC(webview2_test)
	DECLARE_TESTAGENTFUNC(zone_unblock)
	DECLARE_TESTAGENTFUNC(notification_center_test)
	DECLARE_TESTAGENTFUNC(notification_center_trayicon)
	DECLARE_TESTAGENTFUNC(rpc_get_object_by_fullpathname)
	DECLARE_TESTAGENTFUNC(rpc_get_parentsLastUpdateTime)
	DECLARE_TESTAGENTFUNC(rpc_get_RsaPublicKey)
	DECLARE_TESTAGENTFUNC(websocket_ping)
}


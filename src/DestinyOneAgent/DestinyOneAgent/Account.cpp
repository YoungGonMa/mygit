#include "stdafx.h"
#include "Account.h"
#include <LibDOBase/DOUtilBase.h>
#include <LibDOCore/DOUtilFile.h>
#include <LibDOBase/DOUtilEncode.h>
#include <LibDOCore/DOUtilConfig.h>
#include <LibDOCore/DOLog.h>
#include <LibDOCore/DOResMsg.h>
#include "ECMAgentCommon.h"
#include "TrayIcon.h"
#include "LibDOCore/DOUtilReg.h"
#include "ShareUtil/NotificationCenterUtil.h"
// test
asdf
using namespace ghost::define;

CString BaseAccountInfo::PrintInfo() const
{
	CString msg;
	msg.Format(L"<id=%s, name=%s, oid=%s, groupOID=%s, useAutoLogin=%s, useLoginAtWindowsStart=%s>",
		id_, name_, oid_, groupOID_, DOUtilBase::ToString(useAutoLogin_, TYPE_TRUEFALSE)
		, DOUtilBase::ToString(useLoginAtWindowsStart_, TYPE_TRUEFALSE));
	return msg;
}

//////////////////////////////////////////////////////////////////////////
// DOAccount static members
CString Account::MakeAccountKeyName(const CString& szUserOID)
{
	if (szUserOID.IsEmpty())
		throw DOException(DOE_TRACE_INFO, RTN_INVALID_PARAMETER, L"NULL == szUserOID || _tcslen(szUserOID) < 1");

	CString csKeyName;
	csKeyName.Format(L"Account_%s", szUserOID);
	return csKeyName;
}

//////////////////////////////////////////////////////////////////////////
// Account Class Helper Functions
namespace
{

UINT GetCurrentTimeX()
{
	constexpr auto kEpoch = 1556668800LL; // 기준점 2019-05-01T00:00:00Z 초단위
		// UINT 4바이트로 2155-06-07T06:28:15Z까지 저장가능
		//     0x00000000: 2019-05-01T00:00:00Z
		//     0xFFFFFFFF: 2155-06-07T06:28:15Z
	auto t = CTime::GetCurrentTime().GetTime();
	return static_cast<UINT>(t - kEpoch);
}

bool HaveToClear_AutoLoginOption(UINT optionSavedTime)
{
	static const UINT periodSeconds = [] {
		UINT period = static_cast<UINT>(DOUtilReg::RegGetIntValue(HKEY_LOCAL_MACHINE, KEY_DESTINYECM_AGENT,
		kDaysOfAutoLoginOption, 0));
		if (static_cast<int>(period) <= 0)
			period = 0;
		else
		{
			constexpr UINT kMaxPeriod = 365;
			if (period > kMaxPeriod)
				period = kMaxPeriod;

			constexpr UINT kDayToSeconds = 24 * 60 * 60;
			period *= kDayToSeconds;
		}
		return period;
	} ();

	if (periodSeconds == 0)
		return false;

	UINT cur = GetCurrentTimeX();
	return (cur < optionSavedTime || cur >= (optionSavedTime + periodSeconds));
}

}

//////////////////////////////////////////////////////////////////////////
// DOAccount class
Account::Account(const CString& szKeyName)
	: DOProfileJson(KEY_DESTINYECM_AGENT_ACCOUNTS, szKeyName)
{
	Initialize();
}

Account::~Account()
{
}

void Account::Initialize()
{
	id_ = L"";
	oid_ = L"";
	name_ = L"";
	encryptedPassword_ = L"";
	encryptedPasswordForLocalLogin_ = L"";
	plainPassword_ = L"";
	groupOID_ = L"";
	groupPath_ = L"";
	saveType_ = L"";
	sessionKey_ = L"";
	minutesForMessageCheckingFrequency_ = DEFAULT_MINS_FOR_MESSAGECHECK_FREQUENCY;
	useAutoLogin_ = FALSE;
	useLoginAtWindowsStart_ = FALSE;
	loginStatus_ = LOGOUT;

	//		value : ('0' : 내부망 로그인 / '1' : 외부망 로그인 / default : '0' - 네트워크 제어 사용하지 않을 경우에는 무조건 0으로 넘어갑니다.)
	lastNetworkMode_ = 0;
	networkCtrlModeUse_ = ((CDestinyOneAgentApp*)AfxGetApp())->GetUseNetworkValue();
	useNetworkConnect_ = 0;
	ZeroMemory(networkConnectMode_, sizeof(networkConnectMode_));

	typePasswordEncryptMode_ = ENCRYPT_MODE_DES_1;

	showSystemManagerMenu_ = FALSE;

	shouldRefreshUserDataLater_ = false;
	aclCriteriaChangedAt_ = 0;
}

void Account::SetLoginStatus(const LOGIN_STATUS typeLoginErr)
{
	if (loginStatus_ == typeLoginErr)
		return;

	loginStatus_ = typeLoginErr;
	
	switch (typeLoginErr)
	{
	case WAITING_FOR_DRIVE_MOUNT:
	case LOGIN: ECMAgent::SetNormalLogInState(); break;
	case LOCAL_LOGIN: ECMAgent::SetLocalLoginState(); break;
	case SESSION_TIMEOUT:
	case SESSION_TIMEOUT_BY_DUPLICATE_LOGIN:
		TrayIcon::GetInstance().Update(TrayIcon::DM_LoginSessionTimeout); break;
	case SESSION_TIMEOUT_BY_DEPT_CHANGED:
		TrayIcon::GetInstance().Update(TrayIcon::DM_LoginSessionTimeoutByDeptChanged); break;
	case LOGOUT: ECMAgent::SetLogOutState(); break;
	case CHANGED_ACCOUNT: loginStatus_ = LOGOUT; break;
	}
}

void Account::SetSessionKey(LPCTSTR szLatestSessionKey)
{
	if (szLatestSessionKey || _tcslen(szLatestSessionKey) > 0)
	{
		if (sessionKey_ != szLatestSessionKey)
		{
			sessionKey_ = szLatestSessionKey;
		}
	}
	else
	{
		if (!sessionKey_.IsEmpty())
		{
			sessionKey_ = L"";
		}
	}
}

void Account::SetMinutesForMessageCheckingFrequency(int minutesForMessageCheckingFrequency)
{
	minutesForMessageCheckingFrequency_ = minutesForMessageCheckingFrequency;
}

void Account::ProcSave()
{
	id_.Trim();
	oid_.Trim();
	encryptedPassword_.Trim();
	encryptedPasswordForLocalLogin_.Trim();
	groupOID_.Trim();
	groupPath_.Trim();
	saveType_.Trim();
	sessionKey_.Trim();
	autoLoginToken_.Trim();

	bool useAutoLoginPrev = GetRegStringBOOL(INI_KEY_ACCOUNT_USEAUTOLOGIN);

	SetRegString(INI_KEY_ACCOUNT_USERID, id_);
	SetRegString(INI_KEY_ACCOUNT_USEROID, oid_);
	SetRegString(INI_KEY_ACCOUNT_USERNAME, name_);
	SetRegString(INI_KEY_ACCOUNT_AUTOLOGINTOKEN, autoLoginToken_);	
	SetRegProtectedString(INI_KEY_ACCOUNT_PW, encryptedPassword_);

	if(FALSE == encryptedPasswordForLocalLogin_.IsEmpty())
		SetRegProtectedString(INI_KEY_ACCOUNT_PW_LOCAL, encryptedPasswordForLocalLogin_);

	SetRegInt(INI_KEY_ACCOUNT_PW_ENC_MODE, (int)typePasswordEncryptMode_);

	SetRegStringBOOL(INI_KEY_ACCOUNT_USEAUTOLOGIN, useAutoLogin_);
	SetRegStringBOOL(INI_KEY_ACCOUNT_LOGIN_ATWINDWOSSTART, useLoginAtWindowsStart_);
	SetRegString(INI_KEY_ACCOUNT_GROUPOID, groupOID_);
	SetRegString(INI_KEY_ACCOUNT_MULTI_USER_PATH, groupPath_);
	SetRegString(INI_KEY_ACCOUNT_SAVETYPE, saveType_);
	SetRegInt(INI_KEY_ACCOUNT_MIN_FOR_MESSAGECHECK_FREQUENCY, minutesForMessageCheckingFrequency_);

	SetRegInt(INI_KEY_USE_NETCTRL, useNetworkConnect_);
	SetRegString(INI_KEY_NETCONNECT_TYPE, networkConnectMode_);

	SetRegInt64(INI_KEY_ACCOUNT_ACL_CRITERIA_CHANGED_AT, aclCriteriaChangedAt_);

	if ( !useAutoLoginPrev && useAutoLogin_)
	{
		SetRegInt(INI_KEY_SAVED_TIME_AUTOLOGIN_OPT, static_cast<int>(GetCurrentTimeX()));
	}
}

void Account::ProcLoad()
{
	Initialize();

	//////////////////////////////////////////////////////////////////////////
	// accounts_info
	id_ = GetRegString(INI_KEY_ACCOUNT_USERID);

	if (id_.IsEmpty())
		DOLog::PrintLogError(DOLOG_FUNCLINE, L"Failed to get id in registry");

	oid_ = GetRegString(INI_KEY_ACCOUNT_USEROID);
	name_ = GetRegString(INI_KEY_ACCOUNT_USERNAME);

	encryptedPassword_ = GetRegProtectedString(INI_KEY_ACCOUNT_PW);
	if (encryptedPassword_.IsEmpty())
	{
		DOLog::PrintLogError(DOLOG_FUNCLINE, L"Failed to get encrypted pw.... id:%s", id_);
		encryptedPassword_ = GetRegString(INI_KEY_ACCOUNT_PW);
	}

	encryptedPasswordForLocalLogin_ = GetRegProtectedString(INI_KEY_ACCOUNT_PW_LOCAL);
	if (encryptedPasswordForLocalLogin_.IsEmpty())
	{
		encryptedPasswordForLocalLogin_ = GetRegString(INI_KEY_ACCOUNT_PW_LOCAL);
		if(encryptedPasswordForLocalLogin_.IsEmpty())
			DOLog::PrintLogError(DOLOG_FUNCLINE, L"Failed to get encrypted password in registry");
	}

	typePasswordEncryptMode_ = (TYPE_ENCRYPT_MODE)GetRegInt(INI_KEY_ACCOUNT_PW_ENC_MODE);

	useAutoLogin_ = GetRegStringBOOL(INI_KEY_ACCOUNT_USEAUTOLOGIN);
	useLoginAtWindowsStart_ = GetRegStringBOOL(INI_KEY_ACCOUNT_LOGIN_ATWINDWOSSTART);

	groupOID_ = GetRegString(INI_KEY_ACCOUNT_GROUPOID);
	groupPath_ = GetRegString(INI_KEY_ACCOUNT_MULTI_USER_PATH);
	saveType_ = GetRegString(INI_KEY_ACCOUNT_SAVETYPE);

	minutesForMessageCheckingFrequency_ = GetRegInt(INI_KEY_ACCOUNT_MIN_FOR_MESSAGECHECK_FREQUENCY);
	if (0 >= minutesForMessageCheckingFrequency_)
		minutesForMessageCheckingFrequency_ = DEFAULT_MINS_FOR_MESSAGECHECK_FREQUENCY;


	// 네트워크 관련 값을 가져온다.
	useNetworkConnect_ = GetRegInt(INI_KEY_USE_NETCTRL);

	CString str = GetRegString(INI_KEY_NETCONNECT_TYPE);
	lstrcpyW(networkConnectMode_, (LPWSTR)(LPCWSTR)str);

	if (useAutoLogin_)
	{
		UINT optionSavedTime = static_cast<UINT>(GetRegInt(INI_KEY_SAVED_TIME_AUTOLOGIN_OPT));
		if (HaveToClear_AutoLoginOption(optionSavedTime))
		{
			useAutoLogin_ = false;
			SetRegStringBOOL(INI_KEY_ACCOUNT_USEAUTOLOGIN, false);
			SetRegString(INI_KEY_ACCOUNT_PW, L"");

			SetRegString(INI_KEY_ACCOUNT_AUTOLOGINTOKEN, L"");
		}

		autoLoginToken_ = GetRegString(INI_KEY_ACCOUNT_AUTOLOGINTOKEN);
		if (autoLoginToken_.IsEmpty())
		{
			DOLog::PrintLogError(DOLOG_FUNCLINE, L"Failed to get autoLoginToken.... id:%s", id_);			
		}
	}

	aclCriteriaChangedAt_ = GetRegInt64(INI_KEY_ACCOUNT_ACL_CRITERIA_CHANGED_AT);

	//////////////////////////////////////////////////////////////////////////
	//#TODO_48853 
	//필요 없지 않을까? 2018.09.18
	//검토 후 삭제 필요
	//id.Trim();
	//oid.Trim();
	//encryptedPassword.Trim();
	//encryptedPasswordForLocalLogin.Trim();
	//groupOID.Trim();
	//groupPath.Trim();
	//saveType.Trim();
	//////////////////////////////////////////////////////////////////////////
}

const LOGIN_STATUS Account::GetLoginStatus() const
{
	return loginStatus_;
}

void Account::SetLastLoginNetworkMode(int nNetworkMode, int nNetworkCtrlUse)
{
	StringCbPrintfW(networkConnectMode_, sizeof(networkConnectMode_), L"%d", nNetworkMode);
	useNetworkConnect_ = (DWORD)nNetworkCtrlUse;
}

void Account::SetPasswordPlain(const CString & plainPassword)
{
	plainPassword_ = plainPassword;
}

void Account::SetFlagForShowSystemManagerMenu(const BOOL show)
{
	showSystemManagerMenu_ = show;
}

void Account::SetUseAutoLogin(BOOL useAutoLogin)
{
	useAutoLogin_ = useAutoLogin;
}

void Account::SetIsLoginAtWindowsStart(BOOL isLoginAfterWindowStart)
{
	useLoginAtWindowsStart_ = isLoginAfterWindowStart;
}

void Account::SetEncryptedPassword(const CString& encryptedPW)
{
	encryptedPassword_ = encryptedPW;
}

void Account::SetEncryptedPasswordForLocalLogin(const CString& encryptedPW)
{
	encryptedPasswordForLocalLogin_ = encryptedPW;
}

void Account::SetEncryptPasswordMode(TYPE_ENCRYPT_MODE typePasswordEncryptMode)
{
	typePasswordEncryptMode_ = typePasswordEncryptMode;
}

void Account::SetUserID(const CString& id)
{
	id_ = id;
}

void Account::SetSaveType(const CString& saveType)
{
	saveType_ = saveType;
}

void Account::SetGroupPath(const CString& groupPath)
{
	groupPath_ = groupPath;
}

void Account::SetAutoLoginToken(const CString& autoLoginToken)
{
	autoLoginToken_ = autoLoginToken;
}

void Account::SetUserOID(const CString & oid)
{
	oid_ = oid;
}

void Account::SetName(const CString& name)
{
	name_ = name;
}

void Account::SetLastLoginErrorType(LOGIN_STATUS type)
{
	loginStatus_ = type;
}

void Account::SetGroupOID(const CString& groupOID)
{
	groupOID_ = groupOID;
}

void Account::SetAclCriteriaChangedAt(long long aclCriteriaChangedAt)
{
	aclCriteriaChangedAt_ = aclCriteriaChangedAt;
}

void Account::RequestRefreshUserDataIfNeeded()
{
	DOLogX log(DOLOG_FUNCLINE);

	if (ShouldRefreshUserData() == false)
		return;

	int pantaResult = RTN_STATUS_FAIL;
	g_ecmIfxCaller->DoXCmd_RefreshAllUserData(log, pantaResult);

	if (pantaResult == RTN_STATUS_SUCCESS)
		SetRefreshUserDataLater(false);

	log.Result(DOLOG_FUNCLINE, pantaResult, L"");
}

bool Account::operator!=(const Account& rhs) const
{
	if (nullptr == &rhs)
	{
		if (!oid_.IsEmpty())
			return TRUE;
		return FALSE;
	}

	if (wcscmp(GetUserOID(), rhs.GetUserOID()))
		return true;
	if (wcscmp(GetSessionKey(), rhs.GetSessionKey()))
		return true;
	if (loginStatus_ != rhs.GetLoginStatus())
		return true;
	if (name_ != rhs.GetName())
		return true;
	if (encryptedPasswordForLocalLogin_ != rhs.encryptedPasswordForLocalLogin_)
		return true;
	if (encryptedPassword_ != rhs.encryptedPassword_)
		return true;
	return false;
}

const BOOL Account::IsSame(const Account* pActiveAccount) const
{
	if (nullptr == pActiveAccount)
	{
		if (!oid_.IsEmpty())
			return FALSE;
		return TRUE;
	}

	if (oid_.Compare(pActiveAccount->GetUserOID()))
		return FALSE;
	if (sessionKey_.Compare(pActiveAccount->GetSessionKey()))
		return FALSE;
	if (loginStatus_ != pActiveAccount->GetLoginStatus())
		return FALSE;
	if (name_ != pActiveAccount->GetName())
		return FALSE;
	return TRUE;
}

void Account::operator=(const TryLoginInfo& rhs) noexcept
{
	id_ = rhs.id_;
	oid_ = rhs.oid_;
	name_ = rhs.name_;
	plainPassword_ = rhs.plainPassword_;
	encryptedPassword_ = rhs.encryptedPassword_;

	groupOID_ = rhs.groupOID_;

	useAutoLogin_ = rhs.useAutoLogin_;
	useLoginAtWindowsStart_ = rhs.useLoginAtWindowsStart_;
}

Account& Account::operator=(const Account& rhs) noexcept
{
	if (this == &rhs)
		return *this;

	id_ = rhs.id_;
	oid_ = rhs.oid_;
	plainPassword_ = rhs.plainPassword_;
	encryptedPassword_ = rhs.encryptedPassword_;
	groupOID_ = rhs.groupOID_;
	name_ = rhs.name_;
	useAutoLogin_ = rhs.useAutoLogin_;
	autoLoginToken_ = rhs.autoLoginToken_;
	useLoginAtWindowsStart_ = rhs.useLoginAtWindowsStart_;

	encryptedPasswordForLocalLogin_ = rhs.encryptedPasswordForLocalLogin_;
	groupPath_ = rhs.groupPath_;
	sessionKey_ = rhs.sessionKey_;
	loginStatus_ = rhs.loginStatus_;
	
	lastNetworkMode_ = rhs.lastNetworkMode_;
	networkCtrlModeUse_ = rhs.networkCtrlModeUse_;

	useNetworkConnect_ = rhs.useNetworkConnect_;
	memcpy_s(networkConnectMode_, MAX_PATH * sizeof(WCHAR), rhs.networkConnectMode_, MAX_PATH * sizeof(WCHAR));
	
	typePasswordEncryptMode_ = rhs.typePasswordEncryptMode_;
	
	saveType_ = rhs.saveType_; // 쓰이긴 하나?
	showSystemManagerMenu_ = rhs.showSystemManagerMenu_;

	return *this;
}
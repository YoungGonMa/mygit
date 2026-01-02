#pragma once

// test

#include <LibDOCore/DOProfileJson.h>
#include <LibDOBase/DODefineBase.h>
#include "DestinyOneAgent.h"


#define INI_SECTION_ACCOUNT_INFO L"account_info"
#define	INI_KEY_ACCOUNT_USERID L"user_id"
#define	INI_KEY_ACCOUNT_USEROID L"user_oid"
#define	INI_KEY_ACCOUNT_USEAUTOLOGIN L"use_auto_login"
#define	INI_KEY_ACCOUNT_LOGIN_ATWINDWOSSTART L"login_at_windowstart"
#define	INI_KEY_ACCOUNT_AUTOLOGINTOKEN L"autoLoginToken"
#define	INI_KEY_ACCOUNT_PW L"pw"
#define	INI_KEY_ACCOUNT_PW_LOCAL L"pwLocal"
#define	INI_KEY_ACCOUNT_PW_ENC_MODE L"pw_enc_mode"
#define	INI_KEY_ACCOUNT_GROUPOID L"group_oid"
#define	INI_KEY_ACCOUNT_MULTI_USER_PATH L"group_path"
#define	INI_KEY_ACCOUNT_SAVETYPE L"save_type"
#define	INI_KEY_ACCOUNT_MIN_FOR_MESSAGECHECK_FREQUENCY L"minutes_for_message_checking_frequency" // 분을 기준으로한 값으로 서버에 신규메시지가 있는지를 물어보는 주기를 나타낸다.
#define INI_KEY_ACCOUNT_AUTOOPEN_DOCLIST L"auto_open_doc_list"
#define	INI_KEY_USE_NETCTRL				L"use_net_ctrl"
#define	INI_KEY_NETCONNECT_TYPE			L"net_connect_type"
#define	INI_KEY_SAVED_TIME_AUTOLOGIN_OPT	L"savet_auto_login"
#define INI_KEY_ACCOUNT_IS_EXECUTED_SCHEDULED_PCCS L"is_executed_scheduled_pccs" //지정일 수집일에 따른 수집 시행 여부 <서버에서 받지 않고 사용하는 키값>
#define INI_KEY_ACCOUNT_PC_FILE_COLLECT_TYPE L"pc_file_collect_type" //pc파일수집정책
#define INI_KEY_ACCOUNT_PC_FILE_COLLECT_TYPE_EXT L"pc_file_collect_type_ext" //pc파일수집정책에 따른 부가적인 값
#define INI_SECTION_ACCOUNTS L"accounts_info"
#define	INI_KEY_ACCOUNTS_ACTIVE_USERID L"active_user_id" // 가장 최근 로그인한 사용자의 id
#define	INI_KEY_ACCOUNTS_ACTIVE_GROUPOID L"active_group_oid" // 가장 최근 로그인한 사용자의 group oid
#define	INI_KEY_ACCOUNTS_LASTESTLOGOUT_USEROID L"latestlogout_user_oid" // 가장 최근 로그아웃한 사용자의 oid
#define	INI_KEY_ACCOUNTS_LASTESTLOGOUT_MODE L"latestlogout_mode" // 가장 최근 로그아웃한 이유 : TYPE_DO_LOGOUT_MODE
#define INI_KEY_ACCOUNT_USERNAME L"user_name"
#define INI_KEY_ACCOUNT_ACL_CRITERIA_CHANGED_AT L"acl_criteria_changed_at" //Acl 기준 변경 일시

// TryLoginInfo와 Account객체에 중복적으로 사용되는 계정정보들을 추려낸 구조체.
struct BaseAccountInfo {
	CString id_;
	CString oid_;
	CString plainPassword_;
	CString encryptedPassword_;
	CString groupOID_;
	CString name_;

	bool useAutoLogin_;
	bool useLoginAtWindowsStart_;

	virtual CString PrintInfo() const;
};

class Accounts;
//////////////////////////////////////////////////////////////////////////
// class DOAccount
// function : 한번이라도 로그인이 성공한 사용자계정정보를 관리하기 위해 새로 구성
//            기존에는 한개의 파일에 모든 계정정보를 암시적인 라인을 매칭하는 방법으로 저장했지만
//            메타의 신규 추가, 삭제시 이를 반영할 수 없는 관리상의 문제점이 발생하고,
//            다중 쓰레드에 의해 동기화 없이 저장되어 무결성을 훼손할 수 있는 잠재적인 문제점을 수반
//            이를 해결하기 위해 Account, Accounts로 양분한 ini에 기조한 profile 파일을 이용하여 관리
//            또한 사용자계정관리에 필요한 모든 정보를 포함하도록 수정
// remark : 모든 에러는 DOException으로 처리된다.
//          호출측에서는 항상 exception을 고려하자.
class Account : public DOProfileJson, public BaseAccountInfo
{
public:
	static CString MakeAccountKeyName(const CString& szUserOID);
	bool operator!=(const Account& rhs) const;
	const BOOL IsSame(const Account* rhs) const;

public:
	Account(const CString& szKeyName);
	virtual ~Account();

protected:
	virtual void ProcSave() override;
	virtual void ProcLoad() override;

public:
	const LOGIN_STATUS GetLoginStatus() const;

	void SetLoginStatus(const LOGIN_STATUS typeLoginErr);

	const BOOL IsLogOut() const { if (loginStatus_ == LOGIN_STATUS::LOGOUT) return TRUE; return FALSE; }
	const BOOL IsSessionTimeOut() const { if (loginStatus_ == LOGIN_STATUS::SESSION_TIMEOUT) return TRUE; return FALSE; }
	const BOOL IsSessionTimeOutByDeptChanged() const { if (loginStatus_ == LOGIN_STATUS::SESSION_TIMEOUT_BY_DEPT_CHANGED) return TRUE; return FALSE; }

	BOOL UseAutoLogin() const { return useAutoLogin_; }
	void SetUseAutoLogin(BOOL useAutoLogin);

	BOOL IsTryLoginAtWindowsStart() const { return useLoginAtWindowsStart_; }
	void SetIsLoginAtWindowsStart(BOOL isLoginAtWindowStart);

	LPCTSTR GetEncryptedPassword() const { return encryptedPassword_; }
	void SetEncryptedPassword(const CString& encryptedPW);

	LPCTSTR GetEncryptedPasswordForLocalLogin() const { return encryptedPasswordForLocalLogin_; }
	void SetEncryptedPasswordForLocalLogin(const CString& encryptedPW);

	TYPE_ENCRYPT_MODE GetEncryptPasswordMode() const { return typePasswordEncryptMode_; }
	void SetEncryptPasswordMode(TYPE_ENCRYPT_MODE typePasswordEncryptMode);

	LPCTSTR GetUserID() const { return id_; }
	void SetUserID(const CString& id);

	LPCTSTR GetGroupOID() const { return groupOID_; }
	void SetGroupOID(const CString& groupOID);

	// Deprecated
	LPCTSTR GetSaveType() const { return saveType_; }
	void SetSaveType(const CString& szAccountSaveType);

	LPCTSTR GetGroupPath() const { return groupPath_; }
	void SetGroupPath(const CString& groupPath);

	LPCTSTR GetUserOID() const { return oid_; }
	void SetUserOID(const CString& oid);

	LPCTSTR GetName() const { return name_; }
	void SetName(const CString& name);

	void SetLastLoginErrorType(LOGIN_STATUS type);

	LPCTSTR GetSessionKey() const { return sessionKey_; }
	bool IsSessionKeyEmpty() const { return sessionKey_.IsEmpty(); }
	void SetSessionKey(LPCTSTR sessionKey);

	LPCTSTR GetAutoLoginToken() const { return autoLoginToken_; }
	void SetAutoLoginToken(const CString& autoLoginToken);
	
	// Deprecated.
	int GetMinutesForMessageCheckingFrequency() const { return minutesForMessageCheckingFrequency_; }
	void SetMinutesForMessageCheckingFrequency(int minutesForMessageCheckingFrequency);

	const DWORD GetUseNetworkConnectValue() const { return useNetworkConnect_; }
	const WCHAR* GetNetworkConnectMode() const { return networkConnectMode_; }

	void SetLastLoginNetworkMode(int nNetworkMode, int nNetworkCtrlUse);

	LPCTSTR GetPasswordPlain() const { return plainPassword_; }
	void SetPasswordPlain(const CString& plainPassword);

	BOOL ShowSystemManagerMenu() const { return showSystemManagerMenu_; }
	void SetFlagForShowSystemManagerMenu(const BOOL show);

    // 겸직부서
	CString SetMultiUserSuggest(CString multiUserSuggest) {
		CString previous = multiUserSuggest_; 
		multiUserSuggest_ = multiUserSuggest;
		return previous;

	};
	CString GetMultiUserSuggest() {
		return multiUserSuggest_;
	}

	long long GetAclCriteriaChangedAt() const { return aclCriteriaChangedAt_; }
	void SetAclCriteriaChangedAt(long long aclCriteriaChangedAt);

	void SetRefreshUserDataLater(const bool flag) { shouldRefreshUserDataLater_ = flag; }
	bool ShouldRefreshUserData() const { return shouldRefreshUserDataLater_; }
	void RequestRefreshUserDataIfNeeded();

	void operator=(const TryLoginInfo& rhs) noexcept;
	Account& operator=(const Account& rhs) noexcept;

private:
	void Initialize();

private:
	CString encryptedPasswordForLocalLogin_;
	CString groupPath_;
	CString sessionKey_;
	CString autoLoginToken_;
	LOGIN_STATUS loginStatus_;
	CString multiUserSuggest_; // ex. E000|사이버다임^1Nu5ILQqLCk|사이버다임>개발팀                               
	int		lastNetworkMode_;
	int		networkCtrlModeUse_;

	DWORD	useNetworkConnect_;
	WCHAR	networkConnectMode_[MAX_PATH];

	TYPE_ENCRYPT_MODE typePasswordEncryptMode_;

	long long aclCriteriaChangedAt_;
	bool shouldRefreshUserDataLater_; // 팬타랩 드라이브 마운트 후 사용자 데이터 리프래시가 필요한지 여부 (aclCriteriaChangedAt 에 의해 결정됨)

	CString saveType_;	// Deprecated.
	int minutesForMessageCheckingFrequency_; // Deprecated.
	//////////////////////////////////////////////////////////////////////////
	// profile 파일에 저장되지 않는 내용
	BOOL showSystemManagerMenu_;

	friend class Accounts;
};

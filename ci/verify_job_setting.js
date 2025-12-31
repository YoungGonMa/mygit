/*
	젠킨스 Job 설정을 잘못했을 경우 파일들이 덮어쓰여지는 것을 방지하기 위해 만들어진 스크립트
	소스코드 checkout 전에 실행되어야 하므로, 사실상 이 위치 (소스폴더/ci)에 있어도 의미가 없다.
	
	d:\_jenkins-global 에 복사해놓고 이것을 실행하도록 함.
*/

/*
	WORKSPACE=d:\_jenkins-workspaces\ecm_agent_trunk_release
	JOB_NAME=ecm_agent_trunk_release
	SVN_URL_1=https://stardust.cyberdigm.co.kr/DestinyActiveX/DestinyOneX/DestinyECM/trunk
	SVN_URL_2=https://stardust.cyberdigm.co.kr/DestinyONE/trunk/DestinyECM/src/main/webapp/client_one_x
	SVN_URL_3=https://stardust.cyberdigm.co.kr/DestinyONE/trunk/cloudium/src/main/webapp/client_one_x
	SVN_URL_4=https://stardust.cyberdigm.co.kr/STARTIA/Agent/trunk
	SVN_URL_5=https://stardust.cyberdigm.co.kr/KTBIZEZ/Agent/trunk
*/

var pc = process.env.USERDOMAIN;
if (pc != 'NCI-03') {
	console.log(`It seems not CI-server, skipping verify.`);
	process.exit(0);
	return;
}

var job = process.env.JOB_NAME;
var folder = process.env.WORKSPACE;
var folderPieceArray = folder.split('\\');
if (folderPieceArray[folderPieceArray.length - 1] != job) {
	console.error(`WorkSpace does not match with JobName!!!`);
	console.error(`- WorkSpace: ${folder}`);
	console.error(`- JobName: ${job}`);
	process.exit(1);
	return;
}

var svn = [
	process.env.SVN_URL_1,
	process.env.SVN_URL_2,
	process.env.SVN_URL_3,
	process.env.SVN_URL_4,
	process.env.SVN_URL_5,
];

if (svn[1].indexOf('/DestinyECM/src/main/webapp/client_one_x') < 0) {
	console.error(`SVN_URL_2 does not contain ECM folder structure!!!`);
	console.error(`- SVN_URL_2: ${svn[1]}`);
	process.exit(1);
	return;
}

if (svn[2].indexOf('/cloudium/src/main/webapp/client_one_x') < 0) {
	console.error(`SVN_URL_3 does not contain cloudium folder structure!!!`);
	console.error(`- SVN_URL_3: ${svn[2]}`);
	process.exit(1);
	return;
}

if (svn[3].indexOf('/STARTIA/Agent/') < 0) {
	console.error(`SVN_URL_4 does not contain STARTIA folder structure!!!`);
	console.error(`- SVN_URL_4: ${svn[3]}`);
	process.exit(1);
	return;
}

if (svn[4].indexOf('/KTBIZEZ/Agent/') < 0) {
	console.error(`SVN_URL_5 does not contain KTBIZEZ folder structure!!!`);
	console.error(`- SVN_URL_5: ${svn[4]}`);
	process.exit(1);
	return;
}

console.log(`Everything seems fine.`);
process.exit(0);
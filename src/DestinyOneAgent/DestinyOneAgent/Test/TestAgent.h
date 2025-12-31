#pragma once

// #TODO: reimplement for more general test.

#define IMPLEMENT_TESTAGENTFUNC(_name)	static void test_##_name(const YAML::Node& testCfg) 
#define DECLARE_TESTAGENTFUNC(_name)	if (id == #_name) { test_##_name(testCfg_); }

class TestAgent
{
public:
	TestAgent(const fs::path& cfgFile);
	~TestAgent();

	void run();

private:
	YAML::Node testCfg_;
};


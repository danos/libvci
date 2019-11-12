// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

#include <iostream>
#include <functional>
#include <mutex>

#include <vci.hpp>

class Config : public vci::Config {
public:
	void set(const std::string &in) throw (vci::Exception) {
		std::lock_guard<std::mutex> lock(this->_mu);
		_config = in;
	}
	void check(const std::string &) throw (vci::Exception) {
		return;
	}
	std::string get() {
		std::lock_guard<std::mutex> lock(this->_mu);
		return _config;
	}
private:
	std::string _config;
	std::mutex _mu;
};

class State : public vci::State {
public:
	std::string get() {
		return "{\"state\":\"foobar\"}";
	}
};

class RPC1 : public vci::Method {
public:
	std::string operator()(const std::string &in) {
		return in;
	}
};

std::string rpcFail(const std::string &) {
	throw(vci::Exception("cppexample-rpc-failure", "This RPC always fails", ""));
}

int main() {
	try {
		vci::Component("net.vyatta.vci.cppexample")
			.model(vci::Model("net.vyatta.vci.cppexample.v1")
				   .config(new Config())
				   .state(new State())
				   .rpc("cppexample", "rpc1", new RPC1()) //As vci::Method
				   .rpc("cppexample", "rpc2", RPC1()) //As std::function
				   .rpc("cppexample", "rpc-fail", &rpcFail)
				   .rpc("cppexample", "lambda",
						[](const std::string &in)->std::string {
							return in;
						}))
			.subscribe("toaster", "toast-done",
					   [](const std::string &in) {
						   std::cout << "toast-done: " << in << std::endl;
					   })
			.run()
			.wait();
	} catch (const vci::Exception &e) {
		std::cout << e.what() << std::endl;
		return 1;
	}
	return 0;
}

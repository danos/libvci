// Copyright (c) 2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

%module(directors="1") vci

%{
#define SWIG_FILE_WITH_INIT
#include "../../vci.hpp"
#include "helpers.hpp"
%}

%include "std_string.i"
%include "std_except.i"

namespace vci {
	 %ignore Config;
	 %ignore State;
	 %ignore Method;
	 %ignore Component;
	 %ignore Model;
	 %ignore Subscription;
	 %ignore Subscriber;
	 %ignore RPCCall;
	 %ignore Client::subscribe;
	 %ignore Client::call;
	 %ignore Client::config_by_model;
	 %ignore Client::state_by_model;

	 %typemap(in) const EncodedInput & (int res = 0){
		auto str = perl_encode_object($input);
		$1 = new std::string(str); //SWIG frees this
	 }

	 %typemap(out) EncodedOutput {
		$result = perl_decode_object($1);
	 }
}

%include "../../vci.hpp"

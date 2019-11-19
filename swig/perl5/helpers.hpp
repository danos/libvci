// Copyright (c) 2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>
#include "../../vci.hpp"

class OwnedPerlObject {
	//This class simplifies reference cleanup of perl objects.
public:
	OwnedPerlObject() : OwnedPerlObject(NULL) {}
	OwnedPerlObject(SV *obj) : _obj(obj){}
	~OwnedPerlObject() {
		SvREFCNT_dec(_obj);
	}
	SV *get() { return _obj; }
private:
	SV *_obj;
};

static
std::string perl_str_to_string(SV *obj) {
	if (obj == NULL) {
		return "";
	}
	return SvPV_nolen(obj);
}

SV *perl_call_json_single(const char *sub_name, SV *input) {
	// See "perlcall" for the particulars regarding this.
	SV *output;
	int count;
	load_module(0, newSVpv("JSON", 4), NULL, (SV*) NULL);
	dSP;						// initialize stack pointer
	ENTER;						// enter a scope
	SAVETMPS;					// save temporaries
	PUSHMARK(SP);				// save the stack pointer
	EXTEND(SP, 1);				// make the stack larger by one
	PUSHs(input);				// push the SV onto the stack
	PUTBACK;					// make the stack pointer global
	count = call_pv(sub_name, G_SCALAR); // call the function
	if (count != 1){
		// TODO: throw an error
	}
	SPAGAIN;					// refresh the stack pointer
	output = newSVsv(POPs);		// pop the variable
	PUTBACK;					// make stack pointer global
	FREETMPS;					// free any temporaries
	LEAVE;						// leave the created scope
	return output;
}

SV *perl_decode_object(const std::string &encoded_input) {
	const char *from_json = "from_json";
	SV *input = newSVpv(encoded_input.c_str(), encoded_input.length());
	return perl_call_json_single(from_json, sv_2mortal(input));
}

std::string perl_encode_object(SV *obj) {
	const char *to_json = "to_json";
	OwnedPerlObject output = perl_call_json_single(to_json, obj);
	return perl_str_to_string(output.get());
}

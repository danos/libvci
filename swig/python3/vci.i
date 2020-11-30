// Copyright (c) 2018-2019, AT&T Intellectual Property.
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
%include "std_shared_ptr.i"
%include "std_except.i"
%include "exception.i"

%shared_ptr(vci::RPCCall);
%shared_ptr(vci::Subscription);
%shared_ptr(vci::Client);

%feature("director:except") {
	if ($error != NULL) {
		py_handle_ex();
	}
}

%exception {
    try {
        $action
    } catch (const vci::Exception &ex) {
        OwnedPyObject py_ex_type = py_get_vci_ex_type();
        OwnedPyObject py_ex = vci_ex_to_py_vci_ex(ex);
        PyErr_SetObject(py_ex_type.get(), py_ex.get());
        SWIG_fail;
    } catch (const std::exception& ex) {
        SWIG_exception_fail(SWIG_SystemError, (&ex)->what());
    } catch(...) {
        SWIG_exception_fail(SWIG_RuntimeError,"unknown exception");
    }
}

namespace vci {
	 %rename("_vci_exception") Exception;

	 %feature("director") Config;
	 %feature("director") State;
	 %feature("director") Method;
	 %feature("director") Subscriber;

	 %typemap(directorout) EncodedOutput {
		 // Convert from a python object to a string using the
		 // JSON package
		 $result = py_encode_object($1);
	 }

	 %typemap(in) const EncodedInput & (int res = 0){
		auto str = py_encode_object($input);
		$1 = new std::string(str); //SWIG frees this
	 }

	 %typemap(out) EncodedOutput {
		$result = py_decode_object($1);
	 }

	 %typemap(typecheck, precedence=0) vci::Method * {
		 $1 = PyCallable_Check($input);
	 }

	 %typemap(in, noblock=1) vci::Config * (void  *argp = 0, int res = 0) {
		Py_INCREF($input);
		res = SWIG_ConvertPtr($input, &argp, $descriptor, $disown | %convertptr_flags);
		if (!SWIG_IsOK(res)) {
			%argument_fail(res, "$type", $symname, $argnum);
		}
		$1 = %reinterpret_cast(argp, $ltype);
	 }

	 %typemap(in, noblock=1) vci::State * (void  *argp = 0, int res = 0) {
		Py_INCREF($input);
		res = SWIG_ConvertPtr($input, &argp, $descriptor, $disown | %convertptr_flags);
		if (!SWIG_IsOK(res)) {
			%argument_fail(res, "$type", $symname, $argnum);
		}
		$1 = %reinterpret_cast(argp, $ltype);
	 }

	 %typemap(in) vci::Method * {
		 Py_INCREF($input);
		 $1 = py_gen_method($input);
	 }

	 %typemap(typecheck, precedence=0) vci::Subscriber * {
		 $1 = PyCallable_Check($input);
	 }

	 %typemap(in) vci::Subscriber * {
		 Py_INCREF($input);
		 $1 = py_gen_subscriber($input);
	 }

	 %typemap(directorin) const EncodedInput & {
		 // For automatic decoding we use the name "encoded_input" to
		 // signal to this layer that it should decode the
		 // value. SWIG's python object will decref this when it is
		 // done with the call.
		 $input = py_decode_object($1);
	 }

	 %typemap(in) SWIGTYPE const *self (PyObject *_global_self=0, $1_type _global_in=0) {
		 // Override the below typemap for 'const *' due to assignment
		 // differences.
		 $typemap(in, $1_type);
	 }

	 %typemap(in) SWIGTYPE *self (PyObject *_global_self=0, $&1_type _global_in=0) {
		 // Store a reference to the current object so we can return it
		 // in the case that we are dealing with at "fluent" style API.
		 $typemap(in, $1_type);
		 _global_self = $input;
		 _global_in = &$1;
	 }

	 %typemap(out) FLUENT& {
		 if ($1 == *_global_in) {
			 // This was a fluent, we don't need a new object, return
			 // the old one.
			 Py_INCREF(_global_self);
			 $result = _global_self;
		 }
		 else {
			 // Looks like it wasn't really fluent here, return a new
			 // pointer object.
			 $result = SWIG_NewPointerObj($1, $descriptor, $owner);
		 }
	 }

	 %apply FLUENT& { Model& };
	 %apply FLUENT& { Component& };
}

%include "../../vci.hpp"

%pythoncode {
	class Exception(__builtin__.Exception, _vci_exception):
		def __init__(self, app_tag, info, path, *args):
			__builtin__.Exception.__init__(self, *args)
			_vci_exception.__init__(self, app_tag, info, path, *args)
		def what(self):
			return _vci_exception.what(self)
		def __repr__(self):
			return self.what()
		def __str__(self):
			return self.what()
}

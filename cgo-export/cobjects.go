// Copyright (c) 2018-2020, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

package main

import (
	"github.com/danos/mgmterror"
	"github.com/danos/vci"
	"runtime"
	"unsafe"
)

/*
#include <stdlib.h>
#include <stdint.h>
#include "../vci.h"

void
_vci_subscriber_call(vci_subscriber_object *sub, char *in)
{
	sub->subscriber(sub->obj, in);
}

void
_vci_subscriber_free_call(vci_subscriber_object *sub)
{
	if (sub->free == NULL) {
		return;
	}
	sub->free(sub->obj);
}

int
_vci_config_set_call(vci_config_object *config, char *in, vci_error *err)
{
	return config->set(config->obj, in, err);
}

int
_vci_config_check_call(vci_config_object *config, char *in, vci_error *err)
{
	return config->check(config->obj, in, err);
}

void
_vci_config_get_call(vci_config_object *config, char **out)
{
	if (config->get != NULL) {
		config->get(config->obj, out);
	}
}

void
_vci_config_free_call(vci_config_object *config)
{
	if (config->free == NULL) {
		return;
	}
	config->free(config->obj);
}

void
_vci_state_get_call(vci_state_object *state, char **out)
{
	state->get(state->obj, out);
}

void
_vci_state_free_call(vci_state_object *state)
{
	if (state->free == NULL) {
		return;
	}
	state->free(state->obj);
}

int
_vci_rpc_call(vci_rpc_object *rpc, char *in, char **out, vci_error *err)
{
	return rpc->call(rpc->obj, in, out, err);
}

void
_vci_rpc_free_call(vci_rpc_object *rpc)
{
	if (rpc->free == NULL) {
		return;
	}
	rpc->free(rpc->obj);
}
*/
import "C"

type encodedString []byte

func (s *encodedString) UnmarshalJSON(data []byte) error {
	*s = encodedString(data)
	return nil
}

func (s encodedString) MarshalJSON() ([]byte, error) {
	if s == nil {
		return []byte("null"), nil
	}
	return s, nil
}

func (s *encodedString) UnmarshalRFC7951(data []byte) error {
	*s = encodedString(data)
	return nil
}

func (s encodedString) MarshalRFC7951() ([]byte, error) {
	if s == nil {
		return []byte("null"), nil
	}
	return s, nil
}

func cSubscriber(sub *C.vci_subscriber_object) func(encodedString) {
	subCpy := *sub
	runtime.SetFinalizer(&subCpy, func(sub *C.vci_subscriber_object) {
		C._vci_subscriber_free_call(sub)
	})
	return func(in encodedString) {
		cin := C.CString(string(in))
		C._vci_subscriber_call(&subCpy, cin)
		C.free(unsafe.Pointer(cin))
	}
}

type cconfig struct {
	cobj *C.vci_config_object
}

func (conf *cconfig) Set(in encodedString) error {
	cin := C.CString(string(in))
	defer C.free(unsafe.Pointer(cin))
	var cerr C.vci_error
	_vci_error_init(&cerr)
	defer _vci_error_free(&cerr)
	rc := C._vci_config_set_call(conf.cobj, cin, &cerr)
	if rc != 0 {
		return vci_error_to_error(&cerr)
	}
	return nil
}

func (conf *cconfig) Check(in encodedString) error {
	cin := C.CString(string(in))
	defer C.free(unsafe.Pointer(cin))
	var cerr C.vci_error
	_vci_error_init(&cerr)
	defer _vci_error_free(&cerr)
	rc := C._vci_config_check_call(conf.cobj, cin, &cerr)
	if rc != 0 {
		return vci_error_to_error(&cerr)
	}
	return nil
}

func (conf *cconfig) Get() encodedString {
	var cout *C.char
	defer func() { C.free(unsafe.Pointer(cout)) }()
	C._vci_config_get_call(conf.cobj, &cout)
	return encodedString(C.GoString(cout))
}

func (conf *cconfig) free() {
	C._vci_config_free_call(conf.cobj)
}

func cConfig(cobj *C.vci_config_object) *cconfig {
	tmp := *cobj
	out := &cconfig{cobj: &tmp}
	runtime.SetFinalizer(out, func(conf *cconfig) {
		conf.free()
	})
	return out
}

type cstate struct {
	cobj *C.vci_state_object
}

func (state *cstate) Get() encodedString {
	var cout *C.char
	defer func() { C.free(unsafe.Pointer(cout)) }()
	C._vci_state_get_call(state.cobj, &cout)
	return encodedString(C.GoString(cout))
}

func (state *cstate) free() {
	C._vci_state_free_call(state.cobj)
}

func cState(cobj *C.vci_state_object) *cstate {
	tmp := *cobj
	out := &cstate{cobj: &tmp}
	runtime.SetFinalizer(out, func(state *cstate) {
		state.free()
	})
	return out
}

type model struct {
	vci.Model
	rpcs map[string]*crpc
}

func newModel(mod vci.Model) *model {
	return &model{
		Model: mod,
		rpcs:  make(map[string]*crpc),
	}
}

func (m *model) addRPC(moduleName, rpcName string, crpc_obj *C.vci_rpc_object) {
	crpc, ok := m.rpcs[moduleName]
	if !ok {
		m.rpcs[moduleName] = cRPC()
		crpc = m.rpcs[moduleName]
	}
	crpc.addRPC(rpcName, crpc_obj)
}

func (m *model) getModuleRPCs(moduleName string) *crpc {
	return m.rpcs[moduleName]
}

type crpc struct {
	rpcs map[string]interface{}
}

func cRPC() *crpc {
	return &crpc{
		rpcs: make(map[string]interface{}),
	}
}

func (rpc *crpc) addRPC(name string, cRPC *C.vci_rpc_object) {
	rpcCpy := *cRPC
	runtime.SetFinalizer(&rpcCpy, func(rpc *C.vci_rpc_object) {
		C._vci_rpc_free_call(rpc)
	})
	rpc.rpcs[name] = func(in encodedString) (encodedString, error) {
		cin := C.CString(string(in))
		defer C.free(unsafe.Pointer(cin))
		var cout *C.char
		defer func() { C.free(unsafe.Pointer(cout)) }()
		var cerr C.vci_error
		_vci_error_init(&cerr)
		defer _vci_error_free(&cerr)
		rc := C._vci_rpc_call(&rpcCpy, cin, &cout, &cerr)
		if rc != 0 {
			return encodedString(""), vci_error_to_error(&cerr)
		}
		return encodedString(C.GoString(cout)), nil
	}
}

func (rpc *crpc) RPCs() map[string]interface{} {
	return rpc.rpcs
}

func vci_error_to_error(cerr *C.vci_error) error {
	err := mgmterror.NewOperationFailedApplicationError()
	err.AppTag = C.GoString(cerr.app_tag)
	err.Path = C.GoString(cerr.path)
	err.Message = C.GoString(cerr.info)
	return err
}

func error_to_vci_error(err error, cerr *C.vci_error) {
	cerr.app_tag = C.CString("vci-failure")
	cerr.info = C.CString(err.Error())
}

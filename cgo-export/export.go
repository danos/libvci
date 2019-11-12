// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

package main

/*
#include <stdlib.h>
#include <stdint.h>
#include "../vci.h"
*/
import "C"
import (
	"github.com/danos/vci"
	"unsafe"
)

//export _vci_component_new
func _vci_component_new(name *C.char) C.uint64_t {
	return C.uint64_t(objects.Register(vci.NewComponent(C.GoString(name))))
}

//export _vci_component_free
func _vci_component_free(cd C.uint64_t) {
	objects.Unregister(OD(cd))
}

//export _vci_component_run
func _vci_component_run(cd C.uint64_t, cerr *C.vci_error) C.int {
	err := objects.Get(OD(cd)).(vci.Component).Run()
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_component_wait
func _vci_component_wait(cd C.uint64_t, cerr *C.vci_error) C.int {
	err := objects.Get(OD(cd)).(vci.Component).Wait()
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_component_stop
func _vci_component_stop(cd C.uint64_t, cerr *C.vci_error) C.int {
	err := objects.Get(OD(cd)).(vci.Component).Stop()
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_component_subscribe
func _vci_component_subscribe(
	cd C.uint64_t,
	module, name *C.char,
	sub *C.vci_subscriber_object,
	cerr *C.vci_error,
) C.int {
	err := objects.Get(OD(cd)).(vci.Component).
		Subscribe(C.GoString(module), C.GoString(name),
			cSubscriber(sub))
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_component_unsubscribe
func _vci_component_unsubscribe(
	cd C.uint64_t,
	module, name *C.char,
	cerr *C.vci_error,
) C.int {
	err := objects.Get(OD(cd)).(vci.Component).
		Unsubscribe(C.GoString(module), C.GoString(name))
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_component_model
func _vci_component_model(cd C.uint64_t, name *C.char) C.uint64_t {
	mod := objects.Get(OD(cd)).(vci.Component).Model(C.GoString(name))
	return C.uint64_t(objects.Register(newModel(mod)))
}

//export _vci_model_config
func _vci_model_config(md C.uint64_t, cobj *C.vci_config_object) {
	objects.Get(OD(md)).(vci.Model).Config(cConfig(cobj))
}

//export _vci_model_state
func _vci_model_state(md C.uint64_t, cobj *C.vci_state_object) {
	objects.Get(OD(md)).(vci.Model).State(cState(cobj))
}

//export _vci_model_rpc
func _vci_model_rpc(
	md C.uint64_t,
	modName, rpcName *C.char,
	cobj *C.vci_rpc_object,
) {
	name := C.GoString(modName)
	vciModel := objects.Get(OD(md)).(vci.Model)
	libvciModel := vciModel.(*model)
	libvciModel.addRPC(name, C.GoString(rpcName), cobj)
	vciModel.RPC(name, libvciModel.getModuleRPCs(name).RPCs())
}

//export _vci_model_free
func _vci_model_free(md C.uint64_t) {
	objects.Unregister(OD(md))
}

//export _vci_error_string
func _vci_error_string(cerr *C.vci_error) *C.char {
	err := vci_error_to_error(cerr)
	return C.CString(err.Error())
}

//export _vci_error_init
func _vci_error_init(cerr *C.vci_error) {
	cerr.app_tag = nil
	cerr.path = nil
	cerr.info = nil
}

//export _vci_error_free
func _vci_error_free(cerr *C.vci_error) {
	if cerr == nil {
		return
	}
	C.free(unsafe.Pointer(cerr.app_tag))
	C.free(unsafe.Pointer(cerr.path))
	C.free(unsafe.Pointer(cerr.info))
}

//export _vci_client_dial
func _vci_client_dial(client *C.uint64_t, cerr *C.vci_error) C.int {
	cl, err := vci.Dial()
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	*client = C.uint64_t(objects.Register(cl))
	return 0
}

//export _vci_client_free
func _vci_client_free(cd C.uint64_t) {
	client := objects.Get(OD(cd)).(*vci.Client)
	objects.Unregister(OD(cd))
	client.Close()
}

//export _vci_client_emit
func _vci_client_emit(
	cd C.uint64_t,
	module, name, data *C.char,
	cerr *C.vci_error,
) C.int {
	client := objects.Get(OD(cd)).(*vci.Client)
	err := client.Emit(
		C.GoString(module), C.GoString(name), C.GoString(data))
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_client_store_config_by_model_into
func _vci_client_store_config_by_model_into(
	cd C.uint64_t,
	model *C.char,
	output **C.char,
	cerr *C.vci_error,
) C.int {
	client := objects.Get(OD(cd)).(*vci.Client)
	var out string
	err := client.StoreConfigByModelInto(C.GoString(model), &out)
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	*output = C.CString(out)
	return 0
}

//export _vci_client_store_state_by_model_into
func _vci_client_store_state_by_model_into(
	cd C.uint64_t,
	model *C.char,
	output **C.char,
	cerr *C.vci_error,
) C.int {
	client := objects.Get(OD(cd)).(*vci.Client)
	var out string
	err := client.StoreStateByModelInto(C.GoString(model), &out)
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	*output = C.CString(out)
	return 0
}

//export _vci_client_call
func _vci_client_call(cd C.uint64_t, module, name, input *C.char) C.uint64_t {
	client := objects.Get(OD(cd)).(*vci.Client)
	rpccall := client.Call(C.GoString(module),
		C.GoString(name), C.GoString(input))
	return C.uint64_t(objects.Register(rpccall))
}

//export _vci_rpccall_free
func _vci_rpccall_free(rd C.uint64_t) {
	objects.Unregister(OD(rd))
}

//export _vci_rpccall_store_output_into
func _vci_rpccall_store_output_into(
	rd C.uint64_t,
	output **C.char,
	cerr *C.vci_error,
) C.int {
	var out string
	rpccall := objects.Get(OD(rd)).(*vci.RPCCall)
	err := rpccall.StoreOutputInto(&out)
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	*output = C.CString(out)
	return 0
}

//export _vci_client_subscribe
func _vci_client_subscribe(
	cd C.uint64_t,
	module, name *C.char,
	sub *C.vci_subscriber_object,
) C.uint64_t {
	client := objects.Get(OD(cd)).(*vci.Client)
	subscription := client.Subscribe(
		C.GoString(module), C.GoString(name), cSubscriber(sub))
	return C.uint64_t(objects.Register(subscription))

}

//export _vci_subscription_free
func _vci_subscription_free(sd C.uint64_t) {
	objects.Unregister(OD(sd))
}

//export _vci_subscription_run
func _vci_subscription_run(sd C.uint64_t, cerr *C.vci_error) C.int {
	err := objects.Get(OD(sd)).(*vci.Subscription).Run()
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_subscription_cancel
func _vci_subscription_cancel(sd C.uint64_t, cerr *C.vci_error) C.int {
	err := objects.Get(OD(sd)).(*vci.Subscription).Cancel()
	if err != nil {
		error_to_vci_error(err, cerr)
		return -1
	}
	return 0
}

//export _vci_subscription_coalesce
func _vci_subscription_coalesce(sd C.uint64_t) {
	objects.Get(OD(sd)).(*vci.Subscription).Coalesce()
}

//export _vci_subscription_drop_after_limit
func _vci_subscription_drop_after_limit(sd C.uint64_t, limit C.uint32_t) {
	objects.Get(OD(sd)).(*vci.Subscription).DropAfterLimit(int(limit))
}

//export _vci_subscription_block_after_limit
func _vci_subscription_block_after_limit(sd C.uint64_t, limit C.uint32_t) {
	objects.Get(OD(sd)).(*vci.Subscription).BlockAfterLimit(int(limit))
}

//export _vci_subscription_remove_limit
func _vci_subscription_remove_limit(sd C.uint64_t) {
	objects.Get(OD(sd)).(*vci.Subscription).RemoveLimit()
}

func main() {
	// required to compile to C shared library
}

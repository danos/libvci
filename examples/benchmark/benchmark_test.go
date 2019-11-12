// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

package main

import (
	"testing"

	"github.com/godbus/dbus"
)

func benchRPC1(b *testing.B, dest, obj, method string) {
	cl, err := dbus.SystemBus()
	if err != nil {
		b.Fatal(err)
	}
	for n := 0; n < b.N; n++ {
		var out string
		err := cl.Object(dest, dbus.ObjectPath(obj)).
			Call(method, 0, "{\"foo\":\"bar\"}").
			Store(&out)
		if err != nil {
			b.Fatal(err)
		}
	}
}

func BenchmarkPy3(b *testing.B) {
	benchRPC1(b, "net.vyatta.vci.py3example.v1",
		"/py3example/rpc", "yang.module.Py3example.RPC.Rpc1")
}

func BenchmarkC(b *testing.B) {
	benchRPC1(b, "net.vyatta.vci.cexample.v1",
		"/cexample/rpc", "yang.module.Cexample.RPC.Rpc1")
}

func BenchmarkCPP(b *testing.B) {
	benchRPC1(b, "net.vyatta.vci.cppexample.v1",
		"/cppexample/rpc", "yang.module.Cppexample.RPC.Rpc1")
}

func BenchmarkGo(b *testing.B) {
	benchRPC1(b, "net.vyatta.vci.goexample.v1",
		"/goexample/rpc", "yang.module.Goexample.RPC.Rpc1")
}

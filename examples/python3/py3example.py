#!/usr/bin/env python3

#Copyright (c) 2018-2019, AT&T Intellectual Property.
#All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-only

import vci

def rpcfail(input):
    raise vci.Exception("py3example-rpc-failure", "This RPC always fails", "")

def rpctest(input):
    return {}

def subscriber(data):
    print(type(data), data)

class Config(vci.Config):
    conf = {}
    def set(self, input):
        self.conf = input
        return

    def get(self):
        return self.conf

    def check(self, input):
        return

class State(vci.State):
    def get(self):
        return {'state':'foobar'}

(vci.Component("net.vyatta.vci.py3example")
        .model(vci.Model("net.vyatta.vci.py3example.v1")
               .config(Config())
               .state(State())
               .rpc("py3example", "rpc1", lambda x: x)
               .rpc("py3example", "rpc2", lambda x: x)
               .rpc("py3example", "rpc-fail", rpcfail)
               .rpc("py3example", "rpc-test", rpctest))
        .subscribe("toaster", "toast-done", subscriber)
        .run()
        .wait())

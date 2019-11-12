// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

package main

import (
	"github.com/danos/vci"
	"sync/atomic"
)

type Config struct {
	data atomic.Value
}

func ConfigNew() *Config {
	conf := &Config{}
	conf.data.Store("{}")
	return conf
}

func (c *Config) Get() string {
	return c.data.Load().(string)
}

func (c *Config) Set(in string) error {
	c.data.Store(in)
	return nil
}

func (c *Config) Check(in string) error {
	return nil
}

type State struct {
}

func StateNew() *State {
	return &State{}
}

func (s *State) Get() string {
	return "{\"state\":\"foobar\"}"
}

type RPC struct{}

func RPCNew() *RPC {
	return &RPC{}
}

func (r *RPC) RPC1(in string) (string, error) {
	return in, nil
}

func main() {
	conf := ConfigNew()
	state := StateNew()
	rpc := RPCNew()
	comp := vci.NewComponent("net.vyatta.vci.goexample")
	comp.Model("net.vyatta.vci.goexample.v1").
		Config(conf).
		State(state).
		RPC("goexample", rpc)
	comp.Run()
	comp.Wait()
}

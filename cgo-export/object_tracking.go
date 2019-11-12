// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

package main

import (
	"sync"
)

/*
We need to track the objects we are handing to C to keep it alive
in the go code. We use a monotonically increasing descriptor as a
reference for a compnent and keep track of it in a map. This is also
required for other types since go has a moving gc so pointers to an
object may change under the covers
*/

type OD uint64

type objectTracker struct {
	currentDescriptor OD
	objects           map[OD]interface{}
	mu                sync.RWMutex
}

func objectTrackerNew() *objectTracker {
	return &objectTracker{
		objects: make(map[OD]interface{}),
	}
}

func (ot *objectTracker) Register(object interface{}) OD {
	ot.mu.Lock()
	defer ot.mu.Unlock()
	od := ot.currentDescriptor
	ot.objects[od] = object
	ot.currentDescriptor++
	return od
}

func (ot *objectTracker) Unregister(od OD) {
	ot.mu.Lock()
	defer ot.mu.Unlock()
	delete(ot.objects, od)
}

func (ot *objectTracker) Get(od OD) interface{} {
	ot.mu.RLock()
	defer ot.mu.RUnlock()
	return ot.objects[od]
}

var objects *objectTracker

func init() {
	objects = objectTrackerNew()
}

// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

#include <string.h>
#include <functional>

#include "vci.hpp"
#include "vci.h"

void
_vci_cpp_error_to_exception(vci_error *error)
{
	std::string app_tag(error->app_tag != NULL ? error->app_tag : "");
	std::string info(error->info != NULL ? error->info : "");
	std::string path(error->path != NULL ? error->path : "");
	vci_error_free(error);
	throw(vci::Exception(app_tag, info, path));
}

void
_vci_cpp_exception_to_error(const vci::Exception &e, vci_error *error)
{
	error->app_tag = strdup(e.app_tag().c_str());
	error->info = strdup(e.info().c_str());
	error->path = strdup(e.path().c_str());
}

int
_vci_cpp_call_config_set(void *obj, const char *in, vci_error *error)
{
	auto conf = (vci::Config *) obj;
	try {
		conf->set(std::string(in));
	} catch (const vci::Exception& e) {
		_vci_cpp_exception_to_error(e, error);
		return -1;
	}
	return 0;
}

int
_vci_cpp_call_config_check (void *obj, const char *in, vci_error *error)
{
	auto conf = (vci::Config *) obj;
	try {
		conf->check(std::string(in));
	} catch (const vci::Exception& e) {
		_vci_cpp_exception_to_error(e, error);
		return -1;
	}
	return 0;
}

void
_vci_cpp_call_config_get (void *obj, char **out)
{
	auto conf = (vci::Config *) obj;
	auto got = conf->get();
	*out = strdup(got.c_str());
}

void
_vci_cpp_call_config_free(void *obj)
{
	auto conf = (vci::Config *) obj;
	delete conf;
}

void
_vci_cpp_call_state_get (void *obj, char **out)
{
	auto state = (vci::State *) obj;
	auto got = state->get();
	*out = strdup(got.c_str());
}

void
_vci_cpp_call_state_free(void *obj)
{
	auto state = (vci::State *) obj;
	delete state;
}

void
_vci_cpp_call_subscriber (void *obj, const char *in)
{
	auto subscriber = (vci::Subscriber *) obj;
	subscriber->operator()(std::string(in));
}

void
_vci_cpp_call_subscriber_free(void *obj)
{
	auto subscriber = (vci::Subscriber *) obj;
	delete subscriber;
}

int
_vci_cpp_call_rpc(void *obj, const char *in, char **out, vci_error *error)
{
	auto method = (vci::Method *) obj;
	try {
		auto got = method->operator()(std::string(in));
		*out = strdup(got.c_str());
	} catch (const vci::Exception &e) {
		_vci_cpp_exception_to_error(e, error);
		return -1;
	}
	return 0;
}

void
_vci_cpp_call_rpc_free(void *obj)
{
	auto method = (vci::Method *) obj;
	delete method;
}

struct _vci::_CompImpl {
	vci_component* comp;
	~_CompImpl() {
		vci_component_free(comp);
	}
};

struct _vci::_ClientImpl {
	vci_client* client;
	_ClientImpl() {};
	_ClientImpl(vci_client *client) {
		this->client=client;
	}
	~_ClientImpl() {
		vci_client_free(client);
	}
};

vci::Exception::Exception(
	const std::string& app_tag,
	const std::string& info,
	const std::string& path)
{
	this->_app_tag = app_tag;
	this->_info = info;
	this->_path = path;
}

std::string
vci::Exception::app_tag() const
{
	return this->_app_tag;
}

std::string
vci::Exception::info() const
{
	return this->_info;
}

std::string
vci::Exception::path() const
{
	return this->_path;
}

vci::Model::Model(std::string name)
{
	this->_name = name;
}

vci::Model&
vci::Model::config(vci::Config* config)
{
	this->_config = config;
	return *this;
}

vci::Model&
vci::Model::state(vci::State* state)
{
	this->_state = state;
	return *this;
}

class methodFunc : public vci::Method {
public:
	methodFunc (vci::MethodFn fn) : _fn(fn) {}
	std::string operator()(const std::string &in) {
		return _fn(in);
	}
private:
	const vci::MethodFn _fn;
};


vci::Model&
vci::Model::rpc(const std::string& module,
				const std::string& name,
				vci::MethodFn fn)
{
	return this->rpc(module, name, new methodFunc(fn));
}

vci::Model&
vci::Model::rpc(const std::string& module,
				const std::string& name,
				Method* method)
{
       this->_methods[module][name] = method;
       return *this;
}


vci::Component::Component(std::string name)
{
	this->_impl = new _vci::_CompImpl();
	this->_impl->comp = vci_component_new(name.c_str());
}

vci::Component::~Component()
{
	delete _impl;
}

vci::Component&
vci::Component::model(Model &model)
{
	auto mod = vci_component_model(this->_impl->comp, model._name.c_str());
	if (model._config != NULL){
		vci_config_object config = {
			model._config,
			_vci_cpp_call_config_set,
			_vci_cpp_call_config_check,
			_vci_cpp_call_config_get,
			_vci_cpp_call_config_free,
		};
		vci_model_config(mod, &config);
	}

	if (model._state != NULL) {
		vci_state_object state = {
			model._state,
			_vci_cpp_call_state_get,
			_vci_cpp_call_state_free,
		};
		vci_model_state(mod, &state);
	}

	for (const auto &module_rpc : model._methods) {
		for (const auto &name_method : module_rpc.second) {
			vci_rpc_object rpc = {
				name_method.second,
				_vci_cpp_call_rpc,
				_vci_cpp_call_rpc_free,
			};
			vci_model_rpc(mod, module_rpc.first.c_str(),
						  name_method.first.c_str(), &rpc);
		}
	}
	free(mod);
	return *this;
}

vci::Component&
vci::Component::run()
{
	vci_error err;
	vci_error_init(&err);
	if (vci_component_run(this->_impl->comp, &err) != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	return *this;
}
vci::Component&
vci::Component::wait()
{
	vci_error err;
	vci_error_init(&err);
	if (vci_component_wait(this->_impl->comp, &err) != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	return *this;
}

vci::Component&
vci::Component::stop()
{
	vci_error err;
	vci_error_init(&err);
	if (vci_component_stop(this->_impl->comp, &err) != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	return *this;
}

class subscriberFunc : public vci::Subscriber {
public:
	subscriberFunc(vci::SubscriberFn fn) : _fn(fn) {}
	void operator()(const std::string &in) {
		return _fn(in);
	}
private:
	const vci::SubscriberFn _fn;
};

vci::Component&
vci::Component::subscribe(
	const std::string& module,
	const std::string& notification,
	Subscriber *subscriber)
{
	vci_error err;
	vci_error_init(&err);
	vci_subscriber_object _csub = {
		subscriber,
		_vci_cpp_call_subscriber,
		_vci_cpp_call_subscriber_free,
	};
	int ret = vci_component_subscribe(
		this->_impl->comp, module.c_str(), notification.c_str(), &_csub, &err);
	if (ret != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	return *this;
}

vci::Component&
vci::Component::subscribe(
	const std::string& module,
	const std::string& notification,
	SubscriberFn subscriber)
{
	this->subscribe(module, notification, new subscriberFunc(subscriber));
	return *this;
}

vci::Component&
vci::Component::unsubscribe(
	const std::string& module, const std::string& notification)
{
	vci_error err;
	vci_error_init(&err);
	int ret = vci_component_unsubscribe(
		this->_impl->comp,  module.c_str(), notification.c_str(), &err);
	if (ret != 0 ) {
		_vci_cpp_error_to_exception(&err);
	}
	return *this;
}

std::shared_ptr<vci::Client>
vci::Component::client() {
	vci_error err;
	vci_error_init(&err);
	vci_client *tmp;
	auto rc = vci_component_client(this->_impl->comp, &tmp, &err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	// Note: std::make_shared doesn't work here as we are using a private
	//       constructor from the friend class of vci::Client.
	std::shared_ptr<vci::Client> out(new vci::Client(new _vci::_ClientImpl(tmp)));
	return out;
}


vci::Client::Client()
{
	vci_error err;
	vci_error_init(&err);
	vci_client *tmp;
	auto rc = vci_client_dial(&tmp, &err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	this->_impl = new _vci::_ClientImpl(tmp);
}

vci::Client::Client(_vci::_ClientImpl* impl) {
	this->_impl = impl;
}

vci::Client::~Client()
{
	delete _impl;
}

struct _vci::_RPCCallImpl {
	vci_rpccall* call;
	~_RPCCallImpl() {
		vci_rpccall_free(call);
	}
};

std::shared_ptr<vci::RPCCall>
vci::Client::call(const std::string& module,
				  const std::string& name, const std::string& input)
{
	auto ccall = vci_client_call(
		this->_impl->client, module.c_str(), name.c_str(), input.c_str());
	auto impl = new _vci::_RPCCallImpl();
	impl->call = ccall;
	auto out = std::make_shared<vci::RPCCall>();
	out->_impl = impl;
	return out;
}

void
vci::Client::emit(
	const std::string& module,
	const std::string& name,
	const vci::EncodedInput& data)
{
	vci_error err;
	vci_error_init(&err);
	auto rc = vci_client_emit(
		this->_impl->client, module.c_str(), name.c_str(), data.c_str(),
		&err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
}

vci::EncodedOutput
vci::Client::config_by_model(const std::string& model) {
	vci_error err;
	vci_error_init(&err);
	char *out;
	auto rc = vci_client_store_config_by_model_into(
		this->_impl->client, model.c_str(), &out, &err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	return vci::EncodedOutput(out);
}

vci::EncodedOutput
vci::Client::state_by_model(const std::string& model) {
	vci_error err;
	vci_error_init(&err);
	char *out;
	auto rc = vci_client_store_state_by_model_into(
		this->_impl->client, model.c_str(), &out, &err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	return vci::EncodedOutput(out);
}

struct _vci::_SubscriptionImpl {
	vci_subscription* sub;
	~_SubscriptionImpl() {
		vci_subscription_free(sub);
	}
};

std::shared_ptr<vci::Subscription>
vci::Client::subscribe(
	const std::string& module,
	const std::string& name,
	vci::Subscriber* subscriber)
{
	vci_subscriber_object _csub = {
		subscriber,
		_vci_cpp_call_subscriber,
		_vci_cpp_call_subscriber_free,
	};
	auto csub = vci_client_subscribe(
		this->_impl->client, module.c_str(), name.c_str(), &_csub);
	auto impl = new _vci::_SubscriptionImpl();
	impl->sub = csub;
	auto out = std::make_shared<vci::Subscription>();
	out->_impl = impl;
	return out;
}

std::shared_ptr<vci::Subscription>
vci::Client::subscribe(
	const std::string& module,
	const std::string& name,
	vci::SubscriberFn subscriber)
{
	return this->subscribe(module, name, new subscriberFunc(subscriber));
}

vci::Subscription::Subscription() {}
vci::Subscription::~Subscription() {
	delete this->_impl;
}

void
vci::Subscription::run()
{
	vci_error err;
	vci_error_init(&err);
	auto rc = vci_subscription_run(this->_impl->sub, &err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
}

void
vci::Subscription::cancel()
{
	vci_error err;
	vci_error_init(&err);
	auto rc = vci_subscription_cancel(this->_impl->sub, &err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
}

void
vci::Subscription::coalesce()
{
	vci_subscription_coalesce(this->_impl->sub);
}

void
vci::Subscription::drop_after_limit(uint32_t limit)
{
	vci_subscription_drop_after_limit(this->_impl->sub, limit);
}

void
vci::Subscription::block_after_limit(uint32_t limit)
{
	vci_subscription_block_after_limit(this->_impl->sub, limit);
}

void
vci::Subscription::remove_limit()
{
	vci_subscription_remove_limit(this->_impl->sub);
}

vci::RPCCall::RPCCall() {}
vci::RPCCall::~RPCCall() {
	delete this->_impl;
}

std::string vci::RPCCall::output()
{
	char *out;
	vci_error err;
	vci_error_init(&err);
	auto rc = vci_rpccall_store_output_into(this->_impl->call, &out, &err);
	if (rc != 0) {
		_vci_cpp_error_to_exception(&err);
	}
	std::string output = out;
	free(out);
	return output;
}

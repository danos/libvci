// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "cgo-export/vci-interface.h"
#include "vci.h"

void
vci_error_free(vci_error *error)
{
	_vci_error_free(error);
}

void
vci_error_init(vci_error *error)
{
	_vci_error_init(error);
}

char *
vci_error_string(vci_error *error)
{
	return _vci_error_string(error);
}

struct vci_component {
	uint64_t cd;
};

struct vci_model {
	uint64_t md;
};

struct vci_client {
	uint64_t cd;
};

struct vci_rpccall {
	uint64_t rd;
};

struct vci_subscription {
	uint64_t sd;
};

vci_component *
vci_component_new(const char *name)
{
	vci_component* comp = malloc(sizeof(vci_component));
	if (comp == NULL) {
		return comp;
	}
	// we know go copies "name" so cast it to a compatible type
	comp->cd = _vci_component_new((char *)name);
	return comp;
}

void
vci_component_free(vci_component *comp)
{
	_vci_component_free(comp->cd);
	free(comp);
}

int
vci_component_run(vci_component *comp, vci_error *err)
{
	return _vci_component_run(comp->cd, err);
}

int
vci_component_wait(vci_component *comp, vci_error *err)
{
	return _vci_component_wait(comp->cd, err);
}

int
vci_component_stop(vci_component *comp, vci_error *err)
{
	return _vci_component_stop(comp->cd, err);
}

int
vci_component_subscribe(vci_component* comp,
						const char *module_name,
						const char *notification_name,
						const vci_subscriber_object* subscriber,
						vci_error *err)
{
	return _vci_component_subscribe(comp->cd, (char *) module_name,
									(char *) notification_name,
									(vci_subscriber_object *) subscriber,
									err);
}

int
vci_component_unsubscribe(vci_component* comp,
						  const char *module_name,
						  const char *notification_name,
						  vci_error *err)
{
	return _vci_component_unsubscribe(comp->cd, (char *)module_name,
									  (char *) notification_name, err);
}

vci_model *
vci_component_model(vci_component *comp,
						const char *name)
{
	vci_model* mod = malloc(sizeof(vci_model));
	if (mod == NULL) {
		return mod;
	}
	
	mod->md = _vci_component_model(comp->cd, (char *) name);
	return mod;
}

void
vci_model_config(vci_model *model, const vci_config_object* config)
{
	_vci_model_config(model->md, (vci_config_object*) config);
}

void
vci_model_state(vci_model *model, const vci_state_object* config)
{
	_vci_model_state(model->md, (vci_state_object*) config);
}

void
vci_model_rpc(vci_model *model, const char *module_name,
			  const char * rpc_name, const vci_rpc_object* rpc)
{
	_vci_model_rpc(model->md, (char *)module_name, (char *)rpc_name,
				   (vci_rpc_object*) rpc);
}

void
vci_model_free(vci_model *model)
{
	_vci_model_free(model->md);
	free(model);
}

int
vci_client_dial(vci_client **client, vci_error *error)
{
	*client = malloc(sizeof(vci_client));
	if (*client == NULL) {
		error->app_tag = strdup("vci-internal");
		error->path = NULL;
		error->info = strdup("failed to allocate client");
		return -1;
	}
	return _vci_client_dial(&(*client)->cd, error);
}

void
vci_client_free(vci_client *client)
{
	_vci_client_free(client->cd);
	free(client);
}

int
vci_client_emit(vci_client *client,
				const char *module, const char *name,
				const char *data, vci_error *err)
{
	return _vci_client_emit(
		client->cd, (char*)module, (char*)name, (char*)data, err);
}

int
vci_client_store_config_by_model_into(
	vci_client *client , const char *model, char **output, vci_error *err)
{
	return _vci_client_store_config_by_model_into(
		client->cd, (char*)model, output, err);
}

int
vci_client_store_state_by_model_into(
	vci_client *client ,const char *model, char **output, vci_error *err)
{
	return _vci_client_store_state_by_model_into(
		client->cd, (char*)model, output, err);
}


vci_rpccall *
vci_client_call(vci_client *client,
				const char *module, const char *name,
				const char *input)
{
	vci_rpccall *out = malloc(sizeof(vci_rpccall));
	if (out == NULL) {
		return NULL;
	}
    out->rd = _vci_client_call(
		client->cd, (char*)module, (char*)name, (char*)input);
	return out;
}

void
vci_rpccall_free(vci_rpccall *call)
{
	_vci_rpccall_free(call->rd);
	free(call);
}

int
vci_rpccall_store_output_into(vci_rpccall *call,
							  char **output, vci_error *err)
{
	return _vci_rpccall_store_output_into(call->rd, output, err);
}

vci_subscription *
vci_client_subscribe(
	vci_client *client, const char *module, const char *name,
	const vci_subscriber_object* subscriber)
{
	vci_subscription *out = malloc(sizeof(vci_subscription));
	if (out == NULL) {
		return NULL;
	}
	out->sd = _vci_client_subscribe(
		client->cd, (char*)module, (char*)name,
		(vci_subscriber_object *)subscriber);
	return out;
}

void
vci_subscription_free(vci_subscription *sub)
{
	_vci_subscription_free(sub->sd);
	free(sub);
}

int
vci_subscription_run(vci_subscription *sub, vci_error *err)
{
	return _vci_subscription_run(sub->sd, err);
}

int
vci_subscription_cancel(vci_subscription *sub, vci_error *err)
{
	return _vci_subscription_cancel(sub->sd, err);
}

void
vci_subscription_coalesce(vci_subscription *sub)
{
	_vci_subscription_coalesce(sub->sd);
}

void
vci_subscription_drop_after_limit(vci_subscription *sub, uint32_t limit)
{
	_vci_subscription_drop_after_limit(sub->sd, limit);
}

void
vci_subscription_block_after_limit(vci_subscription *sub, uint32_t limit)
{
	_vci_subscription_drop_after_limit(sub->sd, limit);
}

void
vci_subscription_remove_limit(vci_subscription *sub)
{
	_vci_subscription_remove_limit(sub->sd);
}

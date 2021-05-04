// Copyright (c) 2018-2021, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

#ifndef __VCI_H__
#define __VCI_H__
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct vci_component vci_component;
typedef struct vci_model vci_model;
typedef struct vci_client vci_client;
typedef struct vci_rpccall vci_rpccall;
typedef struct vci_subscription vci_subscription;

typedef struct {
	char *app_tag;
	char *path;
	char *info;
} vci_error;

void vci_error_init(vci_error *error);
char *vci_error_string(vci_error *error);
void vci_error_free(vci_error *error);

typedef struct {
	void *obj;
	int (*set)(void *obj, const char *in, vci_error *error);
	int (*check) (void *obj, const char *in, vci_error *error);
	void (*get) (void *obj, char **out);
	void (*free)(void *obj);
} vci_config_object;

typedef struct {
	void *obj;
	void (*get) (void *obj, char **out);
	void (*free)(void *obj);
} vci_state_object;

typedef struct {
	void *obj;
	int (*call) (void *obj, const char *in, char **out, vci_error *error);
	void (*free)(void *obj);
} vci_rpc_object;

typedef struct {
	void *obj;
	int (*call) (void *obj, const char *meta, const char *in, char **out, vci_error *error);
	void (*free)(void *obj);
} vci_rpc_meta_object;

typedef struct {
	void *obj;
	void (*subscriber)(void *obj, const char *in);
	void (*free)(void *obj);
} vci_subscriber_object;

vci_component * vci_component_new(const char* name);
void vci_component_free(vci_component*);
int vci_component_run(vci_component* comp, vci_error *error);
int vci_component_wait(vci_component* comp, vci_error *error);
int vci_component_stop(vci_component* comp, vci_error *error);
int vci_component_client(vci_component *comp,
						 vci_client **client,
						 vci_error *error);
int vci_component_subscribe(vci_component* comp,
							const char *module_name,
							const char *notification_name,
							const vci_subscriber_object* subscriber,
							vci_error *error);
int vci_component_unsubscribe(vci_component* comp,
							  const char *module_name,
							  const char *notification_name,
							  vci_error *error);

vci_model * vci_component_model(vci_component *comp,
								const char *name);
void vci_model_config(vci_model *model, const vci_config_object* config);
void vci_model_state(vci_model *model, const vci_state_object* state);
void vci_model_rpc(vci_model *model, const char *module_name,
				   const char *rpc_name, const vci_rpc_object* rpc);
void vci_model_rpc_meta(vci_model *model, const char *module_name,
				   const char *rpc_name, const vci_rpc_meta_object* rpc);
void vci_model_free(vci_model *model);

int vci_client_dial(vci_client **client, vci_error *error);
void vci_client_free(vci_client *client);
int vci_client_emit(vci_client *client,
					const char *module, const char *name,
					const char *data, vci_error *err);
int vci_client_store_config_by_model_into(
	vci_client *client ,const char *model, char **output, vci_error *err);
int vci_client_store_state_by_model_into(
	vci_client *client ,const char *model, char **output, vci_error *err);

vci_rpccall *vci_client_call(vci_client *client,
							 const char *module, const char *name,
							 const char *input);
void vci_rpccall_free(vci_rpccall *call);
int vci_rpccall_store_output_into(vci_rpccall *call,
								  char **output, vci_error *err);

vci_subscription *vci_client_subscribe(
	vci_client *client, const char *module, const char *name,
	const vci_subscriber_object* subscriber);
void vci_subscription_free(vci_subscription *sub);
int vci_subscription_run(vci_subscription *sub, vci_error *err);
int vci_subscription_cancel(vci_subscription *sub, vci_error *err);
void vci_subscription_coalesce(vci_subscription *sub);
void vci_subscription_drop_after_limit(vci_subscription *sub, uint32_t limit);
void vci_subscription_block_after_limit(vci_subscription *sub, uint32_t limit);
void vci_subscription_remove_limit(vci_subscription *sub);

#ifdef __cplusplus
}
#endif

#endif

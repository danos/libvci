// Copyright (c) 2018-2019, AT&T Intellectual Property.
// All rights reserved.
//
// SPDX-License-Identifier: LGPL-2.1-only

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <vci.h>

void
toast_done_sub(void *obj, const char *in)
{
	printf("toast-done %s\n", in);
}

typedef struct {
	pthread_mutex_t mu;
	char *config;
} cexample_config;

cexample_config *
cexample_config_new(char *config)
{
	cexample_config *conf = malloc(sizeof(cexample_config));
	if (conf == NULL) {
		return conf;
	}
	if (pthread_mutex_init(&conf->mu, NULL) != 0) {
		free(conf);
		return NULL;
	}
	conf->config = strdup(config);
	return conf;
}

void
cexample_config_free(cexample_config *config)
{
	free(config->config);
	free(config);
}

int
cexample_config_set(void *obj, const char *in, vci_error *error)
{
	printf("set %s\n", in);
	cexample_config *conf = (cexample_config *)obj;
	pthread_mutex_lock(&conf->mu);
	char *old = conf->config;
	conf->config = strdup(in);
	free(old);
	pthread_mutex_unlock(&conf->mu);
	return 0;
}

int
cexample_config_check(void *obj, const char *in, vci_error *error)
{
	printf("check %s\n", in);
	return 0;
}

void
cexample_config_get(void *obj, char **out)
{
	cexample_config *conf = (cexample_config *)obj;
	pthread_mutex_lock(&conf->mu);
	*out = strdup(conf->config);
	pthread_mutex_unlock(&conf->mu);
}

void
cexample_state_get(void *obj, char **out)
{
	*out = strdup("{\"state\":\"foobar\"}");
}

int
cexample_rpc1(void *obj, const char *in, char **out, vci_error *error)
{
	*out = strdup(in);
	return 0;
}

int
cexample_rpc_fail(void *obj, const char *in, char **out, vci_error *err)
{
	err->app_tag = strdup("rpc-failure");
	err->info = strdup("This RPC always fails");
	return -1;
}

int
main()
{
	int rc = 0;
	vci_error err;
	vci_error_init(&err);
	vci_component *comp = vci_component_new("net.vyatta.vci.cexample");
	if (comp == NULL) {
		fprintf(stderr, "alloc of component failed\n");
		return 1;
	}

	vci_model *model = vci_component_model(comp, "net.vyatta.vci.cexample.v1");
	if (model == NULL) {
		fprintf(stderr, "alloc of model failed\n");
		rc = 1;
		goto err2;
	}

	cexample_config *conf = cexample_config_new("{}");
	if (conf == NULL) {
		fprintf(stderr, "alloc of cexample_config failed\n");
		rc = 1;
		goto err1;
	}
	vci_config_object config = {
		.obj = conf,
		.set = cexample_config_set,
		.check = cexample_config_check,
		.get = cexample_config_get,
	};
	vci_model_config(model, &config);

	vci_state_object state = {
		.obj = NULL,
		.get = cexample_state_get,
	};
	vci_model_state(model, &state);

	vci_rpc_object rpc1 = {
		.obj = NULL,
		.call = cexample_rpc1,
	};
	vci_model_rpc(model, "cexample", "rpc1", &rpc1);
	vci_model_rpc(model, "cexample", "rpc2", &rpc1);

	vci_rpc_object rpc_fail = {
		.obj = NULL,
		.call = cexample_rpc_fail,
	};
	vci_model_rpc(model, "cexample", "rpc-fail", &rpc_fail);

	vci_subscriber_object sub = {
		.obj = NULL,
		.subscriber = toast_done_sub,
	};
	rc = vci_component_subscribe(comp, "toaster", "toast-done", &sub, &err);
	if (rc != 0) {
		fprintf(stderr, "subscribe returned with: %d\n", rc);
		goto err0;
	}

	rc = vci_component_run(comp, &err);
	if (rc != 0) {
		fprintf(stderr, "run returned with: %d\n", rc);
		goto err0;
	}
	rc = vci_component_wait(comp, &err);
	if (rc != 0) {
		fprintf(stderr, "wait returned with: %d\n", rc);
		goto err0;
	}
err0:
	cexample_config_free(conf);
err1:
	vci_model_free(model);
err2:
	vci_component_free(comp);
	return rc;
}

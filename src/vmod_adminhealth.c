#include "vas.h"
#include "vrt.h"
#include "vcc_if.h"
#include <lib/libvgz/vgz.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>

#include <bin/varnishd/cache/cache.h>
#include <bin/varnishd/cache/cache_director.h>
#include <bin/varnishd/cache/cache_backend.h>
#include <bin/varnishd/cache/cache_vcl.h>
#include <vcl.h>
#include <vrt_obj.h>


#define LOG_E(...) fprintf(stderr, __VA_ARGS__);


#define ALLOC_BUFFER(ptr, l) do {		     \
		(ptr) = calloc(sizeof(*(ptr)) * l, 1);	\
		assert((ptr) != NULL);		     \
	} while (0)


#define FREE_BUFFER(x) do {				\
		if (x)					\
			free(x);			\
		(x) = NULL;				\
	} while (0)

extern const char * vbe_str2adminhealth(const char *wstate);

/**
 * Free our private config structure.
 */
typedef struct vmodConfig {
	const struct vcl* conf;
} config_t;

static void
free_config(config_t *priv_conf)
{
	FREE_BUFFER(priv_conf);
}

static config_t *
make_config(const struct vcl *conf)
{
	config_t *priv_conf;

	AN(conf);
	ALLOC_BUFFER(priv_conf, sizeof(config_t));
	priv_conf->conf = conf;
	return priv_conf;
}

/**
 * This init function is called whenever a VCL program imports this VMOD.
 */
int
init_function(VRT_CTX, struct vmod_priv *priv)
{
	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);
	CHECK_OBJ_NOTNULL(ctx->vcl, VCL_MAGIC);

	if (priv->priv == NULL) {
		priv->priv = make_config(ctx->vcl);
		priv->free = (vmod_priv_free_f *)free_config;
	}

	return (0);
}

static int
event_load(VRT_CTX, struct vmod_priv *priv)
{
	return (0);
}

static int
event_warm(VRT_CTX, const struct vmod_priv *priv)
{
	return (0);
}

static int
event_cold(VRT_CTX, const struct vmod_priv *priv)
{
	return (0);
}

int __match_proto__(vmod_event_f)
event_function(VRT_CTX, struct vmod_priv *priv, enum vcl_event_e e)
{
	switch (e) {
	case VCL_EVENT_LOAD: return (event_load(ctx, priv));
	case VCL_EVENT_WARM: return (event_warm(ctx, priv));
	case VCL_EVENT_COLD: return (event_cold(ctx, priv));
	default: return (0);
	}
}

struct backend*
_getbackend(const struct vcl *conf, const char *backend)
{
	int i;
	struct backend *be;

	AN(conf);
	AN(backend);

	VTAILQ_FOREACH(be, &conf->backend_list, vcl_list)
		if (strcmp(backend, be->vcl_name) == 0) {
			return be;
		}

	return NULL;
}

VCL_VOID __match_proto__()
vmod_set(VRT_CTX, struct vmod_priv *priv, const char *backend, const char* state)
{
	const char *state_code;
	const struct vcl *conf;
	config_t *priv_conf;

	CHECK_OBJ_NOTNULL(ctx, VRT_CTX_MAGIC);

	if (state == NULL) {
		return;
	}

	state_code = vbe_str2adminhealth(state);
	if (! state_code) {
		syslog(LOG_NOTICE, "ADMINHEALTH Invalid state <%s>", state);
		return;
	}

	if (ctx == NULL) {
		syslog(LOG_NOTICE, "ADMINHEALTH Context is NULL");
		return;
	}
	conf = ctx->vcl;

	struct backend* back = _getbackend(conf, backend);
	if (back == NULL) {
		syslog(LOG_NOTICE, "ADMINHEALTH Unknown backend <%s>", backend);
		return;
	}

	back->admin_health = state_code;
	syslog(LOG_NOTICE, "ADMINHEALTH state of <%s> has been set to <%s>", backend, state);
}

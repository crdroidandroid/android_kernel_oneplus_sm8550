#ifndef __QCOM_RPROC_H__
#define __QCOM_RPROC_H__

struct notifier_block;
struct rproc;

/**
 * enum qcom_ssr_notify_type - Startup/Shutdown events related to a remoteproc
 * processor.
 *
 * @QCOM_SSR_BEFORE_POWERUP:	Remoteproc about to start (prepare stage)
 * @QCOM_SSR_AFTER_POWERUP:	Remoteproc is running (start stage)
 * @QCOM_SSR_BEFORE_SHUTDOWN:	Remoteproc crashed or shutting down (stop stage)
 * @QCOM_SSR_AFTER_SHUTDOWN:	Remoteproc is down (unprepare stage)
 */
enum qcom_ssr_notify_type {
	QCOM_SSR_BEFORE_POWERUP,
	QCOM_SSR_AFTER_POWERUP,
	QCOM_SSR_BEFORE_SHUTDOWN,
	QCOM_SSR_AFTER_SHUTDOWN,
};

struct qcom_ssr_notify_data {
	const char *name;
	bool crashed;
};

#if IS_ENABLED(CONFIG_QCOM_RPROC_COMMON)

void *qcom_register_ssr_notifier(const char *name, struct notifier_block *nb);
void *qcom_register_early_ssr_notifier(const char *name, struct notifier_block *nb);
int qcom_unregister_early_ssr_notifier(void *notify, struct notifier_block *nb);
int qcom_unregister_ssr_notifier(void *notify, struct notifier_block *nb);
int qcom_rproc_set_dtb_firmware(struct rproc *rproc, const char *dtb_fw_name);

#else

static inline void *qcom_register_ssr_notifier(const char *name,
					       struct notifier_block *nb)
{
	return NULL;
}

static inline void *qcom_register_early_ssr_notifier(const char *name, struct notifier_block *nb)
{
	return NULL;
}

static inline int qcom_unregister_early_ssr_notifier(void *notify,
					       struct notifier_block *nb)
{
	return 0;
}

static inline int qcom_unregister_ssr_notifier(void *notify,
					       struct notifier_block *nb)
{
	return 0;
}
static inline int qcom_rproc_set_dtb_firmware(struct rproc *rproc, const char *dtb_fw_name)
{
	return -EINVAL;
}


#endif

#if IS_ENABLED(CONFIG_QCOM_Q6V5_PAS) && IS_ENABLED(CONFIG_QCOM_DS_SKIP_Q6_STOP)
void adsp_set_ops_stop(struct rproc *rproc, bool suspend);
#else
static inline void adsp_set_ops_stop(struct rproc *rproc, bool suspend) { }
#endif

#endif

#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x64ec254f, "module_layout" },
	{ 0x1b618986, "blk_init_queue" },
	{ 0x6d25fab6, "blk_cleanup_queue" },
	{ 0x8728140, "malloc_sizes" },
	{ 0xbed60566, "sub_preempt_count" },
	{ 0xc633495b, "schedule_work" },
	{ 0x105e2727, "__tracepoint_kmalloc" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xea147363, "printk" },
	{ 0xf397b9aa, "__tasklet_schedule" },
	{ 0x4c6ff041, "add_preempt_count" },
	{ 0xa5808bbf, "tasklet_init" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0x43b0c9c3, "preempt_schedule" },
	{ 0x90ab2af2, "blk_insert_request" },
	{ 0xdc74cc24, "kmem_cache_alloc" },
	{ 0x9c0d1863, "blk_fetch_request" },
	{ 0x37a0cba, "kfree" },
	{ 0x4992ff0a, "blk_end_request_all" },
	{ 0x67566da0, "blk_requeue_request" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x8c4f575f, "blk_get_request" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "9F948CAAD4C47514572181D");

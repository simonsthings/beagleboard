/*
 * omap iommu: tlb and pagetable primitives
 *
 * Copyright (C) 2008-2009 Nokia Corporation
 *
 * Written by Hiroshi DOYU <Hiroshi.DOYU@nokia.com>,
 *		Paul Mundt and Toshihiro Kobayashi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/cacheflush.h>

#include <mach/clock.h>
#include <mach/iommu.h>

#include "iopgtable.h"

/* accommodate the difference between omap1 and omap2/3 */
static const struct iommu_functions *arch_iommu;

static struct platform_driver omap_iommu_driver;
static struct kmem_cache *iopte_cachep;

/**
 * install_iommu_arch() - Install archtecure specific iommu functions
 * @ops:	a pointer to architecture specific iommu functions
 *
 * There are several kind of iommu algorithm(tlb, pagetable) among
 * omap series. This interface installs such an iommu algorighm.
 **/
int install_iommu_arch(const struct iommu_functions *ops)
{
	if (arch_iommu)
		return -EBUSY;

	arch_iommu = ops;
	return 0;
}
EXPORT_SYMBOL_GPL(install_iommu_arch);

/**
 * uninstall_iommu_arch() - Uninstall archtecure specific iommu functions
 * @ops:	a pointer to architecture specific iommu functions
 *
 * This interface uninstalls the iommu algorighm installed previously.
 **/
void uninstall_iommu_arch(const struct iommu_functions *ops)
{
	if (arch_iommu != ops)
		pr_err("%s: not your arch\n", __func__);

	arch_iommu = NULL;
}
EXPORT_SYMBOL_GPL(uninstall_iommu_arch);

/**
 * iommu_save_ctx() - Save registers for pm off-mode support
 * @obj:	target iommu
 **/
void iommu_save_ctx(struct iommu *obj)
{
	arch_iommu->save_ctx(obj);
}
EXPORT_SYMBOL_GPL(iommu_save_ctx);

/**
 * iommu_restore_ctx() - Restore registers for pm off-mode support
 * @obj:	target iommu
 **/
void iommu_restore_ctx(struct iommu *obj)
{
	arch_iommu->restore_ctx(obj);
}
EXPORT_SYMBOL_GPL(iommu_restore_ctx);

/**
 * iommu_arch_version() - Return running iommu arch version
 **/
u32 iommu_arch_version(void)
{
	return arch_iommu->version;
}
EXPORT_SYMBOL_GPL(iommu_arch_version);

static int iommu_enable(struct iommu *obj)
{
	int err;

	if (!obj)
		return -EINVAL;

	clk_enable(obj->clk);

	err = arch_iommu->enable(obj);

	clk_disable(obj->clk);
	return err;
}

static void iommu_disable(struct iommu *obj)
{
	if (!obj)
		return;

	clk_enable(obj->clk);

	arch_iommu->disable(obj);

	clk_disable(obj->clk);
}

#ifdef DEBUG
static ssize_t iommu_dump_ctx(struct iommu *obj, char *buf)
{
	if (!obj || !buf)
		return -EINVAL;

	return arch_iommu->dump_ctx(obj, buf);
}
#endif

/*
 *	TLB operations
 */
static inline void iotlb_cr_to_e(struct cr_regs *cr, struct iotlb_entry *e)
{
	BUG_ON(!cr || !e);

	arch_iommu->cr_to_e(cr, e);
}

static inline int iotlb_cr_valid(struct cr_regs *cr)
{
	if (!cr)
		return -EINVAL;

	return arch_iommu->cr_valid(cr);
}

static inline struct cr_regs *iotlb_alloc_cr(struct iommu *obj,
					     struct iotlb_entry *e)
{
	if (!e)
		return NULL;

	return arch_iommu->alloc_cr(obj, e);
}

static inline u32 iotlb_cr_to_virt(struct cr_regs *cr)
{
	return arch_iommu->cr_to_virt(cr);
}

static u32 get_iopte_attr(struct iotlb_entry *e)
{
	return arch_iommu->get_pte_attr(e);
}

static u32 iommu_report_fault(struct iommu *obj, u32 *da)
{
	return arch_iommu->fault_isr(obj, da);
}

static void iotlb_lock_get(struct iommu *obj, struct iotlb_lock *l)
{
	u32 val;

	val = iommu_read_reg(obj, MMU_LOCK);

	l->base = MMU_LOCK_BASE(val);
	l->vict = MMU_LOCK_VICT(val);

	BUG_ON(l->base != 0); /* Currently no preservation is used */
}

static void iotlb_lock_set(struct iommu *obj, struct iotlb_lock *l)
{
	u32 val;

	BUG_ON(l->base != 0); /* Currently no preservation is used */

	val = (l->base << MMU_LOCK_BASE_SHIFT);
	val |= (l->vict << MMU_LOCK_VICT_SHIFT);

	iommu_write_reg(obj, val, MMU_LOCK);
}

static void iotlb_read_cr(struct iommu *obj, struct cr_regs *cr)
{
	arch_iommu->tlb_read_cr(obj, cr);
}

static void iotlb_load_cr(struct iommu *obj, struct cr_regs *cr)
{
	arch_iommu->tlb_load_cr(obj, cr);

	iommu_write_reg(obj, 1, MMU_FLUSH_ENTRY);
	iommu_write_reg(obj, 1, MMU_LD_TLB);
}

/**
 * iotlb_dump_cr() - Dump an iommu tlb entry into buf
 * @obj:	target iommu
 * @cr:		contents of cam and ram register
 * @buf:	output buffer
 **/
ssize_t iotlb_dump_cr(struct iommu *obj, struct cr_regs *cr, char *buf)
{
	BUG_ON(!cr || !buf);

	return arch_iommu->dump_cr(obj, cr, buf);
}
EXPORT_SYMBOL_GPL(iotlb_dump_cr);

/**
 * load_iotlb_entry() - Set an iommu tlb entry
 * @obj:	target iommu
 * @e:		an iommu tlb entry info
 **/
int load_iotlb_entry(struct iommu *obj, struct iotlb_entry *e)
{
	int i;
	int err = 0;
	struct iotlb_lock l;
	struct cr_regs *cr;

	if (!obj || !obj->nr_tlb_entries || !e)
		return -EINVAL;

	clk_enable(obj->clk);

	for (i = 0; i < obj->nr_tlb_entries; i++) {
		struct cr_regs tmp;

		iotlb_lock_get(obj, &l);
		l.vict = i;
		iotlb_lock_set(obj, &l);
		iotlb_read_cr(obj, &tmp);
		if (!iotlb_cr_valid(&tmp))
			break;
	}

	if (i == obj->nr_tlb_entries) {
		dev_dbg(obj->dev, "%s: full: no entry\n", __func__);
		err = -EBUSY;
		goto out;
	}

	cr = iotlb_alloc_cr(obj, e);
	if (IS_ERR(cr)) {
		clk_disable(obj->clk);
		return PTR_ERR(cr);
	}

	iotlb_load_cr(obj, cr);
	kfree(cr);

	/* increment victim for next tlb load */
	if (++l.vict == obj->nr_tlb_entries)
		l.vict = 0;
	iotlb_lock_set(obj, &l);
out:
	clk_disable(obj->clk);
	return err;
}
EXPORT_SYMBOL_GPL(load_iotlb_entry);

/**
 * flush_iotlb_page() - Clear an iommu tlb entry
 * @obj:	target iommu
 * @da:		iommu device virtual address
 *
 * Clear an iommu tlb entry which includes 'da' address.
 **/
void flush_iotlb_page(struct iommu *obj, u32 da)
{
	struct iotlb_lock l;
	int i;

	clk_enable(obj->clk);

	for (i = 0; i < obj->nr_tlb_entries; i++) {
		struct cr_regs cr;
		u32 start;
		size_t bytes;

		iotlb_lock_get(obj, &l);
		l.vict = i;
		iotlb_lock_set(obj, &l);
		iotlb_read_cr(obj, &cr);
		if (!iotlb_cr_valid(&cr))
			continue;

		start = iotlb_cr_to_virt(&cr);
		bytes = iopgsz_to_bytes(cr.cam & 3);

		if ((start <= da) && (da < start + bytes)) {
			dev_dbg(obj->dev, "%s: %08x<=%08x(%x)\n",
				__func__, start, da, bytes);

			iommu_write_reg(obj, 1, MMU_FLUSH_ENTRY);
		}
	}
	clk_disable(obj->clk);

	if (i == obj->nr_tlb_entries)
		dev_dbg(obj->dev, "%s: no page for %08x\n", __func__, da);
}
EXPORT_SYMBOL_GPL(flush_iotlb_page);

/**
 * flush_iotlb_range() - Clear an iommu tlb entries
 * @obj:	target iommu
 * @start:	iommu device virtual address(start)
 * @end:	iommu device virtual address(end)
 *
 * Clear an iommu tlb entry which includes 'da' address.
 **/
void flush_iotlb_range(struct iommu *obj, u32 start, u32 end)
{
	u32 da = start;

	while (da < end) {
		flush_iotlb_page(obj, da);
		/* FIXME: Optimize for multiple page size */
		da += IOPTE_SIZE;
	}
}
EXPORT_SYMBOL_GPL(flush_iotlb_range);

/**
 * flush_iotlb_all() - Clear all iommu tlb entries
 * @obj:	target iommu
 **/
void flush_iotlb_all(struct iommu *obj)
{
	struct iotlb_lock l;

	clk_enable(obj->clk);

	l.base = 0;
	l.vict = 0;
	iotlb_lock_set(obj, &l);

	iommu_write_reg(obj, 1, MMU_GFLUSH);

	clk_disable(obj->clk);
}
EXPORT_SYMBOL_GPL(flush_iotlb_all);

/*
 *	H/W pagetable operations
 */
static void flush_iopgd_range(u32 *first, u32 *last)
{
	/* FIXME: L2 cache should be taken care of if it exists */
	do {
		asm("mcr	p15, 0, %0, c7, c10, 1 @ flush_pgd"
		    : : "r" (first));
		first += L1_CACHE_BYTES / sizeof(*first);
	} while (first <= last);
}

static void flush_iopte_range(u32 *first, u32 *last)
{
	/* FIXME: L2 cache should be taken care of if it exists */
	do {
		asm("mcr	p15, 0, %0, c7, c10, 1 @ flush_pte"
		    : : "r" (first));
		first += L1_CACHE_BYTES / sizeof(*first);
	} while (first <= last);
}

static void iopte_free(u32 *iopte)
{
	/* Note: freed iopte's must be clean ready for re-use */
	kmem_cache_free(iopte_cachep, iopte);
}

static u32 *iopte_alloc(struct iommu *obj, u32 *iopgd, u32 da)
{
	u32 *iopte;

	/* a table has already existed */
	if (*iopgd)
		goto pte_ready;

	/*
	 * do the allocation outside the page table lock
	 */
	spin_unlock(&obj->page_table_lock);
	iopte = kmem_cache_zalloc(iopte_cachep, GFP_KERNEL);
	spin_lock(&obj->page_table_lock);

	if (!*iopgd) {
		if (!iopte)
			return ERR_PTR(-ENOMEM);

		*iopgd = virt_to_phys(iopte) | IOPGD_TABLE;
		flush_iopgd_range(iopgd, iopgd);

		dev_vdbg(obj->dev, "%s: a new pte:%p\n", __func__, iopte);
	} else {
		/* We raced, free the reduniovant table */
		iopte_free(iopte);
	}

pte_ready:
	iopte = iopte_offset(iopgd, da);

	dev_vdbg(obj->dev,
		 "%s: da:%08x pgd:%p *pgd:%08x pte:%p *pte:%08x\n",
		 __func__, da, iopgd, *iopgd, iopte, *iopte);

	return iopte;
}

static int iopgd_alloc_section(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);

	*iopgd = (pa & IOSECTION_MASK) | prot | IOPGD_SECTION;
	flush_iopgd_range(iopgd, iopgd);
	return 0;
}

static int iopgd_alloc_super(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);
	int i;

	for (i = 0; i < 16; i++)
		*(iopgd + i) = (pa & IOSUPER_MASK) | prot | IOPGD_SUPER;
	flush_iopgd_range(iopgd, iopgd + 15);
	return 0;
}

static int iopte_alloc_page(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);
	u32 *iopte = iopte_alloc(obj, iopgd, da);

	if (IS_ERR(iopte))
		return PTR_ERR(iopte);

	*iopte = (pa & IOPAGE_MASK) | prot | IOPTE_SMALL;
	flush_iopte_range(iopte, iopte);

	dev_vdbg(obj->dev, "%s: da:%08x pa:%08x pte:%p *pte:%08x\n",
		 __func__, da, pa, iopte, *iopte);

	return 0;
}

static int iopte_alloc_large(struct iommu *obj, u32 da, u32 pa, u32 prot)
{
	u32 *iopgd = iopgd_offset(obj, da);
	u32 *iopte = iopte_alloc(obj, iopgd, da);
	int i;

	if (IS_ERR(iopte))
		return PTR_ERR(iopte);

	for (i = 0; i < 16; i++)
		*(iopte + i) = (pa & IOLARGE_MASK) | prot | IOPTE_LARGE;
	flush_iopte_range(iopte, iopte + 15);
	return 0;
}

static int iopgtable_store_entry_core(struct iommu *obj, struct iotlb_entry *e)
{
	int (*fn)(struct iommu *, u32, u32, u32);
	u32 prot;
	int err;

	if (!obj || !e)
		return -EINVAL;

	switch (e->pgsz) {
	case MMU_CAM_PGSZ_16M:
		fn = iopgd_alloc_super;
		break;
	case MMU_CAM_PGSZ_1M:
		fn = iopgd_alloc_section;
		break;
	case MMU_CAM_PGSZ_64K:
		fn = iopte_alloc_large;
		break;
	case MMU_CAM_PGSZ_4K:
		fn = iopte_alloc_page;
		break;
	default:
		fn = NULL;
		BUG();
		break;
	}

	prot = get_iopte_attr(e);

	spin_lock(&obj->page_table_lock);
	err = fn(obj, e->da, e->pa, prot);
	spin_unlock(&obj->page_table_lock);

	return err;
}

#ifdef DEBUG
static void dump_tlb_entries(struct iommu *obj)
{
	int i;
	struct iotlb_lock l;

	clk_enable(obj->clk);

	pr_info("%8s %8s\n", "cam:", "ram:");
	pr_info("-----------------------------------------\n");

	for (i = 0; i < obj->nr_tlb_entries; i++) {
		struct cr_regs cr;
		static char buf[4096];

		iotlb_lock_get(obj, &l);
		l.vict = i;
		iotlb_lock_set(obj, &l);
		iotlb_read_cr(obj, &cr);
		if (!iotlb_cr_valid(&cr))
			continue;

		memset(buf, 0, 4096);
		iotlb_dump_cr(obj, &cr, buf);
		pr_err("%s", buf);
	}

	clk_disable(obj->clk);
}
#else
static inline void dump_tlb_entries(struct iommu *obj) {}
#endif

/**
 * iopgtable_store_entry() - Make an iommu pte entry
 * @obj:	target iommu
 * @e:		an iommu tlb entry info
 **/
int iopgtable_store_entry(struct iommu *obj, struct iotlb_entry *e)
{
	int err;

	flush_iotlb_page(obj, e->da);
	err = iopgtable_store_entry_core(obj, e);
#ifdef USE_IOTLB
	if (!err)
		load_iotlb_entry(obj, e);
#endif
	return err;
}
EXPORT_SYMBOL_GPL(iopgtable_store_entry);

/**
 * iopgtable_lookup_entry() - Lookup an iommu pte entry
 * @obj:	target iommu
 * @da:		iommu device virtual address
 * @ppgd:	iommu pgd entry pointer to be returned
 * @ppte:	iommu pte entry pointer to be returned
 **/
void iopgtable_lookup_entry(struct iommu *obj, u32 da, u32 **ppgd, u32 **ppte)
{
	u32 *iopgd, *iopte = NULL;

	iopgd = iopgd_offset(obj, da);
	if (!*iopgd)
		goto out;

	if (*iopgd & IOPGD_TABLE)
		iopte = iopte_offset(iopgd, da);
out:
	*ppgd = iopgd;
	*ppte = iopte;
}
EXPORT_SYMBOL_GPL(iopgtable_lookup_entry);

static size_t iopgtable_clear_entry_core(struct iommu *obj, u32 da)
{
	size_t bytes;
	u32 *iopgd = iopgd_offset(obj, da);
	int nent = 1;

	if (!*iopgd)
		return 0;

	if (*iopgd & IOPGD_TABLE) {
		int i;
		u32 *iopte = iopte_offset(iopgd, da);

		bytes = IOPTE_SIZE;
		if (*iopte & IOPTE_LARGE) {
			nent *= 16;
			/* rewind to the 1st entry */
			iopte = (u32 *)((u32)iopte & IOLARGE_MASK);
		}
		bytes *= nent;
		memset(iopte, 0, nent * sizeof(*iopte));
		flush_iopte_range(iopte, iopte + (nent - 1) * sizeof(*iopte));

		/*
		 * do table walk to check if this table is necessary or not
		 */
		iopte = iopte_offset(iopgd, 0);
		for (i = 0; i < PTRS_PER_IOPTE; i++)
			if (iopte[i])
				goto out;

		iopte_free(iopte);
		nent = 1; /* for the next L1 entry */
	} else {
		bytes = IOPGD_SIZE;
		if (*iopgd & IOPGD_SUPER) {
			nent *= 16;
			/* rewind to the 1st entry */
			iopgd = (u32 *)((u32)iopgd & IOSUPER_MASK);
		}
		bytes *= nent;
	}
	memset(iopgd, 0, nent * sizeof(*iopgd));
	flush_iopgd_range(iopgd, iopgd + (nent - 1) * sizeof(*iopgd));
out:
	return bytes;
}

/**
 * iopgtable_clear_entry() - Remove an iommu pte entry
 * @obj:	target iommu
 * @da:		iommu device virtual address
 **/
size_t iopgtable_clear_entry(struct iommu *obj, u32 da)
{
	size_t bytes;

	spin_lock(&obj->page_table_lock);

	bytes = iopgtable_clear_entry_core(obj, da);
	flush_iotlb_page(obj, da);

	spin_unlock(&obj->page_table_lock);

	return bytes;
}
EXPORT_SYMBOL_GPL(iopgtable_clear_entry);

static void iopgtable_clear_entry_all(struct iommu *obj)
{
	int i;

	spin_lock(&obj->page_table_lock);

	for (i = 0; i < PTRS_PER_IOPGD; i++) {
		u32 da;
		u32 *iopgd;

		da = i << IOPGD_SHIFT;
		iopgd = iopgd_offset(obj, da);

		if (!*iopgd)
			continue;

		if (*iopgd & IOPGD_TABLE)
			iopte_free(iopte_offset(iopgd, 0));

		*iopgd = 0;
		flush_iopgd_range(iopgd, iopgd);
	}

	flush_iotlb_all(obj);

	spin_unlock(&obj->page_table_lock);
}

/*
 *	Device IOMMU generic operations
 */
static irqreturn_t iommu_fault_handler(int irq, void *data)
{
	u32 stat, da;
	u32 *iopgd, *iopte;
	int err = -EIO;
	struct iommu *obj = data;

	/* Dynamic loading TLB or PTE */
	if (obj->isr)
		err = obj->isr(obj);

	if (!err)
		return IRQ_HANDLED;

	stat = iommu_report_fault(obj, &da);
	if (!stat)
		return IRQ_HANDLED;

	iopgd = iopgd_offset(obj, da);

	if (!(*iopgd & IOPGD_TABLE)) {
		dev_err(obj->dev, "%s: da:%08x pgd:%p *pgd:%08x\n", __func__,
			da, iopgd, *iopgd);
		return IRQ_NONE;
	}

	iopte = iopte_offset(iopgd, da);

	dev_err(obj->dev, "%s: da:%08x pgd:%p *pgd:%08x pte:%p *pte:%08x\n",
		__func__, da, iopgd, *iopgd, iopte, *iopte);

	dump_tlb_entries(obj);

	return IRQ_NONE;
}

static int device_match_by_alias(struct device *dev, void *data)
{
	struct iommu *obj = to_iommu(dev);
	const char *name = data;

	pr_debug("%s: %s %s\n", __func__, obj->name, name);

	return strcmp(obj->name, name) == 0;
}

/**
 * iommu_put() - Get iommu handler
 * @name:	target iommu name
 **/
struct iommu *iommu_get(const char *name)
{
	int err = -ENOMEM;
	struct device *dev;
	struct iommu *obj;

	dev = driver_find_device(&omap_iommu_driver.driver, NULL, (void *)name,
				 device_match_by_alias);
	if (!dev)
		return ERR_PTR(-ENODEV);

	obj = to_iommu(dev);

	mutex_lock(&obj->iommu_lock);

	if (obj->refcount++ == 0) {
		err = iommu_enable(obj);
		if (err)
			goto err_enable;
		flush_iotlb_all(obj);
	}

	if (!try_module_get(obj->owner))
		goto err_module;

	mutex_unlock(&obj->iommu_lock);

	dev_dbg(obj->dev, "%s: %s\n", __func__, obj->name);
	return obj;

err_module:
	if (obj->refcount == 1)
		iommu_disable(obj);
err_enable:
	mutex_unlock(&obj->iommu_lock);
	return ERR_PTR(err);
}
EXPORT_SYMBOL_GPL(iommu_get);

/**
 * iommu_put() - Put back iommu handler
 * @obj:	target iommu
 **/
void iommu_put(struct iommu *obj)
{
	if (!obj && IS_ERR(obj))
		return;

	mutex_lock(&obj->iommu_lock);

	if (--obj->refcount == 0)
		iommu_disable(obj);

	module_put(obj->owner);

	mutex_unlock(&obj->iommu_lock);

	dev_dbg(obj->dev, "%s: %s\n", __func__, obj->name);
}
EXPORT_SYMBOL_GPL(iommu_put);

/*
 *	OMAP Device MMU(IOMMU) detection
 */
static int __devinit omap_iommu_probe(struct platform_device *pdev)
{
	int err = -ENODEV;
	void *p;
	int irq;
	struct iommu *obj;
	struct resource *res;
	struct iommu_platform_data *pdata = pdev->dev.platform_data;

	if (pdev->num_resources != 2)
		return -EINVAL;

	obj = kzalloc(sizeof(*obj) + MMU_REG_SIZE, GFP_KERNEL);
	if (!obj)
		return -ENOMEM;

	obj->clk = clk_get(&pdev->dev, pdata->clk_name);
	if (IS_ERR(obj->clk))
		goto err_clk;

	obj->nr_tlb_entries = pdata->nr_tlb_entries;
	obj->name = pdata->name;
	obj->dev = &pdev->dev;
	obj->ctx = (void *)obj + sizeof(*obj);

	mutex_init(&obj->iommu_lock);
	mutex_init(&obj->mmap_lock);
	spin_lock_init(&obj->page_table_lock);
	INIT_LIST_HEAD(&obj->mmap);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		err = -ENODEV;
		goto err_mem;
	}
	obj->regbase = ioremap(res->start, resource_size(res));
	if (!obj->regbase) {
		err = -ENOMEM;
		goto err_mem;
	}

	res = request_mem_region(res->start, resource_size(res),
				 dev_name(&pdev->dev));
	if (!res) {
		err = -EIO;
		goto err_mem;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		err = -ENODEV;
		goto err_irq;
	}
	err = request_irq(irq, iommu_fault_handler, IRQF_SHARED,
			  dev_name(&pdev->dev), obj);
	if (err < 0)
		goto err_irq;
	platform_set_drvdata(pdev, obj);

	p = (void *)__get_free_pages(GFP_KERNEL, get_order(IOPGD_TABLE_SIZE));
	if (!p) {
		err = -ENOMEM;
		goto err_pgd;
	}
	memset(p, 0, IOPGD_TABLE_SIZE);
	clean_dcache_area(p, IOPGD_TABLE_SIZE);
	obj->iopgd = p;

	BUG_ON(!IS_ALIGNED((unsigned long)obj->iopgd, IOPGD_TABLE_SIZE));

	dev_info(&pdev->dev, "%s registered\n", obj->name);
	return 0;

err_pgd:
	free_irq(irq, obj);
err_irq:
	release_mem_region(res->start, resource_size(res));
	iounmap(obj->regbase);
err_mem:
	clk_put(obj->clk);
err_clk:
	kfree(obj);
	return err;
}

static int __devexit omap_iommu_remove(struct platform_device *pdev)
{
	int irq;
	struct resource *res;
	struct iommu *obj = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	iopgtable_clear_entry_all(obj);
	free_pages((unsigned long)obj->iopgd, get_order(IOPGD_TABLE_SIZE));

	irq = platform_get_irq(pdev, 0);
	free_irq(irq, obj);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, resource_size(res));
	iounmap(obj->regbase);

	clk_put(obj->clk);
	dev_info(&pdev->dev, "%s removed\n", obj->name);
	kfree(obj);
	return 0;
}

static struct platform_driver omap_iommu_driver = {
	.probe	= omap_iommu_probe,
	.remove	= __devexit_p(omap_iommu_remove),
	.driver	= {
		.name	= "omap-iommu",
	},
};

static void iopte_cachep_ctor(void *iopte)
{
	clean_dcache_area(iopte, IOPTE_TABLE_SIZE);
}

static int __init omap_iommu_init(void)
{
	struct kmem_cache *p;
	const unsigned long flags = SLAB_HWCACHE_ALIGN;

	p = kmem_cache_create("iopte_cache", IOPTE_TABLE_SIZE, 0, flags,
			      iopte_cachep_ctor);
	if (!p)
		return -ENOMEM;
	iopte_cachep = p;

	return platform_driver_register(&omap_iommu_driver);
}
module_init(omap_iommu_init);

static void __exit omap_iommu_exit(void)
{
	kmem_cache_destroy(iopte_cachep);

	platform_driver_unregister(&omap_iommu_driver);
}
module_exit(omap_iommu_exit);

MODULE_DESCRIPTION("omap iommu: tlb and pagetable primitives");
MODULE_ALIAS("platform:omap-iommu");
MODULE_AUTHOR("Hiroshi DOYU, Paul Mundt and Toshihiro Kobayashi");
MODULE_LICENSE("GPL v2");

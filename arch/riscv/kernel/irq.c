#include <linux/interrupt.h>
#include <linux/ftrace.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include "../../../drivers/irqchip/irqchip.h"
#include <linux/linkage.h>

#include <asm/ptrace.h>
#include <asm/sbi.h>
#include <asm/smp.h>

static struct irq_domain *riscv_irq_domain;

asmlinkage void __irq_entry do_IRQ(unsigned int irq, struct pt_regs *regs)
{
	if (irq == IRQ_SOFTWARE && handle_ipi())
		return;

	handle_domain_irq(riscv_irq_domain, irq, regs);
}

static void riscv_irq_mask(struct irq_data *d)
{
	switch (d->irq) {
	case IRQ_TIMER: 
		csr_clear(sie, SIE_STIE);
		break;
	case IRQ_SOFTWARE: 
		csr_clear(sie, SIE_SSIE);
		break;
	case IRQ_EXTERNAL:
		csr_clear(sie, SIE_SXIE);
		break;
	default:
		BUG();
	}
}

static void riscv_irq_unmask(struct irq_data *d)
{
	switch (d->irq) {
	case IRQ_TIMER: 
		csr_set(sie, SIE_STIE);
		break;
	case IRQ_SOFTWARE: 
		csr_set(sie, SIE_SSIE);
		break;
	case IRQ_EXTERNAL:
		csr_set(sie, SIE_SXIE);
		break;
	default:
		BUG();
	}
}

struct irq_chip riscv_irq_chip = {
	.name = "riscv",
	.irq_mask = riscv_irq_mask,
	.irq_mask_ack = riscv_irq_mask,
	.irq_unmask = riscv_irq_unmask,
};

static int riscv_irq_domain_map(struct irq_domain *d, unsigned int virq,
				irq_hw_number_t hw)
{
	irq_set_chip_and_handler(virq, &riscv_irq_chip, handle_level_irq);

	return 0;
}

static struct irq_domain_ops riscv_irq_domain_ops = {
	.map = riscv_irq_domain_map,
	.xlate = irq_domain_xlate_onecell,
};

void __init init_IRQ(void)
{
	riscv_irq_domain = irq_domain_add_linear(NULL, 5,
						 &riscv_irq_domain_ops, NULL);
	if (!riscv_irq_domain)
		panic("Unable to add RISC-V IRQ domain\n");
	irq_set_default_host(riscv_irq_domain);

}

static int __init riscv_irq_of_init(struct device_node *np,
				    struct device_node *interrupt_parent)
{
	/* this is probably frowned upon */
	if (riscv_irq_domain->of_node == NULL)
		riscv_irq_domain->of_node = np;

	return riscv_irq_domain ? 0 : -ENODEV;
}

IRQCHIP_DECLARE(riscv, "riscv,irq", riscv_irq_of_init);
